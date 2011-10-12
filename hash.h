#ifndef _HASH_H_
#define _HASH_H_

/** <config> **/
#include <stddef.h>
#include <inttypes.h>
typedef uint32_t hkey;

#ifndef HASH_HASH_FUNC
#define HASH_HASH_FUNC jenkins_oneatatime
#endif 
#define HASH_FIND_FUNC hash_path_trie_get
#define HASH_PATH_FUNC hash_path_trie_set
#ifndef HTRIE_BIT_SPAN
#define HTRIE_BIT_SPAN 1	/* 1, 2, 4 or 8 */
#endif
#define HTABLE_ENTRIES 256
/** </config> **/

typedef struct hash_t hash_t;

struct hash_t {
	hash_t* tab;
	int span;
	hkey key;
	void* user;
};

/* Generic hash API */
extern hash_t* hash_new();
extern void  hash_init(hash_t* tree, int span);
extern void  hash_done(hash_t* tree);
extern void    hash_free(hash_t* tree);
extern void* hash_kget(hash_t* tree, uint32_t key);
extern void  hash_kset(hash_t* tree, uint32_t key, void* value);
inline void* hash_get(hash_t* tree, const char* strkey);
inline void  hash_set(hash_t* tree, const char* strkey, void* value);
inline hash_t* hash_find(hash_t* root, uint32_t key, int make);

extern uint32_t HASH_HASH_FUNC (const char* key, size_t len);

#ifdef DEBUG_HASH /* XXX */
extern void hash_report();
#endif

#endif
