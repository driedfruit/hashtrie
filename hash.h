#ifndef _HASH_H_
#define _HASH_H_

/** <config> **/
#include <inttypes.h>
typedef uint32_t hkey;

#define HASH_HASH_FUNC jenkins_one_at_a_time_hash
#define HASH_PATH_FUNC hash_path_trie
#define HTRIE_BIT_SPAN 8	/* 1, 2, 4 or 8 */
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
extern void hash_init(hash_t* tree, int span);
extern void hash_done(hash_t *tree);
extern hash_t* hash_new();
extern void hash_free(hash_t *tree);
extern void* hash_get(hash_t* tree, const char* addr);
extern void hash_set(hash_t* tree, const char* addr, void* value);

/* Generic "hash_find" function, signature */ 
inline hash_t* hash_find(hash_t *root, uint32_t key, int make);

#endif