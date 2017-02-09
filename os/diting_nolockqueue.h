#ifndef __diting_nolockqueue_h__
#define __diting_nolockqueue_h__

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define DITING_NOLOCKQUEUE_X86_ALIGN_SIZE 64
#define DITING_NOLOCKQUEUE_X86_ALIGN_MASK (DITING_NOLOCKQUEUE_X86_ALIGN_SIZE - 1)

#define DITING_NOLOCKQUEUE_COMPILER_BARRIER() do {\
	        asm volatile ("" : : : "memory");\
} while(0)

typedef struct diting_nolockqueue diting_nolockqueue_t;

struct diting_nolockqueue_module
{
	diting_nolockqueue_t *(* create)(uint32_t nem);
	int (* enqueue)(diting_nolockqueue_t *ring, void *item);
	int (* dequeue)(diting_nolockqueue_t *ring, void **item);
	int (* destroy)(void);
}__attribute__((packed));


struct diting_nolockqueue
{
	/*diting_nolockqueue producer status*/
	struct producer
	{
		uint32_t watermark;
		uint32_t sp_enqueue;
		uint32_t size;
		uint32_t mask;

		volatile uint32_t head;
		volatile uint32_t tail;
	}prod __attribute__((__aligned__(64)));

	/*diting_nolockqueue consumer status */
	struct consumer
	{
		uint32_t sc_dequeue;	
		uint32_t size;
		uint32_t mask;

		volatile uint32_t head;
		volatile uint32_t tail;
	}cons __attribute__((__aligned__(64)));

	void *ring[0];
}__attribute__((packed));

extern struct diting_nolockqueue_module diting_nolockqueue_module;

#endif

