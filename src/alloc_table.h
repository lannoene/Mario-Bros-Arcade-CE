#pragma once

#define DEF_ALLOC_TABLE(x, size)\
unsigned int __nalloc_ ## x = 0;\
void * x ## _s_alloc(int s) {\
	static uint8_t _s_heap[size];\
	\
	if (__nalloc_ ## x + s > sizeof(_s_heap)) {\
		dbg_printf("Could not statically allocate variable (" #x ").\n");\
		return NULL;\
	} else {\
		dbg_printf("Allocating " #x " variable: size: %d\n", s);\
	}\
	void *ret = &_s_heap[__nalloc_ ## x];\
	__nalloc_ ## x += s;  /* reserve space */\
	return ret;\
}\
\
void x ## _s_reset (void) {\
	__nalloc_ ## x = 0;\
}

#define EXPORT_ALLOC_TABLE(x)\
void * x ## _s_alloc(int s);\
void x ## _s_reset(void);
