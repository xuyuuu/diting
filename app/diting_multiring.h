#ifndef __diting_multiring_h__
#define __diting_multiring_h__

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define DITING_MULTIRING_X86_ALIGN_SIZE 64
#define DITING_MULTIRING_X86_ALIGN_MASK (DITING_MULTIRING_X86_ALIGN_SIZE - 1)

#define DITING_MULTIRING_COMPILER_BARRIER() do {\
	        asm volatile ("" : : : "memory");\
} while(0)

typedef struct diting_multiring diting_multiring_t;

struct diting_multiring_module
{
	diting_multiring_t *(* create)(uint32_t nem);
	int (* enqueue)(diting_multiring_t *ring, void *item);
	int (* dequeue)(diting_multiring_t *ring, void **item);
	int (* destroy)(void);
}__attribute__((packed));


struct diting_multiring
{
	/*diting_multiring producer status*/
	struct producer
	{
		uint32_t watermark;
		uint32_t sp_enqueue;
		uint32_t size;
		uint32_t mask;

		volatile uint32_t head;
		volatile uint32_t tail;
	}prod __attribute__((__aligned__(64)));

	/*diting_multiring consumer status */
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

extern struct diting_multiring_module diting_multiring_module;

#endif

