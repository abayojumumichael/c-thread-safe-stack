/*
 * BlockingStack.h
 *
 * Module interface for a generic fixed-size Blocking Stack implementation.
 *
 */

#ifndef BLOCKING_STACK_H_
#define BLOCKING_STACK_H_

#include <stdbool.h>

typedef struct BlockingStack BlockingStack;

/*
 * Creates a new BlockingStack for at most max_size void* elements.
 * Returns a pointer to valid BlockingStack* on success and NULL on failure.
 */
BlockingStack* new_BlockingStack(int max_size);

/*
 * Pushes the given void* element onto this Stack.
 * If the stack is full, the function will block the calling thread until there is space in the stack.
 * Returns false when element is NULL and true on success.
 */
bool BlockingStack_push(BlockingStack* this, void* element);

/*
 * Pops the top element from this Stack.
 * If the stack is empty, the function will block until an element can be popped.
 * Returns the popped void* element.
 */
void* BlockingStack_pop(BlockingStack* this);

/*
 * Returns the number of elements currently on this Stack.
 */
int BlockingStack_size(BlockingStack* this);

/*
 * Returns true is this Stack is empty, false otherwise.
 */
bool BlockingStack_isEmpty(BlockingStack* this);

/*
 * Clears this Stack returning it to an empty state.
 */
void BlockingStack_clear(BlockingStack* this);

/*
 * Destroys this Stack by freeing the memory used by the Stack.
 */
void BlockingStack_destroy(BlockingStack* this);

#endif /* BLOCKING_STACK_H_ */
