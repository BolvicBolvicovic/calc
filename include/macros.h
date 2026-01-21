#ifndef MACROS_H
#define MACROS_H

#define PROMPT	"calc> "
#define	ALIGN(x, b)		(((x) + (b) - 1)&(~((b) - 1)))
#define ALIGN_DOWN(x, b)	((x)&(~((b) - 1)))
#define MIN(a, b)		((a) < (b) ? (a) : (b))
#define MAX(a, b)		((a) > (b) ? (a) : (b))
#define CLAMP_TOP(a, x)		MIN(a, x)
#define CLAMP_BOT(x, b)		MAX(x, b)

#endif
