/* Stash. A hash table. Public Domain.
 **/ 
#include <malloc.h>
#include <memory.h>

#include "stash.h"
#include "hash.h"

#ifdef DEBUG_HASH
int stash_mallocs = 0;
int bucket_travs = 0;
int bucket_waste_travs = 0;
void stash_report() {
	printf("Called malloc: %d times\n", stash_mallocs);
	printf("Wasted on bucket traversal: %d\n", bucket_travs);
	printf("Wasted on last node traversal: %d\n", bucket_waste_travs);
	printf("\n\n");
}
#endif

static stash_entry* stash_entry_new(stash_t *st, const char *addr, int addr_len, int size) {
	stash_entry *en;
	int i;
	/* Init table when needed */
	if (st->table == NULL) {
		st->table = malloc(sizeof(stash_entry) * STASH_MIN_SPAN);
		memset(st->table, 0, sizeof(stash_entry) * STASH_MIN_SPAN);
		if (st->table == NULL) return NULL;
		st->span = STASH_MIN_SPAN;
	}
	/* Grow table 2x times when needed */	
	if (st->fill >= st->span) {
		stash_entry *larger_table = (stash_entry *)realloc(st->table, sizeof(stash_entry) * (st->span * 2));
		if (larger_table == NULL) return NULL;
		for (i = 0; i < st->fill; i++) { /* Rebase all pointers :( */
			stash_entry *tmp = larger_table + i;
			if (tmp->next != NULL)
				tmp->next = larger_table + (tmp->next - st->table);	
#ifdef DEBUG_HASH
	stash_mallocs++;
#endif
		}
		st->table = larger_table;
		st->span *= 2;
	}
	/* Alloc new entry */
	en = st->table + st->fill;
	en->data = malloc(sizeof(byte) * (size + addr_len));
	if (en->data == NULL) return NULL;
	/* Save offsets */
	en->key_size = addr_len;
	en->val_size = size;
	en->next = NULL;
	/* Save key */	
	memcpy(en->data, addr, addr_len);
	/* Count */
	st->fill++;

	return en;
}

/* Generic API: */
byte* stash_get(stash_t *st, const char *addr, int addr_len, int *size) {
	stash_entry *en;	
	hkey key = HASH_HASH_FUNC(addr, addr_len);
	hash_t *node = hash_find(st->tab, key, 0);
	if (node == NULL) return NULL; /* No pointer found */
	if (node->user == NULL) return NULL; /* No buckets here */
	en = (stash_entry *)st->table + ((int)node->user-1);	/* Traverse buckets */
	do {
		if (en->key_size == addr_len 
		&& !memcmp(en->data, addr, addr_len))
			break;
		en = en->next;
#ifdef DEBUG_HASH
	bucket_travs++;
#endif
	} while (en);
	if (en == NULL) return NULL;	/* No matching entry */ 
	/* Success */	
	*size = en->val_size;
	return (byte*)&en->data[en->key_size];
} 

byte* stash_peek(stash_t *st, const char *addr, int addr_len) {
	int size;
	return stash_get(st, addr, addr_len, &size);
} 

void stash_put(stash_t* st, const char *addr, int addr_len, byte *var, int size) {
	stash_entry *en;
	hkey key = HASH_HASH_FUNC(addr, addr_len);
	hash_t *node = hash_find(st->tab, key, 1);
	if (node == NULL) return; // Fatal error
	if (node->user != NULL) { /* Node has buckets */
		stash_entry *last;
		en = (stash_entry *)st->table + ((int)node->user-1); /* Traverse buckets */
		do {
			last = en;
			if (en->key_size == addr_len 
			&& !memcmp(en->data, addr, addr_len))
				break;
			en = en->next;
#ifdef DEBUG_HASH
	bucket_travs++;
#endif
		} while (en);
		if (en == NULL) { /* Key collision, create new entry */
			//int rebase_offset = last - st->table;
			en = stash_entry_new(st, addr, addr_len, size);
			if (en == NULL) return;
			//last = (stash_entry *)st->table + (rebase_offset); /* Find list tail */
			while (last->next) {
				last = last->next;
#ifdef DEBUG_HASH				
	bucket_waste_travs++;
#endif	
			}
			last->next = en;
		} else if (size != en->val_size) {	/* Re-alloc: */
			free(en->data); 
			en->data = malloc(sizeof(byte) * (size + addr_len));
			if (en->data == NULL) return; // Fatal error
			en->key_size = addr_len;
			en->val_size = size;
			memcpy(en->data, addr, addr_len);
		} 
	} else {	/* Node is new, create new entry for it */
		en = stash_entry_new(st, addr, addr_len, size);
		if (en == NULL) return;
		node->user = (void*)((int)(en - st->table) + 1); /* Make it a head of linked list */
		node->key = key;
	}
	/* Save new value */
	memcpy(&en->data[addr_len], var, size);
}

stash_t* stash_new() {
	stash_t* node;
	if ((node = malloc(sizeof(stash_t)))) {
		node->tab = hash_new();
		node->table = NULL;
		node->span = 0;
		node->fill = 0;
	}
	return node;
}

void stash_free(stash_t* node) {
	int i;
	hash_free(node->tab);
	for (i = 0; i < node->fill; i++) {
		stash_entry *en = node->table + i;
		free(en->data);
	}
	free(node->table);
	free(node);
}

/* Dangerous global variables that aid H_XXX helper macros (see stash.h) */
int   stash_unsafe_int;
byte* stash_unsafe_ptr;
int*  stash_static_int(int Num) { static int XNum; XNum = Num; return &XNum; }
