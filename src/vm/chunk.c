#include <vm/chunk.h>
#include <stdio.h>
#include <assert.h>

u8
chunk_read(chunk_t* chunk, u32 index)
{
	u32	capacity	= chunk->capacity;
	u32	skip_chunk	= index / capacity;

	index -= capacity * skip_chunk;

	while (chunk && chunk->id < skip_chunk)
		chunk = chunk->next;

	assert(chunk);
	
	return chunk->code[index];

}

chunk_t*
chunk_write(arena_t* arena, chunk_t* chunk, u8 byte, u32 line)
{
	assert(arena);
	assert(chunk);

	u32	capacity = chunk->capacity;

	while(chunk->next)
		chunk = chunk->next;

	if (chunk->count >= capacity)
	{
		chunk_t*	new_chunk = ARENA_PUSH_STRUCT(arena, chunk_t);

		new_chunk->code		= ARENA_PUSH_ARRAY(arena, u8, capacity);
		new_chunk->lines	= ARENA_PUSH_ARRAY(arena, u32, capacity);
		new_chunk->capacity	= capacity;
		new_chunk->prev		= chunk;
		new_chunk->context	= chunk->context;
		new_chunk->id		= chunk->id + 1;
		chunk->next		= new_chunk;
		chunk			= new_chunk;
	}

	u32	line_idx= chunk->line_idx;
	u32*	lines	= chunk->lines;

	chunk->code[chunk->count] = byte;
	chunk->count++;

	if (CHUNK_LINE_NUMBER_GET(lines[line_idx]) == line)
		lines[line_idx] += 1;
	else
	{
		if (lines[line_idx])
			chunk->line_idx++;
		lines[chunk->line_idx] = CHUNK_LINE_NUMBER_PUT(line) + 1;
	}

	return chunk;
}

chunk_t*
chunk_write_const(arena_t* arena, chunk_t* chunk, value_t constant, u32 line)
{
	value_array_t*	constants = chunk->context->constants;

	value_array_write(arena, constants, constant);

	u32	index = value_array_index(constants);

	if (index <= UINT8_MAX)
	{
		chunk = chunk_write(arena, chunk, OP_CONST_8, line);
		chunk = chunk_write(arena, chunk, (u8)index, line);
	}
	else
	{
		chunk = chunk_write(arena, chunk, OP_CONST_16, line);
		chunk = chunk_write(arena, chunk, (u8)(index >> 8), line);
		chunk = chunk_write(arena, chunk, (u8)index, line);
	}
	
	return chunk;
}

chunk_t*
chunk_new(context_t* context, u32 capacity)
{
	arena_t*	arena = context->arena_line;
	chunk_t*	chunk = ARENA_PUSH_STRUCT(arena, chunk_t);

	chunk->code	= ARENA_PUSH_ARRAY(arena, op_code_t, capacity);
	chunk->lines	= ARENA_PUSH_ARRAY(arena, u32, capacity);
	chunk->capacity	= capacity;
	chunk->context	= context;

	return chunk;
}


static u32
op_code_disassemble(chunk_t* chunk, u32 index, u32 offset)
{
	value_array_t*	constants = chunk->context->constants;

	switch (chunk->code[index])
	{
	case OP_CONST_8:
	{
		u8	const_idx = chunk_read(chunk, offset + 1);

		printf("OP_CONST_8  %5d '", const_idx);
		value_print(value_array_read(constants, const_idx));
		printf("'\n");
		return 1;
	}
	case OP_CONST_16:
	{
		u32	const_idx =
			CHUNK_CONST_16_GET(chunk_read(chunk, offset + 1), chunk_read(chunk, offset + 2));

		printf("OP_CONST_16 %5d '", const_idx);
		value_print(value_array_read(constants, const_idx));
		printf("'\n");
		return 2;
	}
	case OP_DEFINE_GLOBAL	: printf("OP_DEFINE_GLOBAL\n"); return 0;
	case OP_GET_GLOBAL	: printf("OP_GET_GLOBAL\n"); return 0;
	case OP_NAN		: printf("OP_NAN\n"); return 0;
	case OP_INF		: printf("OP_INF\n"); return 0;
	case OP_TRUE		: printf("OP_TRUE\n"); return 0;
	case OP_FALSE		: printf("OP_FALSE\n"); return 0;
	case OP_NEG		: printf("OP_NEG\n"); return 0;
	case OP_EQ		: printf("OP_EQ\n"); return 0;
	case OP_NOT		: printf("OP_NOT\n"); return 0;
	case OP_MORE		: printf("OP_MORE\n"); return 0;
	case OP_LESS		: printf("OP_LESS\n"); return 0;
	case OP_NOT_EQ		: printf("OP_NOT_EQ\n"); return 0;
	case OP_MORE_EQ		: printf("OP_MORE_EQ\n"); return 0;
	case OP_LESS_EQ		: printf("OP_LESS_EQ\n"); return 0;
	case OP_ADD		: printf("OP_ADD\n"); return 0;
	case OP_SUB		: printf("OP_SUB\n"); return 0;
	case OP_MUL		: printf("OP_MUL\n"); return 0;
	case OP_DIV		: printf("OP_DIV\n"); return 0;
	case OP_PRINT		: printf("OP_PRINT\n"); return 0;
	case OP_POP		: printf("OP_POP\n"); return 0;
	case OP_RET		: printf("OP_RET\n"); return 0;
	default			: printf("Unknown OP '%u'\n", chunk->code[offset]);
	}
	
	return 0;
}

void
chunk_dissassemble(chunk_t* chunk)
{
	u32	index		= 0;
	u32	line_current	= 0;
	u32	i		= 0;

	printf("**Dissassemble chunks**\n\n");
	printf("Byte Line Ope Idx Val\n");

	while (chunk)
	{
		u32	line		= CHUNK_LINE_NUMBER_GET(chunk->lines[0]);
		u32	line_count	= chunk->lines[0] & CHUNK_LINE_COUNT;
		u32	line_idx	= 0;

		printf("\nChunk id-%d\n", chunk->id);
		
		if (i > 0)
		{
			line_current = i;

			while (line_current >= line_count)
			{
				line_idx++;
				line		= CHUNK_LINE_NUMBER_GET(chunk->lines[line_idx]);
				line_current	-= line_count;
				line_count	= chunk->lines[line_idx] & CHUNK_LINE_COUNT;
			}
		}

		for (; i < chunk->count; i++)
		{
			u32	offset	= i + index;
			u32	inc	= 0;

			printf("%04u ", offset);

			if (!line_current)
				printf("%4d ", line);
			else if (line_current < line_count)
				printf("%3s| ", "");
			
			inc		+= op_code_disassemble(chunk, i, offset);
			i		+= inc;
			line_current	+= inc + 1;
			
			if (line_current >= line_count)
			{
				line_idx++;
				line		= CHUNK_LINE_NUMBER_GET(chunk->lines[line_idx]);
				line_current	-= line_count;
				line_count	= chunk->lines[line_idx] & CHUNK_LINE_COUNT;
			}
		}

		if (chunk->count != chunk->capacity)
			return;

		index	+= chunk->capacity;
		i	-= chunk->capacity;
		chunk	=  chunk->next;
	}
}

#ifdef TESTER

void
test_chunk_write(void)
{
	arena_t*	arena = ARENA_ALLOC();
	chunk_t*	chunk = &(chunk_t)
	{
		.capacity=10,
		.count=0,
		.prev=0,
		.code=ARENA_PUSH_ARRAY(arena, op_code_t, 10),
		.constants=0,
		.lines=ARENA_PUSH_ARRAY(arena, u32, 10),
	};

	for (u32 i = 0; i < chunk->capacity; i++)
		chunk = chunk_write(arena, chunk, OP_RET, 0);

	assert(chunk->capacity == chunk->count);
	assert(chunk->prev == 0);
	assert(chunk->next == 0);
	assert(chunk->lines[0] == 10);

	for (u32 i = 0; i < chunk->capacity; i++)
		assert(chunk->code[i] == OP_RET);
	

	arena_release(arena);
}

void
test_chunk_write_const(void)
{
	u32		capct = 500;
	arena_t*	arena = ARENA_ALLOC();
	value_array_t*	array = value_array_new(arena, capct);
	chunk_t*	chunk = &(chunk_t)
	{
		.capacity=capct,
		.count=0,
		.prev=0,
		.code=ARENA_PUSH_ARRAY(arena, op_code_t, capct),
		.constants=array,
		.lines=ARENA_PUSH_ARRAY(arena, u32, capct),
	};

	for (u32 i = 0; i < capct; i++)
		chunk_write_const(arena, chunk, i, i);

	// TODO: check lines and bytes
	//u32	chunk_idx = 0;
	for (u32 i = 0; i <= UINT8_MAX; i++)
	{
		value_t	val = value_array_read(array, i);

		assert(val == i);
	}
	
	for (u32 i = UINT8_MAX + 1; i < capct; i++)
	{
		value_t	val = value_array_read(array, i);

		assert(val == i);
	}

	arena_release(arena);
}

#endif
