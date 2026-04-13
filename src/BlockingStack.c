/*
 * BlockingStack.c
 *
 * Fixed-size generic array-based BlockingStack implementation.
 *
 */

#include "BlockingStack.h"

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>


BlockingStack *new_BlockingStack(int max_size) {
	if (max_size <= 0) {
		errno = EINVAL;
		perror("new_BlockingStack: invalid max_size");
		return NULL;
	}

	BlockingStack* blockingStack = malloc(sizeof(BlockingStack));
	if (!blockingStack) {
		perror("new_BlockingStack: malloc blocking stack");
		return NULL;		
	}

	blockingStack->stack = new_Stack(max_size);
	if (!blockingStack->stack) {
		free(blockingStack);
		return NULL;
	}

	if (pthread_mutex_init(&blockingStack->mutex, NULL) != 0) {
		Stack_destroy(blockingStack->stack);
		free(blockingStack);
		return NULL;
	}

	if (pthread_cond_init(&blockingStack->not_empty, NULL) != 0) {
		pthread_mutex_destroy(&blockingStack->mutex);
		Stack_destroy(blockingStack->stack);
		free(blockingStack);
		return NULL;
	}

	if (pthread_cond_init(&blockingStack->not_full, NULL) != 0) {
		pthread_cond_destroy(&blockingStack->not_empty);
		pthread_mutex_destroy(&blockingStack->mutex);
		Stack_destroy(blockingStack->stack);
		free(blockingStack);
		return NULL;
	}

	return blockingStack;
}


bool BlockingStack_push(BlockingStack* this, void* element) {
	if (this == NULL) {
		errno = EINVAL;
		perror("BlockingStack_push: NULL stack");
		return false;
	}

	if (element == NULL) {
		errno = EINVAL;
		perror("BlockingStack_push: NULL element");
		return false;
	}

	pthread_mutex_lock(&this->mutex);

	while (Stack_size(this->stack) >= this->stack->max_size) {
		pthread_cond_wait(&this->not_full, &this->mutex);
	}

	bool pushed = Stack_push(this->stack, element);
	if (pushed) {
		pthread_cond_signal(&this->not_empty);
	}

	pthread_mutex_unlock(&this->mutex);
	return pushed;
}


void* BlockingStack_pop(BlockingStack* this) {
	if (this == NULL) {
		errno = EINVAL;
		perror("BlockingStack_pop: NULL stack");
		return NULL;
	}

	pthread_mutex_lock(&this->mutex);

	while (Stack_size(this->stack) == 0) {
		pthread_cond_wait(&this->not_empty, &this->mutex);
	}

	void* element = Stack_pop(this->stack);
	pthread_cond_signal(&this->not_full);
	
	
	pthread_mutex_unlock(&this->mutex);
	return element;
}


int BlockingStack_size(BlockingStack* this) {
	if (this == NULL) {
		errno = EINVAL;
		perror("BlockingStack_size: NULL stack");
		return 0;
	}

	pthread_mutex_lock(&this->mutex);
	int size = Stack_size(this->stack);
	pthread_mutex_unlock(&this->mutex);

	return size;
}


bool BlockingStack_isEmpty(BlockingStack* this) {
	if (this == NULL) {
		errno = EINVAL;
		perror("BlockingStack_isEmpty: NULL stack");
		return true;
	}

	pthread_mutex_lock(&this->mutex);
	bool isEmpty = Stack_isEmpty(this->stack);
	pthread_mutex_unlock(&this->mutex);

	return isEmpty;
}


void BlockingStack_clear(BlockingStack* this) {
	if (this == NULL) {
		errno = EINVAL;
		perror("BlockingStack_clear: NULL stack");
		return;
	}

	pthread_mutex_lock(&this->mutex);
	Stack_clear(this->stack);
	pthread_cond_broadcast(&this->not_full);
	pthread_mutex_unlock(&this->mutex);
}


void BlockingStack_destroy(BlockingStack* this) {
	if (this == NULL) {
		errno = EINVAL;
		perror("BlockingStack_destroy: NULL stack");
		return;
	}
	
	Stack_destroy(this->stack);
	pthread_cond_destroy(&this->not_full);
	pthread_cond_destroy(&this->not_empty);
	pthread_mutex_destroy(&this->mutex);
	free(this);	
}
