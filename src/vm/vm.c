#include <math.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <vm/vm.h>

#define VM_ERROR(vm, msg, ...)					\
do								\
{								\
	fprintf(stderr, msg, ##__VA_ARGS__);			\
	chunk_t*_chunk	= (vm)->tail;		\
	u32	_inst	= (vm)->ip - _chunk->code - 1;		\
	s32	_line	= _chunk->lines[_inst];			\
	fprintf(stderr, "\n[line %d] in script\n", _line);	\
	vm_reset_stack(vm);					\
	return VM_RES_RUNTIME_ERR;				\
} while (0)

#define VM_UOP_NB(vm, op)								\
do											\
{											\
	value_t*	val = (vm)->stack_top - 1;					\
											\
	switch (val->type)								\
	{										\
	case VAL_NB	: val->as.number = op val->as.number; break;			\
	case VAL_INT	: val->as.integer = op val->as.integer; break;			\
	case VAL_ARR	:								\
	case VAL_STR	:								\
	case VAL_BOOL	:								\
		VM_ERROR(vm, "Operand must be a number");				\
	}										\
} while (0)

#define VM_BOP(vm, op)									\
do											\
{											\
	value_t	a = *vm_pop(vm);							\
	value_t b = *((vm)->stack_top - 1);						\
											\
	if (a.type > VAL_STR || b.type > VAL_STR)					\
		VM_ERROR(vm, "Operands must be numbers or strings");			\
											\
	if ((a.type == VAL_STR || b.type == VAL_STR) && memcmp("+", #op, 1) != 0)	\
		VM_ERROR(vm, "Operands must be numbers");				\
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
	case VAL_STR	: VM_ERROR(vm, "Operands cannot be strings and numbers");	\
	default:									\
	}										\
} while (0)

#define VM_BITOP(vm, op)								\
do											\
{											\
	value_t	a = *vm_pop(vm);							\
	value_t b = *((vm)->stack_top - 1);						\
											\
	if (a.type > VAL_NB || b.type > VAL_NB)						\
		VM_ERROR(vm, "Operands must be numbers");				\
											\
	value_t*	top = ((vm)->stack_top - 1);					\
											\
	if (a.type == VAL_NB)								\
		a.as.integer = (s64)a.as.number;					\
	if (b.type == VAL_NB)								\
		b.as.integer = (s64)b.as.number;					\
											\
	top->as.integer	= a.as.integer op b.as.integer;					\
	top->type	= VAL_INT;							\
} while (0)

#define VM_COMPARE(vm, op)								\
do											\
{											\
	value_t	a = *vm_pop(vm);							\
	value_t b = *((vm)->stack_top - 1);						\
											\
	if (a.type > VAL_NB || b.type > VAL_NB)						\
		VM_ERROR(vm, "Operands must be numbers");				\
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
		VM_ERROR(vm, "Operands cannot be array");					\
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
	default:										\
	}											\
} while (0)

static inline u8
vm_consume_byte(vm_t* vm)
{
	u8	byte	= *vm->ip;

	if (vm->ip == vm->tail->end)
	{
		vm->tail= vm->tail->next;
		vm->ip	= vm->tail->code;
	}
	else
		vm->ip++;

	return byte;
}

static inline void
vm_revert_state(vm_t* vm, u32 offset)
{
	chunk_t*	chunk	= vm->tail;
	u32		capacity= chunk->capacity;
	u32		start	= chunk->id * capacity;
	u32		idx	= (u32)(vm->ip - chunk->code);
	u32		to_jump	= start + idx - offset;

	while (chunk->prev && to_jump < start)
	{
		chunk = chunk->prev;
		start -= capacity;
	}
	
	vm->ip	= chunk->code + (to_jump - start);
	vm->tail= chunk;
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

static inline value_t*
vm_pop_many(vm_t* vm, u8 count)
{
	if (vm->stack_top - count + 1 <= vm->stack)
		return 0;

	vm->stack_top -= count;
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
		case OP_JMPF:
		{
			value_t*	val	= vm->stack_top - 1;
			u32		l	= vm_consume_byte(vm);
			u32		r	= vm_consume_byte(vm);
			u32		jump	= CHUNK_CONST_16_GET(l, r)
				* (val->type == VAL_BOOL ? val->as.boolean == 0 : val->as.integer == 0);
			
			while (jump--)
				vm_consume_byte(vm);
		} break;
		case OP_JMP:
		{
			u32		l	= vm_consume_byte(vm);
			u32		r	= vm_consume_byte(vm);
			u32		jump	= CHUNK_CONST_16_GET(l, r);
			
			while (jump--)
				vm_consume_byte(vm);
		} break;
		case OP_LOOP:
		{
			u32		l	= vm_consume_byte(vm);
			u32		r	= vm_consume_byte(vm);
			u32		jump	= CHUNK_CONST_16_GET(l, r);
			
			vm_revert_state(vm, jump);
		} break;
		case OP_POP_MANY:
		{
			u8	count = vm_consume_byte(vm);

			if (!vm_pop_many(vm, count))
				VM_ERROR(vm, "Could not pop %d variables", count);
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
				VM_ERROR(vm, "Undefined variable %s", name.as.string->buf);

			vm_push(vm, *val);
		} break;
		case OP_SET_GLOBAL:
		{
			value_t		name	= *vm_pop(vm);
			value_t*	val	= value_map_get(globals, name);
			
			if (!val)
				VM_ERROR(vm, "Undefined variable %s", name.as.string->buf);

			*val = *vm_pop(vm);
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
				VM_ERROR(vm, "Operand cannot be an object");
			}
		} break;
		case OP_GET_LOCAL	: vm_push(vm, vm->stack[vm_consume_byte(vm)]); break;
		case OP_SET_LOCAL	: vm->stack[vm_consume_byte(vm)] = *(vm->stack_top - 1); break;
		case OP_ZERO		: vm_push(vm, VAL_AS_INT(0)); break;
		case OP_EQ		: VM_EQ(vm, ==); break;
		case OP_NOT_EQ		: VM_EQ(vm, !=); break;
		case OP_MORE		: VM_COMPARE(vm, >); break;
		case OP_LESS		: VM_COMPARE(vm, <); break;
		case OP_MORE_EQ		: VM_COMPARE(vm, >=); break;
		case OP_LESS_EQ		: VM_COMPARE(vm, <=); break;
		case OP_NEG		: VM_UOP_NB(vm, -); break;
		case OP_INC		: VM_UOP_NB(vm, 1+); break;
		case OP_DEC		: VM_UOP_NB(vm, -1+); break;
		case OP_ADD		: VM_BOP(vm, +); break;
		case OP_SUB		: VM_BOP(vm, -); break;
		case OP_MUL		: VM_BOP(vm, *); break;
		case OP_DIV		: VM_BOP(vm, /); break;
		case OP_OR		: VM_BITOP(vm, |); break;
		case OP_XOR		: VM_BITOP(vm, ^); break;
		case OP_AND		: VM_BITOP(vm, &); break;
		case OP_NAN		: vm_push(vm, VAL_AS_NB(NAN)); break; 
		case OP_INF		: vm_push(vm, VAL_AS_NB(INFINITY)); break;
		case OP_TRUE		: vm_push(vm, VAL_AS_BOOL(1)); break;
		case OP_FALSE		: vm_push(vm, VAL_AS_BOOL(0)); break;
		case OP_PRINT		: value_print(*vm_pop(vm)); printf("\n"); break;
		case OP_POP		: vm_pop(vm); break;
		case OP_RET		: return VM_RES_OK;
		}
	}

	return VM_RES_RUNTIME_ERR;
}

#ifdef TESTER


#endif	// TESTER
