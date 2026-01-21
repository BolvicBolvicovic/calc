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
	floatmap*	sm	= floatmap_new(arena, 10);
	char*		test	= "test";
	f64		value	= 42;

	assert(sm);

	assert(floatmap_get(sm, test) == 0);

	floatmap_put(sm, test, value);

	assert(*floatmap_get(sm, test) == 42);

	arena_release(arena);
}

#endif // TESTER
