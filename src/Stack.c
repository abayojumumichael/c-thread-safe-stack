/*
 * Stack.c
 *
 * Fixed-size generic array-based Stack implementation.
 *
 */

#include "Stack.h"

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

struct Stack {
    void** elements;    // dynamic array of void* items
    int max_size;       // capacity
    int current_size;   // number of items currently in the stack
};

Stack* new_Stack(int max_size) {
	if (max_size <= 0) {
		errno = EINVAL;
		perror("new_Stack: invalid max_size");
		return NULL;
	}

	Stack* stack = malloc(sizeof(Stack));
	if (!stack) {
		perror("new_Stack: malloc stack");
		return NULL;
	}

	stack->elements = malloc(sizeof(void*) * max_size);
	if (!stack->elements) {
		perror("new_Stack: malloc elements");
		free(stack);
		return NULL;
	}
	stack->max_size = max_size;
	stack->current_size = 0;

	return stack;
}


bool Stack_push(Stack* this, void* element) {
	if (!this) {
		errno = EINVAL;
		perror("Stack_push: NULL stack");
		return false;
	}

	if (!element) {
		errno = EINVAL;
		perror("Stack_push: NULL element");
		return false;
	}

	if (this->current_size >= this->max_size) {
		errno = ENOMEM;
		perror("Stack_push: stack full");
		return false;
	}

	this->elements[this->current_size++] = element;
	return true;
}


void* Stack_pop(Stack* this) {
	if (!this) {
		errno = EINVAL;
		perror("Stack_pop: NULL stack");
		return NULL;
	}
	if (Stack_isEmpty(this)) {
		errno = EINVAL;
		perror("Stack_pop: stack empty");
		return NULL;
	}

	return this->elements[--this->current_size];
}


int Stack_size(Stack* this) {
	if (!this) {
		errno = EINVAL;
		perror("Stack_size: NULL stack");
		return 0;
	}

	return this->current_size;
}


bool Stack_isEmpty(Stack* this) {
	if (!this) {
		errno = EINVAL;
		perror("Stack_isEmpty: NULL stack");
		return true;
	}

	return this->current_size == 0;
}


void Stack_clear(Stack* this) {
	if (!this) {
		errno = EINVAL;
		perror("Stack_clear: NULL stack");
		return;
	}

	this->current_size = 0;
}


void Stack_destroy(Stack* this) {
	if (!this) {
		errno = EINVAL;
		perror("Stack_destroy: NULL stack");
		return;
	}

	free(this->elements);
	free(this);
}
