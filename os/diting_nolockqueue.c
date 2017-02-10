#include <linux/types.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/spinlock.h>

#include "diting_nolockqueue.h"

static diting_nolockqueue_t * diting_nolockqueue_privhead = NULL;

static int diting_nolockqueue_atomic32_cmpset(volatile uint32_t *dst, uint32_t exp, uint32_t src)
{
    uint8_t res;

    asm volatile(
            "lock ; "
            "cmpxchgl %[src], %[dst];"
            "sete %[res];"
            : [res] "=a" (res), 
              [dst] "=m" (*dst)
            : [src] "r" (src), 
              "a" (exp),
              "m" (*dst)
            : "memory"); 
    return res;
}

static int diting_nolockqueue_module_inside_align_pow2(uint32_t num)
{
	int offset = 0;
	uint32_t mark1, mark2;
	do
	{
		offset++;
		mark1 = (1 << offset);
		mark2 = (1 << (offset + 1));
	}while(num >= mark1);

	return (1 << offset); 
}

static diting_nolockqueue_t *diting_nolockqueue_module_inside_init_npool(uint32_t nem)
{
	int i, tnem;
	struct diting_nolockqueue *npool;

	tnem = diting_nolockqueue_module_inside_align_pow2(nem);

	npool = (struct diting_nolockqueue *)kmalloc(sizeof(struct diting_nolockqueue)
			+ sizeof(void *) * tnem, GFP_KERNEL);
	if(npool && !IS_ERR(npool))/*calloc success and initialize*/
	{
		for(i = 0; i < tnem; i++)
		{
			npool->ring[i] = NULL; 
		}
		npool->prod.watermark = tnem;
		npool->prod.size = npool->cons.size = tnem;
		npool->prod.mask = npool->cons.mask = tnem - 1;
		npool->prod.head = npool->cons.head = 0;
		npool->prod.tail = npool->cons.tail = 0;

		diting_nolockqueue_privhead = npool;
	}
	return npool;
}

static diting_nolockqueue_t *diting_nolockqueue_module_create(uint32_t nem)
{
	struct diting_nolockqueue *npool;

	npool = diting_nolockqueue_module_inside_init_npool(nem);
	if(!npool || IS_ERR(npool)){
		npool = NULL;
		goto out;
	}

out:
	return npool;
}

static diting_nolockqueue_t *diting_nolockqueue_module_getqueue(void)
{
	return diting_nolockqueue_privhead;
}

static int diting_nolockqueue_module_enqueue(struct diting_nolockqueue * ring, void * item)
{
	int success, ret;
	uint32_t prod_head, prod_next;
	uint32_t cons_tail, free_entries;
	uint32_t mask = ring->prod.mask;

	do{
		prod_head = ring->prod.head;
		cons_tail = ring->cons.tail;
		free_entries = (cons_tail - prod_head + mask);
		if(free_entries <= 0)
		{
			ret = -1;
			goto out;
		}

		prod_next = prod_head + 1;
		success = diting_nolockqueue_atomic32_cmpset(&ring->prod.head, prod_head, prod_next);
	}while(unlikely(success == 0));

	ring->ring[prod_head & mask] = item;
	DITING_NOLOCKQUEUE_COMPILER_BARRIER();

	while(unlikely(ring->prod.tail != prod_head));
	ring->prod.tail = prod_next;

	ret = 0;
out:
	return ret;
}

static int diting_nolockqueue_module_dequeue(struct diting_nolockqueue *ring, void **item)
{
	int success, ret;
	uint32_t cons_head, prod_tail;	
	uint32_t cons_next, busy_entries;
	uint32_t mask = ring->prod.mask;

	do
	{
		prod_tail = ring->prod.tail;		
		cons_head = ring->cons.head;
		busy_entries = (prod_tail - cons_head);
		if(busy_entries <= 0)	
		{
			ret = -1;
			goto out;
		}

		cons_next = cons_head + 1;
		success = diting_nolockqueue_atomic32_cmpset(&ring->cons.head,\
			cons_head, cons_next);

	}while(unlikely(success == 0));

	*item = ring->ring[cons_head & mask];
	DITING_NOLOCKQUEUE_COMPILER_BARRIER();

	while(unlikely(ring->cons.tail != cons_head));
	ring->cons.tail = cons_next;

	ret = 0;
out:	
	return ret;
}

static int diting_nolockqueue_module_destroy(diting_nolockqueue_t *ring)
{
	if(ring && !IS_ERR(ring))
		kfree(ring);
	return 0;
}


struct diting_nolockqueue_module diting_nolockqueue_module = 
{
	.create		= diting_nolockqueue_module_create,
	.getque		= diting_nolockqueue_module_getqueue,
	.enqueue	= diting_nolockqueue_module_enqueue,
	.dequeue	= diting_nolockqueue_module_dequeue,
	.destroy	= diting_nolockqueue_module_destroy
};
