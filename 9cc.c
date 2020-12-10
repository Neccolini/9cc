#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv) {
	if(argc!=2){
          	fprintf(stderr, "The number of arguments is incorrect.\n");
		return 1;
	}
	char *p = argv[1];

	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");
	printf("\tmov rax, %ld\n", strtol(p, &p, 10));
	while(*p) {
		if(*p == '+') {
			p++;
			printf("\tadd rax, %ld\n", strtol(p, &p, 10));
			continue;
		}
		if(*p == '-') {
			p++;
			printf("\tsub rax, %ld\n", strtol(p, &p, 10));
			continue;
		}
		fprintf(stderr, "unexpected character: '%c'\n", *p);
	}
	printf("\tret\n");
	return 0;
}
