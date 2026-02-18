#ifndef VM_H
#define VM_H

#include <vm/chunk.h>

#define VM_STACK_SIZE	0x1000

typedef enum vm_result_t	vm_result_t;
enum vm_result_t
{
	VM_RES_OK,
	VM_RES_COMPILE_ERR,
	VM_RES_RUNTIME_ERR,
};

typedef struct vm_t	vm_t;
struct vm_t
{
	chunk_t*	head;
	chunk_t*	tail;
	u8*		ip;
	value_t		stack[VM_STACK_SIZE];
	value_t*	stack_top;
	context_t*	context;
};

vm_result_t	vm_run(vm_t*, chunk_t*);
vm_t*		vm_new(context_t*);

#ifdef TESTER

#endif	// TESTER

#endif	// VM_H
