#ifndef CHUNK_H
#define CHUNK_H

#include <c_types.h>
#include <arena.h>
#include <vm/value.h>

#define CHUNK_LINE_NUMBER		0xffff0000
#define CHUNK_LINE_COUNT		0x0000ffff
#define CHUNK_LINE_NUMBER_GET(x)	(x >> 16)
#define CHUNK_LINE_NUMBER_PUT(x)	(x << 16)
#define CHUNK_CONST_16_GET(l, r)	(u32)(((u32)(l) << 8) | ((u32)(r)))

typedef enum op_code_t	op_code_t;
enum __attribute__((packed)) op_code_t
{
	OP_CONST_8,
	OP_CONST_16,
	OP_DEFINE_GLOBAL,
	OP_GET_GLOBAL,
	OP_ZERO,
	OP_TRUE,
	OP_FALSE,
	OP_NAN,
	OP_INF,
	OP_NEG,
	OP_NOT,
	OP_EQ,
	OP_MORE,
	OP_LESS,
	OP_NOT_EQ,
	OP_MORE_EQ,
	OP_LESS_EQ,
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_PRINT,
	OP_POP,
	OP_RET,
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
	value_array_t*	constants;
	string_set*	strings;
	u32*		lines;
};

u8		chunk_read(chunk_t* chunk, u32 index);
chunk_t*	chunk_new(arena_t*, u32 capacity);
chunk_t*	chunk_write(arena_t*, chunk_t*, u8 byte, u32 line);
chunk_t*	chunk_write_const(arena_t* arena, chunk_t* chunk, value_t constant, u32 line);
void		chunk_dissassemble(chunk_t*);

#ifdef TESTER

void	test_chunk_write(void);
void	test_chunk_write_const(void);

#endif	// TESTER

#endif	// CHUNK_H
