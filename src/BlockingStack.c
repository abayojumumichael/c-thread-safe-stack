/*
 * BlockingStack.c
 *
 * Fixed-size generic array-based BlockingStack implementation.
 *
 */

#include "BlockingStack.h"

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "Stack.h"

struct BlockingStack {
	Stack *stack;
	pthread_mutex_t mutex;
	sem_t items;
	sem_t spaces;
};

static bool BlockingStack_lock(BlockingStack *this) {
	if (pthread_mutex_lock(&this->mutex) != 0) {
		perror("BlockingStack: pthread_mutex_lock");
		return false;
	}

	return true;
}

static void BlockingStack_unlock(BlockingStack *this) {
	if (pthread_mutex_unlock(&this->mutex) != 0) {
		perror("BlockingStack: pthread_mutex_unlock");
	}
}

BlockingStack* new_BlockingStack(int max_size) {
	BlockingStack* blockingStack;

	if (max_size <= 0) {
		errno = EINVAL;
		perror("new_BlockingStack: invalid max_size");
		return NULL;
	}

	blockingStack = malloc(sizeof(BlockingStack));
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
		perror("new_BlockingStack: pthread_mutex_init");
		Stack_destroy(blockingStack->stack);
		free(blockingStack);
		return NULL;
	}

	if (sem_init(&blockingStack->items, 0, 0) != 0) {
		perror("new_BlockingStack: sem_init items");
		pthread_mutex_destroy(&blockingStack->mutex);
		Stack_destroy(blockingStack->stack);
		free(blockingStack);
		return NULL;
	}

	if (sem_init(&blockingStack->spaces, 0, (unsigned int) max_size) != 0) {
		perror("new_BlockingStack: sem_init spaces");
		sem_destroy(&blockingStack->items);
		pthread_mutex_destroy(&blockingStack->mutex);
		Stack_destroy(blockingStack->stack);
		free(blockingStack);
		return NULL;
	}

	return blockingStack;
}

bool BlockingStack_push(BlockingStack *this, void *element) {
	bool pushed;

	if (!this) {
		errno = EINVAL;
		perror("BlockingStack_push: NULL stack");
		return false;
	}

	if (!element) {
		errno = EINVAL;
		perror("BlockingStack_push: NULL element");
		return false;
	}

	if (sem_wait(&this->spaces) != 0) {
		perror("BlockingStack_push: sem_wait spaces");
		return false;
	}

	if (!BlockingStack_lock(this)) {
		sem_post(&this->spaces);
		return false;
	}

	pushed = Stack_push(this->stack, element);
	BlockingStack_unlock(this);

	if (!pushed) {
		sem_post(&this->spaces);
		return false;
	}

	if (sem_post(&this->items) != 0) {
		perror("BlockingStack_push: sem_post items");
		return false;
	}

	return true;
}

void* BlockingStack_pop(BlockingStack *this) {
	void* element;

	if (!this) {
		errno = EINVAL;
		perror("BlockingStack_pop: NULL stack");
		return NULL;
	}

	if (sem_wait(&this->items) != 0) {
		perror("BlockingStack_pop: sem_wait items");
		return NULL;
	}

	if (!BlockingStack_lock(this)) {
		sem_post(&this->items);
		return NULL;
	}

	element = Stack_pop(this->stack);
	BlockingStack_unlock(this);

	if (!element) {
		sem_post(&this->items);
		return NULL;
	}

	if (sem_post(&this->spaces) != 0) {
		perror("BlockingStack_pop: sem_post spaces");
		return NULL;
	}

	return element;
}

int BlockingStack_size(BlockingStack *this) {
	int size;

	if (!this) {
		errno = EINVAL;
		perror("BlockingStack_size: NULL stack");
		return 0;
	}

	if (!BlockingStack_lock(this)) {
		return 0;
	}

	size = Stack_size(this->stack);
	BlockingStack_unlock(this);

	return size;
}

bool BlockingStack_isEmpty(BlockingStack *this) {
	bool isEmpty;

	if (!this) {
		errno = EINVAL;
		perror("BlockingStack_isEmpty: NULL stack");
		return true;
	}

	if (!BlockingStack_lock(this)) {
		return true;
	}

	isEmpty = Stack_isEmpty(this->stack);
	BlockingStack_unlock(this);

	return isEmpty;
}

void BlockingStack_clear(BlockingStack *this) {
	int removed_count;

	if (!this) {
		errno = EINVAL;
		perror("BlockingStack_clear: NULL stack");
		return;
	}

	if (!BlockingStack_lock(this)) {
		return;
	}

	removed_count = Stack_size(this->stack);
	Stack_clear(this->stack);
	BlockingStack_unlock(this);

	while (removed_count > 0) {
		if (sem_trywait(&this->items) != 0) {
			break;
		}

		if (sem_post(&this->spaces) != 0) {
			perror("BlockingStack_clear: sem_post spaces");
			break;
		}

		removed_count--;
	}
}

void BlockingStack_destroy(BlockingStack *this) {
	if (!this) {
		errno = EINVAL;
		perror("BlockingStack_destroy: NULL stack");
		return;
	}

	sem_destroy(&this->spaces);
	sem_destroy(&this->items);
	pthread_mutex_destroy(&this->mutex);
	Stack_destroy(this->stack);
	free(this);
}
