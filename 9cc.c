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

typedef enum {
	ND_ADD, // +
	ND_SUB, // -
	ND_MUL, // *
	ND_DIV, // /
	ND_NUM, // integer
} NodeKind;

typedef struct Token Token;
struct Token {
	TokenKind kind; //Token Type
	Token *next; //next Token
	int val; // the number when kind is TK_NUM
	char *str; // Token String
};

typedef struct Node Node;
struct Node {
	NodeKind kind;
	Node *lhs;
	Node *rhs;
	int val;
};

char *user_input;

// Current Token
Token *token;

//error function
void error(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

void error_at(char *loc, char *fmt, ...) {
	va_list ap;
	va_start(ap,fmt);
	int pos = loc - user_input;
	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", pos, " ");
	fprintf(stderr, "^ ");
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
	       error_at(token->str, "expected '%c'", op);
	token = token->next;
}

int expect_number() {
	if(token->kind != TK_NUM)
		error_at(token->str, "expected a number");
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

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node *new_node_num(int val) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_NUM;
	node->val = val;
	return node;
}

Node *expr();
Node *mul();
Node *primary();

Node *primary() {
	if(consume('(')) {
		Node *node = expr();
		expect(')');
		return node;
	}
	return new_node_num(expect_number());
}
Node *mul() {
	Node *node = primary();

	for(;;) {
		if(consume('*'))
			node = new_node(ND_MUL, node ,primary());
		else if(consume('/'))
			node = new_node(ND_DIV, node, primary());
		else return node;
	}
}
Node *expr() {
	Node *node = mul();

	for(;;) {
		if(consume('+'))
			node = new_node(ND_ADD, node, mul());
		else if(consume('-'))
			node = new_node(ND_SUB, node, mul());
		else return node;
	}
}

// tokenize p and return the result
Token *tokenize() {
	char *p = user_input;
	Token head;
	head.next = NULL;
	Token *cur = &head;
	
	while(*p) {
		// skip the space
		if (isspace(*p)) {
			p++;
			continue;
		}
		if(strchr("+-*/()",*p)) {
			cur =new_token(TK_RESERVED, cur, p++);
			continue;
		}
		if(isdigit(*p)) {
			cur = new_token(TK_NUM, cur, p);
			cur->val = strtol(p, &p ,10);
			continue;
		}
		error_at(p, "expected a number");
	}
	new_token(TK_EOF, cur, p);
	return head.next;
}

void gen(Node *node) {
	if(node->kind == ND_NUM) {
		printf("\tpush %d\n", node->val);
		return;
	}
	gen(node->lhs);
	gen(node->rhs);

	printf("\tpop rdi\n");
	printf("\tpop rax\n");

	switch(node->kind) {
		case ND_ADD:
			printf("\tadd rax, rdi\n");
			break;
		case ND_SUB:
			printf("\tsub rax, rdi\n");
			break;
		case ND_MUL:
			printf("\timul rax, rdi\n");
			break;
		case ND_DIV:
			printf("\tcqo\n");
			printf("\tidiv rdi\n");
			break;
	}
	printf("\tpush rax\n");
}


int main(int argc, char **argv) {
	if(argc!=2)
        fprintf(stderr, "The number of arguments is incorrect.\n");
	
	user_input = argv[1];
	token = tokenize();
	Node *node = expr();
	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");

	gen(node);

	printf("\tpop rax\n");
	printf("\tret\n");
	return 0;
}
