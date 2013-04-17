#ifndef __STASH_H_
#define __STASH_H_

#include "hash.h"

#define STASH_MIN_SPAN 96

#ifndef byte
typedef uint8_t byte;
#endif

typedef struct stash_t stash_t;
typedef struct stash_entry stash_entry;

struct stash_t {
	hash_t *tab;
	stash_entry *table;
	int span;
	int fill;
}; 

struct stash_entry {
	byte *data;
	int key_size;
	int val_size;
	stash_entry *next;
};

/* Generic stash API */
extern stash_t* stash_new();
extern void stash_free(stash_t* node);

extern byte* stash_get(stash_t *st, const char *addr, int addr_len, int *size);
extern byte* stash_peek(stash_t *st, const char *addr, int addr_len);
extern void stash_put(stash_t* node, const char *path, int path_len, byte* var, int size);
extern void stash_del(stash_t* node, const char *path, int path_len);

/* Helpers */
extern int   stash_unsafe_int;
extern byte* stash_unsafe_ptr;
extern int*  stash_static_int(int Num);
#ifndef MAX
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#endif
#define H_INT(A) ((byte*)stash_static_int((A)))
#define H_INV(A) ((byte*)& (A))
#define H_PTR(A) ((byte*) (A))
#define H_REV(A, T) *((T*)A)

/* Wrappers -- put/peek/cpy data/asciiz_strings/integer/const_integer */
#define H_PUT(NODE, KEY, DATA)  stash_put((NODE), (KEY), strlen((KEY)) + 1, (DATA), sizeof((DATA)))
#define H_PUTs(NODE, KEY, STR)  stash_put((NODE), (KEY), strlen((KEY)) + 1, (STR), strlen((STR)))
#define H_PUTi(NODE, KEY, VAL)  stash_put((NODE), (KEY), strlen((KEY)) + 1, H_INT(VAL), sizeof(int))
#define H_PUTv(NODE, KEY, VAL)  stash_put((NODE), (KEY), strlen((KEY)) + 1, H_INV(VAL), sizeof(int))

#define H_GET(NODE, KEY)  stash_get((NODE), (KEY), strlen((KEY)) + 1, &stash_unsafe_int)
#define H_GETs(NODE, KEY) H_GET(NODE, KEY)
#define H_GETi(NODE, KEY) (int)*(H_GET((NODE), (KEY)))

#define H_READ(DST, NODE, KEY) do { \
	stash_unsafe_ptr = stash_get((NODE), (KEY), strlen((KEY)) + 1, &stash_unsafe_int); \
	if (stash_unsafe_ptr != NULL) \
		memcpy((DST), stash_unsafe_ptr, stash_unsafe_int); \
	else \
		(DST)[0] = '\0'; \
	} while(0)

#define H_READs(DST, NODE, KEY) do { \
	H_READ(DST, NODE, KEY); \
	(DST)[ stash_unsafe_int ] = '\0'; \
	} while(0)

#define H_READi(DST, NODE, KEY) (DST) = H_GETi(NODE, KEY)

#ifdef DEBUG_HASH /* XXX */
extern void stash_report();
#endif

#endif
