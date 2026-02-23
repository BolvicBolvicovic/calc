#ifndef CHUNK_H
#define CHUNK_H

#include <c_types.h>
#include <arena.h>
#include <vm/value.h>
#include <vm/context.h>

#define CHUNK_LINE_NUMBER		0xffff0000
#define CHUNK_LINE_COUNT		0x0000ffff
#define CHUNK_LINE_NUMBER_GET(x)	(x >> 16)
#define CHUNK_LINE_NUMBER_PUT(x)	(x << 16)
#define CHUNK_CONST_16_GET(l, r)	(u32)(((u32)(l) << 8) | ((u32)(r)))

typedef enum op_code_t	op_code_t;
enum __attribute__((packed)) op_code_t
{
	// Variables
	OP_CONST_8,		// 0
	OP_CONST_16,		// 1
	OP_DEFINE_GLOBAL,	// 2
	OP_GET_GLOBAL,		// 3
	OP_GET_LOCAL,		// 4
	OP_SET_GLOBAL,		// 5
	OP_SET_LOCAL,		// 6
	OP_ZERO,		// 7
	OP_TRUE,		// 8
	OP_FALSE,		// 9
	OP_NAN,			// 10
	OP_INF,			// 11

	// Control flow
	OP_JMPF,		// 12
	OP_JMP,			// 13
	OP_LOOP,		// 14

	// Operators
	OP_INC,			// 15
	OP_DEC,			// 16
	OP_NEG,			// 17
	OP_NOT,			// 18
	OP_EQ,			// 19
	OP_MORE,		// 20
	OP_LESS,		// 21
	OP_NOT_EQ,		// 22
	OP_MORE_EQ,		// 23
	OP_LESS_EQ,		// 24
	OP_ADD,			// 25
	OP_SUB,			// 26
	OP_MUL,			// 27
	OP_DIV,			// 28
	OP_XOR,			// 29
	OP_AND,			// 30
	OP_OR,			// 31

	// Helper
	OP_PRINT,		// 32
	OP_POP,			// 33
	OP_POP_MANY,		// 34
	OP_RET,			// 35
};

typedef struct chunk_t	chunk_t;
struct chunk_t
{
	u32		capacity;
	u32		count;
	u32		line_idx;
	u32		id;
	chunk_t*	prev;
	chunk_t*	next;
	u8*		code;
	u8*		end;
	u32*		lines;
	context_t*	context;
};

u8		chunk_read(chunk_t*, u32 index);
chunk_t*	chunk_new(context_t*, u32 capacity);
chunk_t*	chunk_write(arena_t*, chunk_t*, u8 byte, u32 line);
chunk_t*	chunk_write_const(arena_t*, chunk_t*, value_t constant, u32 line);
void		chunk_dissassemble(chunk_t*);
u32		chunk_index(chunk_t* chunk);
void		chunk_write_at(chunk_t* chunk, u8 byte, u32 idx);

#ifdef TESTER

void	test_chunk_write(void);
void	test_chunk_write_const(void);

#endif	// TESTER

#endif	// CHUNK_H
