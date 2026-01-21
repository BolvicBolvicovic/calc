#ifndef SWISSMAP_H
#define SWISSMAP_H

#include <c_types.h>
#include <string.h>

// TODO: write documentation
/**/
#define SWISS_EMPTY	0x80
#define SWISS_DELETED	0xFE
#define SWISS_H2_MASK	0x7F

#define SWISSMAP_DECLARE(name, K_t, V_t)	\
typedef struct name	name;			\
struct name					\
{						\
	u8*	ctrl;				\
	K_t*	keys;				\
	V_t*	values;				\
	u64	capacity;			\
	u64	size;				\
};

static inline u64
swiss_h1(u64 hash, u64 cap)
{
	return (hash >> 7) & (cap - 1);
}

static inline u8
swiss_h2(u64 hash)
{
	return (u8)(hash & SWISS_H2_MASK);
}

static inline void
swiss_init_ctrl(u8* ctrl, u64 cap)
{
	memset(ctrl, SWISS_EMPTY, cap);
}

static inline u8
swiss_match_h2(u8* ctrl, u8 h2, u64 base)
{
	u8	mask = 0;

	for (u32 i = 0; i < 8; i++)
		if (ctrl[base + i] == h2)
			mask |= (1u << i);

	return mask;
}

static inline s32
swiss_find_empty(u8* ctrl, u64 base)
{
	for (u32 i = 0; i < 8; i++)
		if (ctrl[base + i] == SWISS_EMPTY)
			return i;

	return -1;
}

static inline u64
djb2_hash(char* str)
{
	u64	hash = 5381;
	s32	c;

	while ((c = *str++))
		hash = ((hash << 5) + hash) + c;

	return hash;
}

static inline s32
strcmp_wrapper(char* s1, char* s2)
{
	return strcmp(s1, s2) == 0;
}

// Note: You need to #include <arena.h> to use this macro
#define SWISSMAP_DEFINE_NEW(name, K_t, V_t)			\
name*								\
name##_new(arena_t* arena, u64 capacity)			\
{								\
	name*	map	= ARENA_PUSH_STRUCT(arena, name);	\
								\
	map->ctrl	= ARENA_PUSH_ARRAY(arena, u8, capacity);\
	map->keys	= ARENA_PUSH_ARRAY(arena, K_t, capacity);\
	map->values	= ARENA_PUSH_ARRAY(arena, V_t, capacity);\
	map->capacity	= capacity;				\
	map->size	= 0;					\
								\
	swiss_init_ctrl(map->ctrl, capacity);			\
								\
	return map;						\
}

#define SWISSMAP_DEFINE_GET(name, K_t, V_t, hash_fn, eq_fn)	\
V_t*								\
name##_get(name* map, K_t key)					\
{								\
	u64	hash	= hash_fn(key);				\
	u8	h2	= swiss_h2(hash);			\
	u64	mask	= map->capacity - 1;			\
	u64	group	= swiss_h1(hash, map->capacity);	\
								\
	while (1)						\
	{							\
		u8	m = swiss_match_h2(map->ctrl, h2, group);\
								\
		while (m)					\
		{						\
			s32	i	= __builtin_ctz(m);	\
			u64	idx	= (group + i) & mask;	\
								\
			if (eq_fn(map->keys[idx], key))		\
				return &map->values[idx];	\
								\
			m &= m - 1;				\
		}						\
								\
		if (swiss_find_empty(map->ctrl, group) != -1)	\
			return 0;				\
								\
		group = (group + 8) & mask;			\
	}							\
}

#define SWISSMAP_DEFINE_PUT(name, K_t, V_t, hash_fn, eq_fn)	\
void								\
name##_put(name* map, K_t key, V_t value)			\
{								\
	u64	hash	= hash_fn(key);				\
	u8	h2	= swiss_h2(hash);			\
	u64	mask	= map->capacity - 1;			\
	u64	group	= swiss_h1(hash, map->capacity);	\
								\
	while (map->size < map->capacity)			\
	{							\
		u8	m = swiss_match_h2(map->ctrl, h2, group);\
								\
		while (m)					\
		{						\
			s32	i	= __builtin_ctz(m);	\
			u64	idx	= (group + i) & mask;	\
								\
			if (eq_fn(map->keys[idx], key))		\
			{					\
				map->values[idx] = value;	\
				return;				\
			}					\
								\
			m &= m - 1;				\
		}						\
								\
		s32	empty	= swiss_find_empty(map->ctrl, group);	\
								\
		if (empty != -1)				\
		{						\
			u64	idx	= (group + empty) & mask;	\
								\
			map->ctrl[idx]	= h2;			\
			map->keys[idx]	= key;			\
			map->values[idx]= value;		\
			map->size++;				\
								\
			return;					\
		}						\
								\
		group = (group + 8) & mask;			\
	}							\
}

#define SWISSMAP_DEFINE_FUNCTIONS(name, K_t, V_t, hash_fn, eq_fn)\
SWISSMAP_DEFINE_NEW(name, K_t, V_t)			\
SWISSMAP_DEFINE_GET(name, K_t, V_t, hash_fn, eq_fn)	\
SWISSMAP_DEFINE_PUT(name, K_t, V_t, hash_fn, eq_fn)

#define SWISSMAP_DEFINE_FUNCTIONS_DEFAULT(name, V_t)	\
SWISSMAP_DEFINE_FUNCTIONS(name, char*, V_t, djb2_hash, strcmp_wrapper)

#define SWISSMAP_DEFINE_FUNCTIONS_DEFAULT_CUSTOM_FUNC(name, V_t, hash_fn, eq_fn)\
SWISSMAP_DEFINE_FUNCTIONS(name, char*, V_t, hash_fn, eq_fn)

#define SWISSMAP_DECLARE_FUNCTIONS(name, K_t, V_t)	\
name*	name##_new(arena_t* arena, u64 capacity);	\
void	name##_put(name* map, K_t key, V_t value);	\
V_t*	name##_get(name* map, K_t key);

#define SWISSMAP_DECLARE_DEFAULT(name, V_t)		\
SWISSMAP_DECLARE(name, char*, V_t)			\
SWISSMAP_DECLARE_FUNCTIONS(name, char*, V_t)


#ifdef TESTER

void	test_swissmap(void);

#endif // TESTER

#endif // SWISSMAP_H
