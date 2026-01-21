#ifdef TESTER

#include <swissmap.h>
#include <assert.h>
#include <arena.h>

SWISSMAP_DECLARE_DEFAULT(floatmap, float)
SWISSMAP_DEFINE_FUNCTIONS_DEFAULT(floatmap, float)

void
test_swissmap(void)
{
	arena_t*	arena	= ARENA_ALLOC();
	s32		sm_cap	= 1;
	floatmap*	sm	= floatmap_new(arena, sm_cap);
	char*		test	= "test\0";
	f64		value	= 42;

	assert(sm);

	assert(floatmap_get(sm, test) == 0);

	floatmap_put(sm, test, value);

	assert(floatmap_get(sm, test) != 0);
	assert(*floatmap_get(sm, test) == 42);
	
	char*	lit = ARENA_PUSH_ARRAY(arena, char, 5);

	sm_cap	= 16;
	sm	= floatmap_new(arena, sm_cap);

	memcpy(lit, test, 5);

	for (s32 i = 0; i <= sm_cap; i++)
	{
		lit[1] += i;

		char*	key = ARENA_PUSH_ARRAY(arena, char, 5);

		memcpy(key, lit, 5);
		floatmap_put(sm, key, (f64)i);

		if (i == sm_cap)
			assert(floatmap_get(sm, lit) == 0);
		else
		{
			assert(floatmap_get(sm, lit) != 0);
			assert(*floatmap_get(sm, lit) == i);
		}
	}

	arena_release(arena);
}

#endif // TESTER
