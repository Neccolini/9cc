#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

typedef enum {
	TK_RESERVED, //symbol
	TK_NUM, //integer
	TK_EOF, //End of File
}TokenKind;
typedef struct Token Token;

struct Token {
	TokenKind kind; //Token Type
	Token *next; //next Token
	int val; // the number when kind is TK_NUM
	char *str; // Token String
};

// Current Token
Token *token;

//error func.
void error(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}
// If the next token is the expected symbol, 
// go ahead of the token and return true.
bool consume(char op) {
	if(token->kind != TK_RESERVED || token->str[0] != op)
		return false;
	token =token->next;
	return true;
}

// If the next token is the expected symbol, 
// go ahead of the token and otherwise report the error.
void expect(char op) {
	if(token->kind != TK_RESERVED || token->str[0] != op)
	       error("not '%c'",op);
	token = token->next;
}

int expect_number() {
	if(token->kind != TK_NUM)
		error("not a number");
	int val = token->val;
	token = token->next;
	return val;
}

bool at_eof() {
	return token->kind == TK_EOF;
}

// make a new token and connect it to cur.
Token *new_token(TokenKind kind, Token *cur, char *str) {
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str=str;
	cur->next=tok;
	return tok;
}

// tokenize p and return the result
Token *tokenize(char *p) {
	Token head;
	head.next = NULL;
	Token *cur = &head;
	
	while(*p) {
		// skip the space
		if (isspace(*p)) {
			p++;
			continue;
		}
		if(*p == '+' || *p == '-') {
			cur =new_token(TK_RESERVED, cur, p++);
			continue;
		}
		if(isdigit(*p)) {
			cur = new_token(TK_NUM, cur, p);
			cur->val = strtol(p, &p ,10);
			continue;
		}
		error("cannot tokenize the input");
	}
	new_token(TK_EOF, cur, p);
	return head.next;
}



int main(int argc, char **argv) {
	if(argc!=2){
          	fprintf(stderr, "The number of arguments is incorrect.\n");
		return 1;
	}
	
	token = tokenize(argv[1]);

	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");

	printf("\tmov rax, %d\n", expect_number());

	while(!at_eof()) {
		if(consume('+')) {
			printf("\tadd rax, %d\n",expect_number());
			continue;
		}
		
		expect('-');
		printf("\tsub rax, %d\n", expect_number());
	}
	printf("\tret\n");
	return 0;
}
