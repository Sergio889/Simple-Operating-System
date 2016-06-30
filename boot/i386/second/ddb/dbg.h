#include "../term.h"
#include "../heap.h"

struct sym_list {
	struct elf_sym *ptr_cur;
	struct sym_list *ptr_next;
};
