typedef char *va_list;
#define va_start(LIST, SADDR) \
	LIST = (va_list)SADDR + sizeof(int)
#define va_arg(LIST, TYPE) \
	*((TYPE *)LIST) += sizeof(int)
