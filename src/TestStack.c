/*
 * TestStack.c
 *
 * Unit tests for Stack functionality.
 *
 */

#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "myassert.h"
#include "Stack.h"


#define DEFAULT_MAX_STACK_SIZE 20

/*
 * The stack to use during tests
 */
static Stack *stack;

/*
 * A simple person record used to verify that the
 * stack correctly handles structured data
 */
typedef struct {
	int id;
	char name[50];
} Person;

/*
 * The number of tests that succeeded
 */
static int success_count = 0;

/*
 * The total number of tests run
 */
static int total_count = 0;


/*
* Setup function to run prior to each test
*/
void setup() {
	stack = new_Stack(DEFAULT_MAX_STACK_SIZE);
	total_count++;
}

/*
* Teardown function to run after each test
*/
void teardown() {
	Stack_destroy(stack);
}

/*
* This function is called multiple times from main for each user-defined test function
*/
void runTest(const char *name, int (*testFunction)()) {
	setup();
	printf("RUNNING: %s\n", name);
	
	if (testFunction()) {
		success_count++;
		printf("PASS: %s\n\n", name);
	} else {
		printf("FAIL: %s\n\n", name);
	}

	teardown();
}

/*
 * Checks that the Stack constructor returns a non-NULL pointer.
 */
int newStackIsNotNull() {
	ASSERT_TRUE(stack != NULL);
	return TEST_SUCCESS;
}

/*
 * Checks that the size of an empty stack is 0.
 */
int newStackSizeZero() {
	ASSERT_INT_EQ(0, Stack_size(stack));
	return TEST_SUCCESS;
}

/*
 * Checks that a new stack reports that it is empty.
 */
int newStackIsEmpty() {
	ASSERT_TRUE(Stack_isEmpty(stack));
	return TEST_SUCCESS;
}

/*
 * Checks that creating a stack of size 0 fails.
 */
int newStackInvalidZero() {
	Stack *local_stack = new_Stack(0);
	ASSERT_NULL(local_stack);
	return TEST_SUCCESS;
}

/*
 * Checks that creating a stack with negative size fails.
 */
int newStackInvalidNegative() {
	Stack *local_stack = new_Stack(-10);
	ASSERT_NULL(local_stack);
	return TEST_SUCCESS;
}

/*
 * Checks that pushing one element succeeds.
 */
int pushOneElement() {
	int value = 42;
	ASSERT_TRUE(Stack_push(stack, &value));
	ASSERT_INT_EQ(1, Stack_size(stack));
	ASSERT_FALSE(Stack_isEmpty(stack));
	return TEST_SUCCESS;
}

/*
 * Checks that a pushed element can be popped again.
 */
int pushAndPopOneElement() {
	int value = 7;
	ASSERT_TRUE(Stack_push(stack, &value));
	ASSERT_PTR_EQ(&value, Stack_pop(stack));
	ASSERT_INT_EQ(0, Stack_size(stack));
	ASSERT_TRUE(Stack_isEmpty(stack));
	return TEST_SUCCESS;
}

/*
 * Checks that popping an empty stack returns NULL.
 */
int popEmptyReturnsNull() {
	ASSERT_NULL(Stack_pop(stack));
	ASSERT_INT_EQ(0, Stack_size(stack));
	return TEST_SUCCESS;
}

/*
 * Checks that pushing NULL fails.
 */
int pushNullFails() {
	ASSERT_FALSE(Stack_push(stack, NULL));
	ASSERT_INT_EQ(0, Stack_size(stack));
	return TEST_SUCCESS;
}

/*
 * Checks that pushing beyond capacity fails.
 */
int pushToCapacityThenFail() {
	int values[DEFAULT_MAX_STACK_SIZE + 1];

	for (int i = 0; i < DEFAULT_MAX_STACK_SIZE; i++) {
		values[i] = i;
		ASSERT_TRUE(Stack_push(stack, &values[i]));
	}

	ASSERT_INT_EQ(DEFAULT_MAX_STACK_SIZE, Stack_size(stack));

	values[DEFAULT_MAX_STACK_SIZE] = DEFAULT_MAX_STACK_SIZE;
	ASSERT_FALSE(Stack_push(stack, &values[DEFAULT_MAX_STACK_SIZE]));
	return TEST_SUCCESS;
}

/*
 * Checks that the stack follows LIFO order.
 */
int lifoOrderThreeElements() {
	int a = 10;
	int b = 20;
	int c = 30;

	ASSERT_TRUE(Stack_push(stack, &a));
	ASSERT_TRUE(Stack_push(stack, &b));
	ASSERT_TRUE(Stack_push(stack, &c));

	ASSERT_PTR_EQ(&c, Stack_pop(stack));
	ASSERT_PTR_EQ(&b, Stack_pop(stack));
	ASSERT_PTR_EQ(&a, Stack_pop(stack));
	ASSERT_NULL(Stack_pop(stack));
	return TEST_SUCCESS;
}

/*
 * Checks that size updates correctly after push and pop operations.
 */
int sizeChangesCorrectly() {
	int a = 1;
	int b = 2;

	ASSERT_INT_EQ(0, Stack_size(stack));
	ASSERT_TRUE(Stack_push(stack, &a));
	ASSERT_INT_EQ(1, Stack_size(stack));
	ASSERT_TRUE(Stack_push(stack, &b));
	ASSERT_INT_EQ(2, Stack_size(stack));
	ASSERT_PTR_EQ(&b, Stack_pop(stack));
	ASSERT_INT_EQ(1, Stack_size(stack));
	ASSERT_PTR_EQ(&a, Stack_pop(stack));
	ASSERT_INT_EQ(0, Stack_size(stack));
	return TEST_SUCCESS;
}

/*
 * Checks that empty/non-empty state changes correctly.
 */
int isEmptyChangesCorrectly() {
	int value = 123;

	ASSERT_TRUE(Stack_isEmpty(stack));
	ASSERT_TRUE(Stack_push(stack, &value));
	ASSERT_FALSE(Stack_isEmpty(stack));
	ASSERT_PTR_EQ(&value, Stack_pop(stack));
	ASSERT_TRUE(Stack_isEmpty(stack));
	return TEST_SUCCESS;
}

/*
 * Checks that clear removes all elements from a non-empty stack.
 */
int clearEmptiesStack() {
	int a = 1;
	int b = 2;

	ASSERT_TRUE(Stack_push(stack, &a));
	ASSERT_TRUE(Stack_push(stack, &b));
	Stack_clear(stack);
	ASSERT_TRUE(Stack_isEmpty(stack));
	ASSERT_INT_EQ(0, Stack_size(stack));
	ASSERT_NULL(Stack_pop(stack));
	return TEST_SUCCESS;
}

/*
 * Checks that clearing an already empty stack is safe.
 */
int clearOnEmptyStack() {
	Stack_clear(stack);
	ASSERT_TRUE(Stack_isEmpty(stack));
	ASSERT_INT_EQ(0, Stack_size(stack));
	ASSERT_NULL(Stack_pop(stack));
	return TEST_SUCCESS;
}

/*
 * Checks that the stack can be reused after calling clear.
 */
int reuseAfterClear() {
	int a = 10;
	int b = 20;
	int c = 30;

	ASSERT_TRUE(Stack_push(stack, &a));
	ASSERT_TRUE(Stack_push(stack, &b));
	Stack_clear(stack);
	ASSERT_TRUE(Stack_isEmpty(stack));
	ASSERT_INT_EQ(0, Stack_size(stack));
	ASSERT_TRUE(Stack_push(stack, &c));
	ASSERT_INT_EQ(1, Stack_size(stack));
	ASSERT_PTR_EQ(&c, Stack_pop(stack));
	return TEST_SUCCESS;
}

/*
 * Checks that integer pointers can be stored and retrieved correctly.
 */
int storeIntPointers() {
	int a = 5;
	int b = 7;

	ASSERT_TRUE(Stack_push(stack, &a));
	ASSERT_TRUE(Stack_push(stack, &b));

	int *pb = (int *) Stack_pop(stack);
	int *pa = (int *) Stack_pop(stack);

	ASSERT_NOT_NULL(pb);
	ASSERT_NOT_NULL(pa);
	ASSERT_INT_EQ(7, *pb);
	ASSERT_INT_EQ(5, *pa);
	return TEST_SUCCESS;
}

/*
 * Checks that string pointers can be stored and retrieved correctly.
 */
int storeCharPointerStrings() {
	char *str1 = "hello";
	char *str2 = "world";

	ASSERT_TRUE(Stack_push(stack, str1));
	ASSERT_TRUE(Stack_push(stack, str2));

	char *p2 = (char *) Stack_pop(stack);
	char *p1 = (char *) Stack_pop(stack);

	ASSERT_NOT_NULL(p2);
	ASSERT_NOT_NULL(p1);
	ASSERT_TRUE(strcmp(p2, "world") == 0);
	ASSERT_TRUE(strcmp(p1, "hello") == 0);
	return TEST_SUCCESS;
}

/*
 * Checks that struct pointers can be stored and retrieved correctly.
 */
int storeStructPointers() {
	Person p1 = {1, "Alice"};
	Person p2 = {2, "Bob"};

	ASSERT_TRUE(Stack_push(stack, &p1));
	ASSERT_TRUE(Stack_push(stack, &p2));

	Person *out2 = (Person *) Stack_pop(stack);
	Person *out1 = (Person *) Stack_pop(stack);

	ASSERT_NOT_NULL(out2);
	ASSERT_NOT_NULL(out1);
	ASSERT_INT_EQ(2, out2->id);
	ASSERT_TRUE(strcmp(out2->name, "Bob") == 0);
	ASSERT_INT_EQ(1, out1->id);
	ASSERT_TRUE(strcmp(out1->name, "Alice") == 0);
	return TEST_SUCCESS;
}

/*
 * Checks many push and pop operations within the stack capacity.
 */
int pushPopManyWithinCapacity() {
	int values[DEFAULT_MAX_STACK_SIZE];

	for (int i = 0; i < DEFAULT_MAX_STACK_SIZE; i++) {
		values[i] = i * 10;
		ASSERT_TRUE(Stack_push(stack, &values[i]));
		ASSERT_INT_EQ(i + 1, Stack_size(stack));
	}

	ASSERT_FALSE(Stack_push(stack, &values[0]));

	for (int i = DEFAULT_MAX_STACK_SIZE - 1; i >= 0; i--) {
		int *value = (int *) Stack_pop(stack);
		ASSERT_NOT_NULL(value);
		ASSERT_INT_EQ(i * 10, *value);
		ASSERT_INT_EQ(i, Stack_size(stack));
	}

	ASSERT_TRUE(Stack_isEmpty(stack));
	ASSERT_NULL(Stack_pop(stack));
	return TEST_SUCCESS;
}

/*
 * Checks alternating push and pop operations.
 */
int alternatingPushPop() {
	int a = 1;
	int b = 2;
	int c = 3;

	ASSERT_TRUE(Stack_push(stack, &a));
	ASSERT_PTR_EQ(&a, Stack_pop(stack));
	ASSERT_TRUE(Stack_push(stack, &b));
	ASSERT_TRUE(Stack_push(stack, &c));
	ASSERT_PTR_EQ(&c, Stack_pop(stack));
	ASSERT_PTR_EQ(&b, Stack_pop(stack));
	ASSERT_TRUE(Stack_isEmpty(stack));
	ASSERT_INT_EQ(0, Stack_size(stack));
	return TEST_SUCCESS;
}

/*
 * Checks that two separate stacks operate independently.
 */
int multipleStacksIndependent() {
	Stack *first = new_Stack(2);
	Stack *second = new_Stack(3);
	int a = 1;
	int b = 2;
	int c = 3;

	ASSERT_NOT_NULL(first);
	ASSERT_NOT_NULL(second);
	ASSERT_TRUE(Stack_push(first, &a));
	ASSERT_TRUE(Stack_push(second, &b));
	ASSERT_TRUE(Stack_push(second, &c));
	ASSERT_INT_EQ(1, Stack_size(first));
	ASSERT_INT_EQ(2, Stack_size(second));
	ASSERT_PTR_EQ(&a, Stack_pop(first));
	ASSERT_PTR_EQ(&c, Stack_pop(second));
	ASSERT_PTR_EQ(&b, Stack_pop(second));
	ASSERT_TRUE(Stack_isEmpty(first));
	ASSERT_TRUE(Stack_isEmpty(second));

	Stack_destroy(first);
	Stack_destroy(second);
	return TEST_SUCCESS;
}

/*
 * Checks that a used stack can be destroyed without error.
 */
int destroyAfterNormalUsage() {
	Stack *local_stack = new_Stack(3);
	int a = 1;
	int b = 2;

	ASSERT_NOT_NULL(local_stack);
	ASSERT_TRUE(Stack_push(local_stack, &a));
	ASSERT_TRUE(Stack_push(local_stack, &b));
	ASSERT_PTR_EQ(&b, Stack_pop(local_stack));
	Stack_destroy(local_stack);
	return TEST_SUCCESS;
}

/*
 * Main function for the Stack tests which will run each user-defined test in turn.
 */
int main() {
	runTest("newStackIsNotNull", newStackIsNotNull);
	runTest("newStackSizeZero", newStackSizeZero);
	runTest("newStackIsEmpty", newStackIsEmpty);
	runTest("newStackInvalidZero", newStackInvalidZero);
	runTest("newStackInvalidNegative", newStackInvalidNegative);
	runTest("pushOneElement", pushOneElement);
	runTest("pushAndPopOneElement", pushAndPopOneElement);
	runTest("popEmptyReturnsNull", popEmptyReturnsNull);
	runTest("pushNullFails", pushNullFails);
	runTest("pushToCapacityThenFail", pushToCapacityThenFail);
	runTest("lifoOrderThreeElements", lifoOrderThreeElements);
	runTest("sizeChangesCorrectly", sizeChangesCorrectly);
	runTest("isEmptyChangesCorrectly", isEmptyChangesCorrectly);
	runTest("clearEmptiesStack", clearEmptiesStack);
	runTest("clearOnEmptyStack", clearOnEmptyStack);
	runTest("reuseAfterClear", reuseAfterClear);
	runTest("storeIntPointers", storeIntPointers);
	runTest("storeCharPointerStrings", storeCharPointerStrings);
	runTest("storeStructPointers", storeStructPointers);
	runTest("pushPopManyWithinCapacity", pushPopManyWithinCapacity);
	runTest("alternatingPushPop", alternatingPushPop);
	runTest("multipleStacksIndependent", multipleStacksIndependent);
	runTest("destroyAfterNormalUsage", destroyAfterNormalUsage);

	printf("Stack Tests complete: %d / %d tests successful.\n----------------\n", success_count, total_count);
	return 0;
}
