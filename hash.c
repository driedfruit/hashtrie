#include "hash.h"
#include "malloc.h"
#include "memory.h"

#define DEBUG_HASH
#ifdef DEBUG_HASH
int hash_mallocs = 0;
int bucket_space = 0;
int pushed_words = 0;
void hash_report() {
	int total = bucket_space * sizeof(hash_t);
	printf("Total: %d bytes (%d kbytes, %d mbytes)\n", total, total/1000,total/1000/1000);
	printf("Called malloc: %d times\n", hash_mallocs);
	printf("Buckets: %d, Words: %d, Load Factor: %0.2f\n", bucket_space, pushed_words, (float)pushed_words/(float)bucket_space);
	printf("\n\n");
}
#endif

/** Hashing functions: (pick one) **/
uint32_t jenkins_one_at_a_time_hash(const char *key, size_t len)
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
hash_t* hash_path_trie(hash_t *root, uint32_t key, int make) {

#if !(TRIE_BIT_SPAN == 1 || TRIE_BIT_SPAN == 2 || TRIE_BIT_SPAN == 4 || TRIE_BIT_SPAN == 8)
	#error "TRIE_BIT_SPAN define must be defined as 1, 2, 4 or 8"
#endif
#define TRIE_BIT_STEPS	(32 / TRIE_BIT_SPAN)
#define TRIE_EDGE_SPAN (2 << (TRIE_BIT_SPAN-1))
#define TRIE_BIT_MASK (TRIE_EDGE_SPAN - 1)

	uint32_t next;
	int i;
	for (i = 0; i < TRIE_BIT_STEPS; i++) {
		next = (key & (TRIE_BIT_MASK << (i * TRIE_BIT_SPAN))) >> (i * TRIE_BIT_SPAN);
		if (make) {
			if (root->key != key && root->key == 0) break;
			if (!root->tab) hash_init(root, TRIE_EDGE_SPAN);
		} else {
			if (root->key == key) break;
			if (!root->tab) return NULL;
		}
		root = root->tab + next;	
	}
	return root;
}

hash_t* hash_path_table(hash_t *root, uint32_t key, int make) {
	hash_t *match = NULL;
	int i;	
	if (!root->tab) {
		if (!make) return NULL;
		hash_init(root, HTABLE_ENTRIES);
	}
	for (i = 0; i < root->span; i++) {
		match = root->tab + i;
		if (match->key == key) break;
		if (make && match->key == 0) break;
		match = NULL;
	}
	/* Grow the table (2x) */
	if (!match && make) {
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

/* Glue: */
inline hash_t* hash_find(hash_t *root, uint32_t key, int make) {
	return HASH_PATH_FUNC(root, key, make);
}

void* hash_get(hash_t* tree, const char* addr) {
	hash_t *node;
	uint32_t key = HASH_HASH_FUNC(addr, strlen(addr));
	node = HASH_PATH_FUNC(tree, key, 0);
	return (node ? node->user : NULL);
}

void hash_set(hash_t* tree, const char* addr, void* value) {
	hash_t *node;
	uint32_t key = HASH_HASH_FUNC(addr, strlen(addr));
	node = HASH_PATH_FUNC(tree, key, 1);
	if (node != NULL) {
		node->key = key;
		node->user = value;
#ifdef DEBUG_HASH
		pushed_words++;
#endif
	}
}

/* Generic API: */
void hash_init(hash_t* tree, int span) {
	/* TODO: calloc? */
	tree->tab = malloc(sizeof(hash_t) * span);
	if (tree->tab) memset(tree->tab, 0, sizeof(hash_t) * span);
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
	tree = malloc(sizeof(hash_t));
	if (tree) {
		tree->tab = NULL;
		tree->span = 0; 
		tree->key = 0; 
		tree->user = NULL;
	}
	return tree;
}

void hash_free(hash_t *tree) {	
	hash_done(tree); 
	free(tree); 
}
