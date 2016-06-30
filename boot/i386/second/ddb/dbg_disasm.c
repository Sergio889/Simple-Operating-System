#include "dbg_disasm.h"
#include "dbg_x86map.h"

/* cadendas para imprimir */
static char *regs_word[]={"eax","ecx","edx","ebx","esp","ebp","esi","edi" };
static char *regs_half[]={"ax","cx","dx","bx","sp","bp","si","di" };
static char *regs_byte[]={"al","cl","dl","bl","ah","ch","dh","bh" };
static char *regs_sib[]={"eax","ecx","edx","ebx","esp",NULL,"esi","edi" };
static char *regs_sib2[]={"eax","ecx","edx","ebx",NULL,"ebp","esi","edi" };
static char *prefx_lock = "LOCK ";
static char *prefx_repn = "repne/repnz ";
static char *prefx_rep = "rep/repe/repz ";
static char *prefx_cs = "CS segment ";
static char *prefx_ss = "SS segment ";
static char *prefx_fs = "FS segment ";
static char *prefx_gs = "GS segment ";
static char *prefx_es = "ES segment ";
static char *prefx_ds = "DS segment ";

/* mantener en variables globales para reducir pila */
static uint_t modrm_glb = FALSE;
static word *op_word;
static ubyte_t *op_code, *ptr_op_code, *cur_modrm;
static word *op_wcode;
static struct op_map *op_codmap;
static struct xop_map *xop_codmap;
static uint_t i, m, diff;
static uint_t prnt_var;


/* manejar casos de prefijos */
static int
op_is_prefix(ubyte_t op_code)
{
	switch (op_code) {
		case 0xf0:
			kprintf(prefx_lock);
			break;
		case 0xf2:
			kprintf(prefx_repn);
			break;
		case 0xf3:
			kprintf(prefx_rep);
			break;
		case 0x2e:
			kprintf(prefx_cs);
			break;
		case 0x36:
			kprintf(prefx_ss);
			break;
		case 0x3e:
			kprintf(prefx_ds);
			break;
		case 0x26:
			kprintf(prefx_es);
			break;
		case 0x64:
			kprintf(prefx_fs);
			break;
		case 0x65:
			kprintf(prefx_gs);
			break;
		case 0x66:
			break;
		case 0x67:
			break;
		default:
			return FALSE;
		return TRUE;
	}
}


static void
disp(uint_t dis_num, ubyte_t *ptr_dis)
{
	uint_t i;

	for (i=0; i<512; i++)
		buffer[i]=0;

	switch (dis_num) {
		case 8:
			kprintf("%1x", ptr_dis);
			break;
		case 32:
			kprintf("%x", ptr_dis);
			break;
	}
}

static void
sib_03(ubyte_t *sib)
{
}
static void
sib_02(ubyte_t *sib)
{
	disp(32, (ubyte_t *)(sib+1));
	kprintf("(");
	kprintf(regs_sib[(*sib)&0x07]);
	kprintf(", ");
	kprintf(regs_sib2[((*sib)>>3)&0x07]);
	kprintf(", ");
	kprintf("4");
	kprintf(")");
}
static void
sib_01(ubyte_t *sib)
{
}
static void
sib_00(ubyte_t *sib)
{
	kprintf("(");
	kprintf(regs_sib[(*sib)&0x07]);
	kprintf(")");
}
static void (*sib_funcs[])(ubyte_t *) = { (void *)sib_00, 
						(void *)sib_01, (void *)sib_02, (void *)sib_03 };

static uint_t
mod_00(ubyte_t *modrm, uaddr_t *regs_sav)
{
	uint_t ret_val;

	/* mask rm */
	switch ((*modrm)&0x07) {
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
			kprintf("(");
			kprintf(regs_word[(*modrm)&0x07]);
			kprintf(")");
			ret_val=0;
			break;
		case 0x04:
			sib_funcs[(((*(modrm+1))>>6)&0x3)](modrm+1);
			ret_val=1;
			break;
		case 0x05:
			disp(32, (ubyte_t *)modrm+1);
			ret_val=4;
			break;
		case 0x06:
		case 0x07:
			kprintf("(");
			kprintf(regs_word[(*modrm)&0x07]);
			kprintf(")");
			ret_val=0;
			break;
	}

	return ret_val;
}
static uint_t
mod_01(ubyte_t *modrm, uaddr_t regs_sav)
{
	uint_t ret_val;
	char cur_char=0;

	/* mask rm */
	switch ((*modrm)&0x07) {
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
			kprintf(""); // XXX
			disp(8, (ubyte_t *)(modrm+1));
			kprintf("(");
			kprintf(regs_word[(*modrm)&0x07]);
			kprintf(")");
			ret_val=1;
			break;
		case 0x04:
			disp(8, (ubyte_t *)(modrm+2));
			sib_funcs[(((*(modrm+1))>>6)&0x3)](modrm+1);
			ret_val=2;
			break;
		case 0x05:
		case 0x06:
		case 0x07:
			disp(8, (ubyte_t *)(modrm+1));
			kprintf("(");
			kprintf(regs_word[(*modrm)&0x07]);
			kprintf(")");
			ret_val=1;
			break;
	}

	return ret_val;
}
static uint_t
mod_02(ubyte_t *modrm, uaddr_t regs_sav)
{
	uint_t ret_val;

	/* mask rm */
	switch ((*modrm)&0x07) {
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
			disp(32, (ubyte_t *)(modrm+1));
			kprintf("(");
			kprintf(regs_word[(*modrm)&0x07]);
			kprintf(")");
			ret_val=4;
			break;
		case 0x04:
			sib_funcs[(((*(modrm+1))>>6)&0x3)](modrm+1);
			disp(32, (ubyte_t *)(modrm+2));
			ret_val=5;
			break;
		case 0x05:
		case 0x06:
		case 0x07:
			disp(32, (ubyte_t *)(modrm+1));
			kprintf("(");
			kprintf(regs_word[(*modrm)&0x07]);
			kprintf(")");
			ret_val=4;
			break;
	}

	return ret_val;
}

static uint_t
mod_03(ubyte_t *modrm, uaddr_t regs_sav)
{
	kprintf(regs_word[(*modrm)&0x07]);

	return 0;
}

static uint_t (*mod_funcs[])(ubyte_t *, uaddr_t *) = { (void *)mod_00, 
						(void *)mod_01, (void *)mod_02, (void *)mod_03 };

/*
 * Disassembles num_ops instructions starting at addr_dasm.
 */
uaddr_t *
dbg_disasm(uaddr_t *addr_dasm, usize_t num_ops, uaddr_t *regs_sav, 
					 uint_t elf_indx)
{
	/* op_code apuntador tipo ubyte_t */
	op_code = (ubyte_t *)addr_dasm;

	/* desensamblar num_ops instrucciones */
	while (num_ops--) {
		dbg_symbol(op_code, elf_indx);
		kprintf("\n ");

		/* contar bytes de prefijo */
		while ((op_is_prefix(*op_code)) == TRUE)
			op_code++;

		/* if escape opcode */
		if (*op_code==0x0f) {
			op_code++, op_codmap = op_indx_1[16];
			/* map opcode */
			//kprintf("Unsuppored instruction");
			//return;
		}
		else
			/* map opcode */
			op_codmap = &op_list_1[*op_code];
		
		if (op_codmap->op_code==-1)
			kprintf("Unsuppored instruction");
		else {
			/* if we have an extended opcode in modrm */
			if (op_codmap->op_str==NULL) {
				if (op_codmap->op_code >= 0x80 && op_codmap->op_code <= 0x83)
					xop_codmap=xop_grp1;
				else if (op_codmap->op_code==0x8f)
					xop_codmap=xop_grp1A;
				else if ((op_codmap->op_code==0xc0 || op_codmap->op_code==0xc1) ||
								  (op_codmap->op_code>=0xd0 && op_codmap->op_code<=0xd4))
					xop_codmap=xop_grp2;
				else if (op_codmap->op_code==0xf6 || op_codmap->op_code==0xf7)
					xop_codmap=xop_grp3;
				else if (op_codmap->op_code==0xfe)
					xop_codmap=xop_grp4;
				else if (op_codmap->op_code==0xff)
					xop_codmap=xop_grp5;
				else if (op_codmap->op_code==0xc6 || op_codmap->op_code==0xc7)
					xop_codmap=xop_grp11;
				while (xop_codmap->op_ext!=(((*(op_code+1))>>3)&0x07))
					xop_codmap++;
				kprintf(xop_codmap->op_str);
				kprintf(" ");
			}
	
			/* print opcode */
			else {
				kprintf(op_codmap->op_str);
				kprintf(" ");
			}
		
			/* print operands */
			cur_modrm=op_code+1; // it might not be modrm byte
			op_code++;
			for (op_word=&(op_codmap->op_dst), i=0; i<3; i++, op_word++) {
				/* print source operand (if one) */
				if (*op_word!=0) {
					/* if determined by the MODRM field */
					if ((*op_word & WITH_MODRM) != 0) {
						if (modrm_glb==FALSE)
							op_code+=1, modrm_glb=TRUE;
						if ((*op_word & WITH_REG) != 0) {
							if ((*op_word&BYTE)!=0)
								kprintf(regs_byte[((*cur_modrm)>>3)&0x07]);
							else
								kprintf(regs_word[((*cur_modrm)>>3)&0x07]);
						}
						else
							if ( (m=mod_funcs[(((*cur_modrm)>>6)&0x3)](cur_modrm, regs_sav)))
								op_code+=m;
					}
					else if ((*op_word & WITH_IMM) != 0) {
						if ((*op_word&BYTE)!=0)
							disp(8, (ubyte_t *)(op_code)), op_code++;
						else
							disp(32, (ubyte_t *)(op_code)), op_code+=4;
					}
					else if ((*op_word & WITH_DIS) != 0) {
						prnt_var=((word)addr_dasm)+(*op_code)+2; // XXX
						kprintf("%x", &prnt_var);
						dbg_symbol(prnt_var, elf_indx);
						op_code++;
					}
					else if ((*op_word & WITH_EIP) != 0) {
						if ((*op_word&BYTE)!=0) {
							prnt_var=((word)addr_dasm)+(*op_code)+2; // XXX
							kprintf("%x", &prnt_var);
							dbg_symbol(prnt_var, elf_indx);
							op_code++;
						}
						else {
							op_wcode=(word *)(op_code); // pointer to EIP offset
							prnt_var=((word)addr_dasm)+(*op_wcode)+5; // XXX
							kprintf("%x", &prnt_var);
							dbg_symbol(prnt_var, elf_indx);
							op_code+=4;
						}
					}
					else if ((*op_word&WITH_eBP)!=0)
						kprintf("ebp");
					else if ((*op_word&WITH_eCX)!=0)
						kprintf("ecx");
					else if ((*op_word&WITH_rAX)!=0)
						kprintf("eax");
					else if ((*op_word&WITH_MEM)!=0) {
						if ((*op_word&BYTE)!=0) {
						}
						else if ((*op_word&WORD)!=0) {
						}
						else if ((*op_word&WITH_PSEUDO)!=0) {
						}
					}
					if (i==2 || (*(op_word+1)) == 0)
						;
					else 
						kprintf(", ");
				}
			}
		}
		addr_dasm = (uaddr_t *)op_code;
		modrm_glb=FALSE;
	}

	return (uaddr_t *)op_code;
}
