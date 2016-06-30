#ifndef COMMON
#define COMMON
#include "../../libcommon/com.h"
#endif
#define PIC_8259 0x01 // pic is legacy 8259
#define PIC_APIC 0x02 // pic is intel modern apic
#define PIC_8259_INTS 8

struct irq_hands {
	uint_t dev_id; // idenfity device related to handler
	uint_t dev_flags; // irq shareable or not, etc...
	
	struct irq_hands *handlr_next;
	void (*irq_handlr)(uint_t);
};

struct irqs {
	uint_t irq_num; // number of the irq line
	struct irq_hands *irq_hands; // pointer to handlers for irq
	struct irqs *irq_next; // next irq on list
};

/*
 * Generic interface to interrupt controllers.
 * Note: we use PIC for refering generically
 * to any interrupt controller.
 */
struct pic_ctrlr {
	uint_t pic_type; // i8259a, APIC...
	uint_t pic_ints; // number of interrupts for pic
	uint_t pic_flags; // pic-specific attributes

	struct irqs *pic_irqs; // pointer to header of irqs
	struct pic_ctrlr *pic_next; // next pic in list

	void (*pic_enabl)(uint_t irq_num); // disable irq line
	void (*pic_disbl)(uint_t irq_num); // enable irq line
	void (*pic_add_handl)(struct irqs *irq, void (*)(uint_t)); // share irq
	void (*pic_del_handl)(struct irqs *irq, void (*)(uint_t)); // remove a share
};
