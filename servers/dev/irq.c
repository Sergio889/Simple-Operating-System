#include "dev.h"
#include "irq.h"
static struct pic_ctrlr *pic_head; // pointer to header of controllers
extern void pic_enabl_8259(uint_t); // enable irq line on 8259
extern void pic_disbl_8259(uint_t); // disable irq line on 8259
extern void pic_add_handl_8259(struct irqs *, void (*)(uint_t));
extern void pic_del_handl_8259(struct irqs *, void (*)(uint_t));

static void
irq_nums(struct pic_ctrlr *ptr_pic)
{
}

/*
 * Initialize the interrupt system specific to PIC.
 */
void
init_irq(uint_t pic_num)
{
}
