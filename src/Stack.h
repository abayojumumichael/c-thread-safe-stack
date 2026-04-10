/*
 * Stack.h
 *
 * Module interface for a generic fixed-size Stack implementation.
 *
 */

#ifndef STACK_H_
#define STACK_H_

#include <stdbool.h>

typedef struct Stack Stack;

struct Stack {
    void** elements;    // dynamic array of void* items
    int max_size;       // capacity
    int current_size;   // number of items currently in the stack
};



/*
 * Creates a new Stack for at most max_size void* elements.
 * Returns a pointer to valid Stack* on success and NULL on failure.
 */
Stack* new_Stack(int max_size);

/*
 * Pushes the given void* element onto this Stack.
 * Returns true on success and false on push failure when element is NULL or stack is full.
 */
bool Stack_push(Stack* this, void* element);

/*
 * Pops the top element from this Stack.
 * Returns popped void* element on success or NULL if stack is empty.
 */
void* Stack_pop(Stack* this);

/*
 * Returns the number of elements currently on this Stack.
 */
int Stack_size(Stack* this);

/*
 * Returns true is this Stack is empty, false otherwise.
 */
bool Stack_isEmpty(Stack* this);

/*
 * Clears this Stack returning it to an empty state.
 */
void Stack_clear(Stack* this);

/*
 * Destroys this Stack by freeing the memory used by the Stack.
 */
void Stack_destroy(Stack* this);

#endif /* STACK_H_ */
