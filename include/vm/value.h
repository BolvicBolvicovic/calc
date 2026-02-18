#ifndef VALUE_H
#define VALUE_H

#include <c_types.h>
#include <arena.h>
#include <swissmap.h>

#define VAL_AS_BOOL(value)	((value_t){.type=VAL_BOOL, .as={.boolean=(value)}})
#define VAL_AS_NB(value)	((value_t){.type=VAL_NB, .as={.number=(value)}})
#define VAL_AS_INT(value)	((value_t){.type=VAL_INT, .as={.integer=(value)}})
#define VAL_AS_STR(value)	((value_t){.type=VAL_STR, .as={.string=(value)}})

typedef enum value_type_t value_type_t;
enum value_type_t
{
	VAL_INT,
	VAL_NB,
	VAL_STR,
	VAL_BOOL,
	VAL_ARR,
};

struct value_array_t;
typedef struct array_t array_t;
struct array_t
{
	struct value_array_t*	head;
	struct value_array_t*	tail;
};

typedef struct string_t string_t;
struct string_t
{
	char*		buf;
	u64		size;	// Full string
	u64		length;	// Current chunk
	string_t*	next;
	string_t*	tail;
};

typedef	struct value_t value_t;
struct value_t
{
	value_type_t	type;
	union
	{
		bool		boolean;
		f64		number;
		s64		integer;
		array_t*	array;
		string_t*	string;
	} as;
};

typedef struct value_array_t value_array_t;
struct value_array_t
{
	u32		id;
	u32		capacity;
	u32		count;
	value_array_t*	next;
	value_array_t*	prev;
	value_t*	values;
};

SWISSMAP_DECLARE(value_map, value_t, value_t)
SWISSMAP_DECLARE(string_set, string_t*, u32)

SWISSMAP_DECLARE_FUNCTIONS(value_map, value_t, value_t)
SWISSMAP_DECLARE_FUNCTIONS(string_set, string_t*, u32)

value_array_t*	value_array_new(arena_t*, u32 capacity);
value_array_t*	value_array_write(arena_t*, value_array_t*, value_t);
value_t		value_array_read(value_array_t*, u32 index);
u32		value_array_index(value_array_t*);
void		value_print(value_t);
value_t*	value_copy(arena_t*, value_t*);
bool		string_cmp(string_t* s1, string_t* s2);
string_t*	string_concat(arena_t* arena, string_t* s1, string_t* s2);
string_t*	string_new(arena_t* arena, char* src, u32 size);
string_t*	string_copy(arena_t* arena_t, string_t* str);

#ifdef TESTER

void	test_value_array(void);

#endif	// TESTER

#endif	// VALUE_H
