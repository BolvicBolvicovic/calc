#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <macros.h>
#include <vm/value.h>

static u64	value_hash(value_t);
static bool	value_cmp(value_t, value_t);
static u64	string_hash(string_t* str);

SWISSMAP_DEFINE_FUNCTIONS(value_map, value_t, value_t, value_hash, value_cmp)
SWISSMAP_DEFINE_FUNCTIONS(string_set, string_t*, u32, string_hash, string_cmp)

static u64
string_hash(string_t* str)
{
	u8*		buf;
	u8*		end;
	u64		hash= 5381;
	
	while (str)
	{
		buf = (u8*)str->buf;
		end = buf + str->length;

		while (buf + 4 <= end)
		{
			hash = ((hash << 5) + hash) + buf[0];
			hash = ((hash << 5) + hash) + buf[1];
			hash = ((hash << 5) + hash) + buf[2];
			hash = ((hash << 5) + hash) + buf[3];

			buf += 4;
		}

		while (buf < end)
			hash = ((hash << 5) + hash) + *buf++;

		str = str->next;
	}

	return hash;
}

static u64
value_hash(value_t val)
{
	switch (val.type)
	{
	case VAL_NB	: 
	{
		u64	x;
		memcpy(&x, &val.as.number, sizeof(u64));
		return swiss_splitmix64(x);
	}
	case VAL_INT	: return swiss_splitmix64(val.as.integer);
	case VAL_STR	: return string_hash(val.as.string);
	default: ERROR(-1, "Cannot use boolean or array as key"); exit(70);
	}

	return 0;
}

static bool
value_cmp(value_t val1, value_t val2)
{
	if (val1.type != val2.type)
		return 0;

	switch (val1.type)
	{
	case VAL_NB	: return val1.as.number == val2.as.number;
	case VAL_INT	: return val1.as.integer == val2.as.integer;
	case VAL_STR	: return string_cmp(val1.as.string, val2.as.string);
	default		: return 0;
	}
}

inline value_array_t*
value_array_new(arena_t* arena, u32 capacity)
{
	assert(arena);

	value_array_t*	array = ARENA_PUSH_STRUCT(arena, value_array_t);

	array->capacity = capacity;
	array->values	= ARENA_PUSH_ARRAY(arena, value_t, capacity);

	return array;
}


value_array_t*
value_array_write(arena_t* arena, value_array_t* array, value_t value)
{
	assert(arena);
	assert(array);
	
	u32	capacity = array->capacity;

	while (array->next)
		array = array->next;

	if (array->count == capacity)
	{
		value_array_t*	new_array = ARENA_PUSH_STRUCT(arena, value_array_t);
		
		new_array->capacity	= capacity;
		new_array->values	= ARENA_PUSH_ARRAY(arena, value_t, capacity);
		new_array->prev		= array;
		new_array->id		= array->id + 1;
		array->next		= new_array;
		array			= new_array;
	}

	array->values[array->count] = value;
	array->count++;

	return array;
}

value_t
value_array_read(value_array_t* array, u32 index)
{
	u32	capacity	= array->capacity;
	u32	skip_array	= index / capacity;

	index -= capacity * skip_array;

	while (array && array->id < skip_array)
		array = array->next;

	assert(array);
	
	return array->values[index];
}

inline u32
value_array_index(value_array_t* array)
{
	while (array->next)
		array = array->next;

	return array->id * array->capacity + array->count - 1;
}

inline void
value_print(value_t value)
{
	switch (value.type)
	{
	case VAL_NB	: printf("%g", value.as.number); break;
	case VAL_BOOL	: printf("%s", value.as.boolean ? "true" : "false"); break;
	case VAL_INT	: printf("%ld", value.as.integer); break;
	case VAL_STR	: 
	{
		string_t*	str = value.as.string;

		while (str)
		{
			printf("%s", str->buf);
			str = str->next;
		}
	} break;
	case VAL_ARR	:
	{
		value_array_t*	array = value.as.array->head;

		if (!array)
			return;

		printf("{ ");

		while (array)
		{
			for (u32 i = 0; i < array->count; i++)
			{
				value_print(array->values[i]);
				printf(", ");
			}

			array = array->next;
		}

		printf("}");
	} break;
	}
}

inline value_t*
value_copy(arena_t* arena, value_t* value)
{
	value_t*	new_val = ARENA_PUSH_STRUCT(arena, value_t);

	memcpy(new_val, value, sizeof(value_t));
	return new_val;
}

bool
string_cmp(string_t* s1, string_t* s2)
{
	if (s1->size != s2->size)
		return 0;
	
	string_t*	s1_current	= s1;
	string_t*	s2_current	= s2;
	u32		s1_len		= s1->length;
	u32		s2_len		= s2->length;
	char*		s1_buf		= s1->buf;
	char*		s2_buf		= s2->buf;
	u32		size		= s1->size;
	u32		i		= 0;

	while (i < size && s1_current && s2_current)
	{
		u32	smallest = s1_len < s2_len ? s1_len : s2_len;

		if (memcmp(s1_buf, s2_buf, smallest) != 0)
			return 0;

		i+= smallest;

		if (s1_len == smallest)
		{
			s1_current	= s1_current->next;

			if (!s1_current)
				break;

			s1_len		= s1_current->length;
			s1_buf		= s1_current->buf;
		}
		else
		{
			s1_len -= smallest;
			s1_buf += smallest;
		}
		
		if (s2_len == smallest)
		{
			s2_current	= s2_current->next;
			
			if (!s2_current)
				break;
			
			s2_len		= s2_current->length;
			s2_buf		= s2_current->buf;
		}
		else
		{
			s2_len -= smallest;
			s2_buf += smallest;
		}
	}

	return i == size;
}

string_t*
string_copy(arena_t* arena, string_t* str)
{
	string_t*	str_new = string_new(arena, str->buf, str->size);
	string_t*	tmp	= str_new;

	str_new->length	= str->length;
	str		= str->next;

	while (str)
	{
		tmp->next	= string_new(arena, str->buf, str->length);
		str		= str->next;
		tmp		= tmp->next;
	}

	return str_new;
}

string_t*
string_concat(arena_t* arena, string_t* s1, string_t* s2)
{
	string_t*	str1 = string_copy(arena, s1);
	string_t*	str2 = string_copy(arena, s2);
	string_t*	tmp = str1;

	tmp->size += str2->size;

	while (tmp->next)
		tmp = tmp->next;

	tmp->next = str2;

	return str1;
}

inline string_t*
string_new(arena_t* arena, char* src, u32 size)
{
	char*		buf = ARENA_PUSH_ARRAY(arena, char, size + 1);
	string_t*	str = ARENA_PUSH_STRUCT(arena, string_t);
	
	memcpy(buf, src, size);
	str->size	= size;
	str->length	= size;
	str->buf	= buf;

	return str;
}

#ifdef TESTER

void
test_value_array(void)
{
	arena_t*	arena = ARENA_ALLOC();
	value_array_t*	array = value_array_new(arena, 10);

	for (u32 i = 0; i < array->capacity; i++)
		array = value_array_write(arena, array, i);

	assert(array);
	assert(array->capacity == array->count);

	for (u32 i = 0; i < array->capacity; i++)
		assert(value_array_read(array, i) == i);
	

	arena_release(arena);
}

#endif
