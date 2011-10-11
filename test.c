#include "stdio.h"

#include "hash.h"

int main(int argc, const char* argv[]) {

	hash_t *root = hash_new();

	uint32_t key2_x = 0xCAFEb012;

	hash_t *m = hash_find(root, key2_x, 1);
	printf("<%p>----------------------\n", m );
	hash_t *n = hash_find(root, key2_x, 0);
	printf("<%p>----------------------\n", n );

	hash_free(root);

	return 0;
}
