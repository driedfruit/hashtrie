#include "stdio.h"

#include "hash.h"

int main(int argc, const char* argv[]) {

	const char filename[] = "5desk.txt";

	const char *file_ptr = filename;

	if (argc > 1) file_ptr = argv[1];

	FILE *f = fopen(file_ptr, "r");
	
	if (!f) {
		printf("Can't open %s\nUsage:\n pushword [FILE] [WORD] \t FILE is 5desk.txt by default\n", file_ptr);
		return 1;
	}

	hash_t *root = hash_new();

	char buf[1024];
	while (fgets ( buf, 1024, f )) {
		buf[strlen(buf)-1] = '\0';

		char *word = malloc(sizeof(char) * 80);
		memcpy(word, buf, 80);
		word[strlen(buf)] = '\0';

		//hash_set(root, buf, 0);		
		hash_set(root, buf, word);
	}

	if (argc > 2) {
		char *x = hash_get(root, argv[2]);
		printf("Word: %s=%s\n", argv[2], x);
	}

	fclose(f);

	hash_free(root);
	hash_report();
	return 0;
}