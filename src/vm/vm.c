#include <math.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <vm/vm.h>

#define VM_ERROR(vm, msg, ...)					\
do								\
{								\
	fprintf(stderr, msg, ##__VA_ARGS__);			\
	chunk_t*_chunk	= vm_find_current_chunk(vm);		\
	u32	_inst	= vm->ip - _chunk->code - 1;		\
	s32	_line	= _chunk->lines[_inst];			\
	fprintf(stderr, "\n[line %d] in script\n", _line);	\
	vm_reset_stack(vm);					\
} while (0)

#define VM_BOP(vm, op)									\
do											\
{											\
	value_t	a = *vm_pop(vm);							\
	value_t b = *((vm)->stack_top - 1);						\
											\
	if (a.type > VAL_STR || b.type > VAL_STR)					\
	{										\
		VM_ERROR(vm, "Operands must be numbers or strings");			\
		return VM_RES_RUNTIME_ERR;						\
	}										\
											\
	if ((a.type == VAL_STR || b.type == VAL_STR) && memcmp("+", #op, 1) != 0)	\
	{										\
		VM_ERROR(vm, "Operands must be numbers");				\
		return VM_RES_RUNTIME_ERR;						\
	}										\
											\
	value_t*	top = ((vm)->stack_top - 1);					\
											\
	if (a.type == b.type)								\
	{										\
		switch (b.type)								\
		{									\
		case VAL_INT	: top->as.integer op##= a.as.integer;	break;		\
		case VAL_NB	: top->as.number op##= a.as.number;	break;		\
		case VAL_STR	:							\
			top->as.string = string_concat((vm)->context->arena_line, b.as.string, a.as.string);\
			break;								\
		default:								\
		}									\
											\
		break;									\
	}										\
											\
	switch (b.type)									\
	{										\
	case VAL_INT	: *top = VAL_AS_NB((f64)b.as.integer op a.as.number); break;	\
	case VAL_NB	: top->as.number op##= (f64)a.as.integer; break;		\
	case VAL_STR	:								\
		VM_ERROR(vm, "Operands cannot be strings and numbers");			\
		return VM_RES_RUNTIME_ERR;						\
	default:									\
	}										\
} while (0)

#define VM_COMPARE(vm, op)								\
do											\
{											\
	value_t	a = *vm_pop(vm);							\
	value_t b = *((vm)->stack_top - 1);						\
											\
	if (a.type > VAL_NB || b.type > VAL_NB)						\
	{										\
		VM_ERROR(vm, "Operands must be numbers");				\
		return VM_RES_RUNTIME_ERR;						\
	}										\
											\
	if (a.type == b.type)								\
	{										\
		switch (b.type)								\
		{									\
		case VAL_INT: *((vm)->stack_top - 1) = VAL_AS_BOOL(b.as.integer op a.as.integer); break;\
		case VAL_NB: *((vm)->stack_top - 1) = VAL_AS_BOOL(b.as.number op a.as.number); break;\
		default:								\
		}									\
											\
		break;									\
	}										\
											\
	switch (b.type)									\
	{										\
	case VAL_INT: *((vm)->stack_top - 1) = VAL_AS_BOOL(b.as.number op a.as.integer); break;\
	case VAL_NB: *((vm)->stack_top - 1) = VAL_AS_BOOL(b.as.integer op a.as.number); break;\
	default:									\
	}										\
} while (0)

#define VM_EQ(vm, op)										\
do												\
{												\
	value_t	a = *vm_pop(vm);								\
	value_t	b = *(vm->stack_top - 1);							\
												\
	if (a.type == VAL_ARR || b.type == VAL_ARR)						\
	{											\
		VM_ERROR(vm, "Operands cannot be array");					\
		return VM_RES_RUNTIME_ERR;							\
	}											\
												\
	if (a.type == b.type)									\
	{											\
		switch (b.type)									\
		{										\
		case VAL_INT:									\
			*((vm)->stack_top - 1) = VAL_AS_BOOL(a.as.integer op b.as.integer);	\
			break;									\
		case VAL_NB:									\
			*((vm)->stack_top - 1) = VAL_AS_BOOL(a.as.number op b.as.number);	\
			break;									\
		case VAL_BOOL:									\
			((vm)->stack_top - 1)->as.boolean = a.as.boolean op b.as.boolean;	\
			break;									\
		case VAL_STR:									\
			*((vm)->stack_top - 1) = VAL_AS_BOOL(string_cmp(a.as.string, b.as.string) op 1);\
												\
		default:									\
		}										\
		break;										\
	}											\
												\
	switch (b.type)										\
	{											\
	case VAL_INT:										\
		if (a.type == VAL_NB)								\
			*((vm)->stack_top - 1) = VAL_AS_BOOL(a.as.number op (f64)b.as.integer);	\
		else										\
			*((vm)->stack_top - 1) = VAL_AS_BOOL(a.as.boolean op (bool)b.as.integer);\
		break;										\
	case VAL_NB:										\
		if (a.type == VAL_INT)								\
			*((vm)->stack_top - 1) = VAL_AS_BOOL((f64)a.as.integer op b.as.number);	\
		else										\
			*((vm)->stack_top - 1) = VAL_AS_BOOL(a.as.boolean op (bool)b.as.number);\
		break;										\
	case VAL_BOOL:										\
		if (a.type == VAL_INT)								\
			*((vm)->stack_top - 1) = VAL_AS_BOOL((bool)a.as.integer op b.as.boolean);\
		else										\
			*((vm)->stack_top - 1) = VAL_AS_BOOL((bool)a.as.number op b.as.boolean);\
		break;										\
	case VAL_STR:										\
		VM_ERROR(vm, "Operands cannot be object and base type");			\
		return VM_RES_RUNTIME_ERR;							\
	default:										\
	}											\
} while (0)

static chunk_t*
vm_find_current_chunk(vm_t* vm)
{
	chunk_t*	chunk 	= vm->head;
	u8*		ip	= vm->ip;
	u32		capacity= chunk->capacity;

	while (chunk && chunk->code + capacity < ip)
		chunk = chunk->next;

	return chunk;
}

static inline u8
vm_consume_byte(vm_t* vm)
{
	u32	capacity= vm->tail->capacity;
	u8	byte	= *vm->ip;

	if (vm->ip == vm->tail->code + capacity)
	{
		vm->tail= vm->tail->next;
		vm->ip	= vm->tail->code;
	}
	else
		vm->ip++;

	return byte;
}

static inline void
vm_push(vm_t* vm, value_t value)
{
	assert(vm->stack_top < vm->stack + VM_STACK_SIZE);

	*vm->stack_top = value;
	vm->stack_top++;
}

static inline value_t*
vm_pop(vm_t* vm)
{
	if (vm->stack_top <= vm->stack)
		return 0;

	vm->stack_top--;
	return vm->stack_top;
}

static inline void
vm_reset_stack(vm_t* vm) { vm->stack_top = vm->stack; }

inline vm_t*
vm_new(context_t* context)
{
	vm_t*	vm = ARENA_PUSH_STRUCT(context->arena_glb, vm_t);

	vm->stack_top	= vm->stack;
	vm->context	= context;

	return vm;
}

vm_result_t
vm_run(vm_t* vm, chunk_t* chunk)
{
	assert(vm);

	if (!chunk)
		return VM_RES_COMPILE_ERR;

	vm->head = chunk;
	vm->tail = chunk;
	vm->ip	 = chunk->code;

	value_array_t*	constants	= vm->context->constants;
	value_map*	globals		= vm->context->globals;

	for (;;)
	{
		u8	instruction = vm_consume_byte(vm);

		switch (instruction)
		{
		case OP_CONST_8:
		{
			value_t	constant = value_array_read(constants, vm_consume_byte(vm));
			vm_push(vm, constant);
		} break;
		case OP_CONST_16:
		{
			u32	l	= vm_consume_byte(vm);
			u32	r	= vm_consume_byte(vm);
			u32	index	= CHUNK_CONST_16_GET(l, r);
			value_t	constant= value_array_read(constants, index);

			vm_push(vm, constant);
		} break;
		case OP_DEFINE_GLOBAL:
		{
			value_t		val	= *vm_pop(vm);
			value_t		name	= *vm_pop(vm);

			value_map_put(globals, name, val);
		} break;
		case OP_GET_GLOBAL:
		{
			value_t		name	= *vm_pop(vm);
			value_t*	val	= value_map_get(globals, name);
			
			if (!val)
			{
				VM_ERROR(vm, "Undefined variable %s", name.as.string->buf);
				return VM_RES_RUNTIME_ERR;
			}

			vm_push(vm, *val);
		} break;
		case OP_NEG:
		{
			value_t*	val = vm->stack_top - 1;

			switch (val->type)
			{
			case VAL_NB	: val->as.number = -val->as.number; break;
			case VAL_INT	: val->as.integer = -val->as.integer; break;
			case VAL_ARR	:
			case VAL_STR	:
			case VAL_BOOL	:
				VM_ERROR(vm, "Operand must be a number"); return VM_RES_RUNTIME_ERR;
			}
		} break;
		case OP_NOT:
		{
			value_t*	val = vm->stack_top - 1;

			switch (val->type)
			{
			case VAL_NB	: *val = VAL_AS_BOOL(!val->as.number); break;
			case VAL_INT	: *val = VAL_AS_BOOL(!val->as.integer); break;
			case VAL_BOOL	: *val = VAL_AS_BOOL(!val->as.boolean); break;
			case VAL_STR	:
			case VAL_ARR	:
				VM_ERROR(vm, "Operand cannot be an object"); return VM_RES_RUNTIME_ERR;
			}
		} break;
		case OP_ZERO	: vm_push(vm, VAL_AS_INT(0)); break;
		case OP_EQ	: VM_EQ(vm, ==); break;
		case OP_NOT_EQ	: VM_EQ(vm, !=); break;
		case OP_MORE	: VM_COMPARE(vm, >); break;
		case OP_LESS	: VM_COMPARE(vm, <); break;
		case OP_MORE_EQ	: VM_COMPARE(vm, >=); break;
		case OP_LESS_EQ	: VM_COMPARE(vm, <=); break;
		case OP_ADD	: VM_BOP(vm, +); break;
		case OP_SUB	: VM_BOP(vm, -); break;
		case OP_MUL	: VM_BOP(vm, *); break;
		case OP_DIV	: VM_BOP(vm, /); break;
		case OP_NAN	: vm_push(vm, VAL_AS_NB(NAN)); break; 
		case OP_INF	: vm_push(vm, VAL_AS_NB(INFINITY)); break;
		case OP_TRUE	: vm_push(vm, VAL_AS_BOOL(1)); break;
		case OP_FALSE	: vm_push(vm, VAL_AS_BOOL(0)); break;
		case OP_PRINT	: value_print(*vm_pop(vm)); printf("\n"); break;
		case OP_POP	: vm_pop(vm); break;
		case OP_RET	: return VM_RES_OK;
		}
	}

	return VM_RES_RUNTIME_ERR;
}

#ifdef TESTER


#endif	// TESTER
