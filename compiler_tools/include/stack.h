#include <stdlib.h>

#ifndef VALUE_T
	#define VALUE_T
	#if __WORDSIZE == 64
		typedef	long long value_t;
	#else
		typedef int value_t;
	#endif
#endif

struct stack_t {
	value_t value;
	struct stack_t* prev;
};

void push(struct stack_t** top, value_t value) {
	struct stack_t* newtop = malloc(sizeof(struct stack_t));
	newtop->value = value;
	newtop->prev = *top;
	*top = newtop;
}

value_t pop(struct stack_t** top) {
	value_t value;
	struct stack_t* prev;

	if (!(*top)) {
		return 0;
	}

	value = (*top)->value;
	prev = (*top)->prev;
	free(*top);
	*top = prev;

	return value;
		
}

value_t peek(struct stack_t* top) {
	if (!top) return 0;

	return top->value;
}

void recursive_push(struct stack_t** dest, struct stack_t* src) {
	if (src->prev) recursive_push(dest, src->prev);
	push(dest, src->value);
}

void copy_stack(struct stack_t** dest, struct stack_t* src) {
	while (pop(dest)); // clear it
	if (src) recursive_push(dest, src); // copy it
}

void clear_stack(struct stack_t** top) {
	while (pop(top));
	*top = NULL;
} 
