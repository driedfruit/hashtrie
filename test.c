#include "stdio.h"

#include "hash.h"
void dump_hash(hash_t *root, int depth);

int main(int argc, const char* argv[]) {

	hash_t *root = hash_new();

	uint32_t key1_x = 0xCAFEb011;
	uint32_t key2_x = 0xCAFEb012;
	uint32_t key3_x = 0xCABEb013;

	hash_kset(root, key1_x, 1);
	hash_kset(root, key2_x, 2);

	printf(">> %08x == %d\n", key1_x, hash_kget(root, key1_x));
	printf(">> %08x == %d\n", key2_x, hash_kget(root, key2_x));
	printf(">> %08x == %d\n", key3_x, hash_kget(root, key3_x));

	dump_hash(root, 0);

	hash_kset(root, key1_x, 0);
	hash_kset(root, key3_x, 3);

	printf(">> %08x == %d\n", key1_x, hash_kget(root, key1_x));

	dump_hash(root, 0);

	hash_free(root);

	return 0;
}
