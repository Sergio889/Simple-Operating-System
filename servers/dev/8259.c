#ifndef COMMON
#define COMMON
#include "../../libcommon/com.h"
#endif
#include "irq.h"

/*
 * Functions specific to the 8259 PIC.
 */
void
pic_enabl_8259(uint_t irq_num)
{

}
void
pic_disbl_8259(uint_t irq_num)
{

}
void
pic_add_handl_8259(struct irqs irq, void (*intr_handlr)(uint_t))
{

}
void
pic_del_handl_8259(struct irqs irq, void (*intr_handlr)(uint_t))
{

}
