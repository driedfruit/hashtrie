/* Hashtrie. Public domain.
 * Jenkins one-at-a-time used as hashing function. 
 **/
#include "hash.h"
#include <malloc.h>
#include <memory.h>

#ifdef DEBUG_HASH
int hash_mallocs = 0;
int bucket_space = 0;
int pushed_words = 0;
void dump_hash(hash_t *root, int depth) {

	int i;
	for (i = 0; i < depth; i++) printf("\t");
	
	printf("[%d] { %08x } == %p\n", root->span, root->key, root->user);
	for (i = 0; i < root->span; i++) {
		dump_hash(root->tab + i, depth+1);
	}

}
void hash_report() {
	int total = bucket_space * sizeof(hash_t);
	printf("Total: %d bytes (%d kbytes, %d mbytes)\n", total, total/1000,total/1000/1000);
	printf("Called malloc: %d times\n", hash_mallocs);
	printf("Buckets: %d, Words: %d, Load Factor: %0.2f\n", bucket_space, pushed_words, (float)pushed_words/(float)bucket_space);
	printf("\n\n");
}
#endif

/** Hashing functions: (pick one) **/
uint32_t jenkins_oneatatime(const char *key, size_t len)
{
    uint32_t hash, i;
    for(hash = i = 0; i < len; ++i)
    {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}


/** Path functions: (pick one pair) **/
#if !(HTRIE_BIT_SPAN == 1 || HTRIE_BIT_SPAN == 2 || HTRIE_BIT_SPAN == 4 || HTRIE_BIT_SPAN == 8)
	#error "HTRIE_BIT_SPAN define must be defined as 1, 2, 4 or 8"
#endif
#define TRIE_BIT_SPAN HTRIE_BIT_SPAN
#define TRIE_BIT_STEPS	(32 / TRIE_BIT_SPAN)
#define TRIE_EDGE_SPAN (2 << (TRIE_BIT_SPAN-1))
#define TRIE_BIT_MASK (TRIE_EDGE_SPAN - 1)

static hash_t* hash_path_trie_get(hash_t *root, uint32_t key) {
	uint32_t next;
	int i;
	for (i = 0; i < TRIE_BIT_STEPS; i++) {
		next = (key & (TRIE_BIT_MASK << (i * TRIE_BIT_SPAN))) >> (i * TRIE_BIT_SPAN);
		if (root->key == key) break;
		if (!root->tab) return NULL;
		root = root->tab + next;
	}
	return root;
}
static hash_t* hash_path_trie_set(hash_t *root, uint32_t key) {
	uint32_t next;
	int i;
	for (i = 0; i < TRIE_BIT_STEPS; i++) {
		next = (key & (TRIE_BIT_MASK << (i * TRIE_BIT_SPAN))) >> (i * TRIE_BIT_SPAN);
		if (root->key == key) break;
		else if (root->user == NULL) break; /* node is unused */
		if (!root->tab) hash_init(root, TRIE_EDGE_SPAN);
		root = root->tab + next;
	}
	return root;
}

static hash_t* hash_path_list_get(hash_t *root, uint32_t key) {
	hash_t *match = NULL;
	int i;
	if (root->tab)
	for (i = 0; i < root->span; i++) {
		match = root->tab + i;
		if (match->key == key) break;
		match = NULL;
	}
	return match;
}
hash_t* hash_path_list_set(hash_t *root, uint32_t key) {
	hash_t *match = NULL;
	int i;

	if (!root->tab) hash_init(root, HTABLE_ENTRIES);
	for (i = 0; i < root->span; i++) {
		match = root->tab + i;
		if (!match->key || match->key == key) break;
		match = NULL;
	}
	/* Grow the table (2x) */
	if (!match) {
		hash_t *larger_table = (hash_t *)realloc(root->tab, sizeof(hash_t) * root->span * 2);
		if (larger_table == NULL) return NULL;//Fatal error
		root->tab = larger_table;
		root->span *= 2;
		match = root->tab + i;
#ifdef DEBUG_HASH
		hash_mallocs++;
		bucket_space *= 2;
#endif
	}
	return match;
}

/* Get/Set */
void* hash_kget(hash_t* tree, uint32_t key) {
	hash_t *node;
	node = HASH_FIND_FUNC(tree, key);
	return (node ? node->user : NULL);
}

void hash_kset(hash_t* tree, uint32_t key, void* value) {
	hash_t *node;
	node = HASH_PATH_FUNC(tree, key);
	if (node != NULL) {
		node->key = key;
		node->user = value;
#ifdef DEBUG_HASH
		pushed_words++;
#endif
	}
}

/* Glue: */
#ifndef __cplusplus
inline
#endif
 hash_t* hash_find(hash_t *root, uint32_t key, int make) {
	return (!make ? HASH_FIND_FUNC(root, key) : HASH_PATH_FUNC(root, key) );
}

#ifndef __cplusplus
inline
#endif
 void* hash_get(hash_t* tree, const char* addr) {
	uint32_t key = HASH_HASH_FUNC(addr, strlen(addr));
	return hash_kget(tree, key);
}

#ifndef __cplusplus
inline
#endif
 void hash_set(hash_t* tree, const char* addr, void* value) {
	uint32_t key = HASH_HASH_FUNC(addr, strlen(addr));
	hash_kset(tree, key, value);
}

//#define CALLOC_INSTEAD_OF_MALLOC

/* Generic API: */
void hash_init(hash_t* tree, int span) {
#ifdef CALLOC_INSTEAD_OF_MALLOC
	tree->tab = (hash_t*) calloc(span, sizeof(hash_t));
#else
	tree->tab = (hash_t*) malloc(sizeof(hash_t) * span);
	if (tree->tab != NULL) memset(tree->tab, 0, sizeof(hash_t) * span);
#endif
	tree->span = span;
#ifdef DEBUG_HASH
	hash_mallocs++;
	bucket_space += span;
#endif
}

void hash_done(hash_t *tree) {
	int i;
	if (tree->tab) 
		for (i = 0; i < tree->span; i++)
			hash_done((hash_t*)(tree->tab + i));
	free(tree->tab);
}

hash_t* hash_new() {
	hash_t *tree;
#ifdef CALLOC_INSTEAD_OF_MALLOC
	tree = (hash_t*) calloc(1, sizeof(hash_t));
#else
	tree = (hash_t*) malloc(sizeof(hash_t));
	if (tree) {
		tree->tab = NULL;
		tree->span = 0; 
		tree->key = 0; 
		tree->user = NULL;
	}
#endif	
	return tree;
}

void hash_free(hash_t *tree) {	
	hash_done(tree); 
	free(tree); 
}
