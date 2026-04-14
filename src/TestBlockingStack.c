/*
 * TestBlockingStack.c
 *
 * Unit tests for BlockingStack functionality.
 *
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "BlockingStack.h"
#include "myassert.h"


#define DEFAULT_MAX_STACK_SIZE 20
#define THREAD_TEST_DELAY_US 1000
#define READER_THREAD_COUNT 3
#define READER_ITERATIONS 200

/*
 * The stack to use during tests.
 */
static BlockingStack *stack;

/*
 * A simple person record used to verify that the
 * stack correctly handles structured data.
 */
typedef struct {
	int id;
	char name[50];
} Person;

/*
 * Arguments passed to a helper thread that pushes one element.
 */
typedef struct {
	BlockingStack *stack;
	void *element;
	int completed;
	bool result;
} PushThreadArgs;

/*
 * Arguments passed to a helper thread that pops one element.
 */
typedef struct {
	BlockingStack *stack;
	int completed;
	void *result;
} PopThreadArgs;

/*
 * Arguments passed to a reader thread that repeatedly checks
 * size and empty-state on a shared stack.
 */
typedef struct {
	BlockingStack *stack;
	int *startFlag;
	int iterations;
	int expectedSize;
	bool expectedEmpty;
	int failed;
} ReadOnlyThreadArgs;


/*
 * The number of tests that succeeded.
 */
static int success_count = 0;

/*
 * The total number of tests run.
 */
static int total_count = 0;


/*
 * Setup function to run prior to each test.
 */
void setup() {
	stack = new_BlockingStack(DEFAULT_MAX_STACK_SIZE);
	total_count++;
}

/*
 * Teardown function to run after each test.
 */
void teardown() {
	BlockingStack_destroy(stack);
}

/*
 * This function is called multiple times from main for each user-defined test function.
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
 * Thread entry point that attempts one push and records the outcome.
 */
static void *pushThreadMain(void *arg) {
	PushThreadArgs *args = (PushThreadArgs *) arg;
	args->result = BlockingStack_push(args->stack, args->element);
	args->completed = 1;
	return NULL;
}

/*
 * Thread entry point that performs one pop and stores the result.
 */
static void *popThreadMain(void *arg) {
	PopThreadArgs *args = (PopThreadArgs *) arg;
	args->result = BlockingStack_pop(args->stack);
	args->completed = 1;
	return NULL;
}

/*
 * Waits until the main test signals that worker threads may start.
 */
static void waitForThreadStart(int *startFlag) {
	while (!(*startFlag)) {
		usleep(THREAD_TEST_DELAY_US);
	}
}

/*
 * Thread entry point that repeatedly checks size and isEmpty.
 */
static void *readOnlyThreadMain(void *arg) {
	ReadOnlyThreadArgs *args = (ReadOnlyThreadArgs *) arg;

	waitForThreadStart(args->startFlag);

	for (int i = 0; i < args->iterations; i++) {
		if (BlockingStack_size(args->stack) != args->expectedSize) {
			args->failed = 1;
		}

		if (BlockingStack_isEmpty(args->stack) != args->expectedEmpty) {
			args->failed = 1;
		}
	}

	return NULL;
}


// Testing Basic Stack Operations //

/*
 * Checks that the BlockingStack constructor returns a non-NULL pointer.
 */
int newStackIsNotNull() {
	ASSERT_NOT_NULL(stack);
	return TEST_SUCCESS;
}

/*
 * Checks that the size of an empty blocking stack is 0.
 */
int newStackSizeZero() {
	ASSERT_INT_EQ(0, BlockingStack_size(stack));
	return TEST_SUCCESS;
}

/*
 * Checks that a new blocking stack reports that it is empty.
 */
int newStackIsEmpty() {
	ASSERT_TRUE(BlockingStack_isEmpty(stack));
	return TEST_SUCCESS;
}

/*
 * Checks that creating a stack of size 0 fails.
 */
int newStackInvalidZero() {
	BlockingStack *local_stack = new_BlockingStack(0);
	ASSERT_NULL(local_stack);
	return TEST_SUCCESS;
}

/*
 * Checks that creating a stack with negative size fails.
 */
int newStackInvalidNegative() {
	BlockingStack *local_stack = new_BlockingStack(-5);
	ASSERT_NULL(local_stack);
	return TEST_SUCCESS;
}

/*
 * Checks that pushing one element succeeds.
 */
int pushOneElement() {
	int value = 42;

	ASSERT_TRUE(BlockingStack_push(stack, &value));
	ASSERT_INT_EQ(1, BlockingStack_size(stack));
	ASSERT_FALSE(BlockingStack_isEmpty(stack));
	return TEST_SUCCESS;
}

/*
 * Checks that a pushed element can be popped again.
 */
int pushAndPopOneElement() {
	int value = 7;

	ASSERT_TRUE(BlockingStack_push(stack, &value));
	ASSERT_PTR_EQ(&value, BlockingStack_pop(stack));
	ASSERT_INT_EQ(0, BlockingStack_size(stack));
	ASSERT_TRUE(BlockingStack_isEmpty(stack));
	return TEST_SUCCESS;
}

/*
 * Checks that pushing NULL fails.
 */
int pushNullFails() {
	ASSERT_FALSE(BlockingStack_push(stack, NULL));
	ASSERT_INT_EQ(0, BlockingStack_size(stack));
	return TEST_SUCCESS;
}

/*
 * Checks that the stack follows LIFO order.
 */
int lifoOrderThreeElements() {
	int a = 10;
	int b = 20;
	int c = 30;

	ASSERT_TRUE(BlockingStack_push(stack, &a));
	ASSERT_TRUE(BlockingStack_push(stack, &b));
	ASSERT_TRUE(BlockingStack_push(stack, &c));

	ASSERT_PTR_EQ(&c, BlockingStack_pop(stack));
	ASSERT_PTR_EQ(&b, BlockingStack_pop(stack));
	ASSERT_PTR_EQ(&a, BlockingStack_pop(stack));
	ASSERT_INT_EQ(0, BlockingStack_size(stack));
	return TEST_SUCCESS;
}

/*
 * Checks that size updates correctly after push and pop operations.
 */
int sizeChangesCorrectly() {
	int a = 1;
	int b = 2;

	ASSERT_INT_EQ(0, BlockingStack_size(stack));
	ASSERT_TRUE(BlockingStack_push(stack, &a));
	ASSERT_INT_EQ(1, BlockingStack_size(stack));
	ASSERT_TRUE(BlockingStack_push(stack, &b));
	ASSERT_INT_EQ(2, BlockingStack_size(stack));
	ASSERT_PTR_EQ(&b, BlockingStack_pop(stack));
	ASSERT_INT_EQ(1, BlockingStack_size(stack));
	ASSERT_PTR_EQ(&a, BlockingStack_pop(stack));
	ASSERT_INT_EQ(0, BlockingStack_size(stack));
	return TEST_SUCCESS;
}

/*
 * Checks that empty/non-empty state changes correctly.
 */
int isEmptyChangesCorrectly() {
	int value = 123;

	ASSERT_TRUE(BlockingStack_isEmpty(stack));
	ASSERT_TRUE(BlockingStack_push(stack, &value));
	ASSERT_FALSE(BlockingStack_isEmpty(stack));
	ASSERT_PTR_EQ(&value, BlockingStack_pop(stack));
	ASSERT_TRUE(BlockingStack_isEmpty(stack));
	return TEST_SUCCESS;
}

/*
 * Checks that clear removes all elements from a non-empty stack.
 */
int clearEmptiesStack() {
	int a = 1;
	int b = 2;

	ASSERT_TRUE(BlockingStack_push(stack, &a));
	ASSERT_TRUE(BlockingStack_push(stack, &b));
	BlockingStack_clear(stack);
	ASSERT_TRUE(BlockingStack_isEmpty(stack));
	ASSERT_INT_EQ(0, BlockingStack_size(stack));
	return TEST_SUCCESS;
}

/*
 * Checks that clearing an already empty stack is safe.
 */
int clearOnEmptyStack() {
	BlockingStack_clear(stack);
	ASSERT_TRUE(BlockingStack_isEmpty(stack));
	ASSERT_INT_EQ(0, BlockingStack_size(stack));
	return TEST_SUCCESS;
}

/*
 * Checks that the stack can be reused after calling clear.
 */
int reuseAfterClear() {
	int a = 10;
	int b = 20;
	int c = 30;

	ASSERT_TRUE(BlockingStack_push(stack, &a));
	ASSERT_TRUE(BlockingStack_push(stack, &b));
	BlockingStack_clear(stack);
	ASSERT_TRUE(BlockingStack_isEmpty(stack));
	ASSERT_TRUE(BlockingStack_push(stack, &c));
	ASSERT_INT_EQ(1, BlockingStack_size(stack));
	ASSERT_PTR_EQ(&c, BlockingStack_pop(stack));
	return TEST_SUCCESS;
}

/*
 * Checks that integer pointers can be stored and retrieved correctly.
 */
int storeIntPointers() {
	int a = 5;
	int b = 7;

	ASSERT_TRUE(BlockingStack_push(stack, &a));
	ASSERT_TRUE(BlockingStack_push(stack, &b));

	int *pb = (int *) BlockingStack_pop(stack);
	int *pa = (int *) BlockingStack_pop(stack);

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

	ASSERT_TRUE(BlockingStack_push(stack, str1));
	ASSERT_TRUE(BlockingStack_push(stack, str2));

	char *p2 = (char *) BlockingStack_pop(stack);
	char *p1 = (char *) BlockingStack_pop(stack);

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

	ASSERT_TRUE(BlockingStack_push(stack, &p1));
	ASSERT_TRUE(BlockingStack_push(stack, &p2));

	Person *out2 = (Person *) BlockingStack_pop(stack);
	Person *out1 = (Person *) BlockingStack_pop(stack);

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
		ASSERT_TRUE(BlockingStack_push(stack, &values[i]));
		ASSERT_INT_EQ(i + 1, BlockingStack_size(stack));
	}

	for (int i = DEFAULT_MAX_STACK_SIZE - 1; i >= 0; i--) {
		int *value = (int *) BlockingStack_pop(stack);
		ASSERT_NOT_NULL(value);
		ASSERT_INT_EQ(i * 10, *value);
		ASSERT_INT_EQ(i, BlockingStack_size(stack));
	}

	ASSERT_TRUE(BlockingStack_isEmpty(stack));
	return TEST_SUCCESS;
}

/*
 * Checks alternating push and pop operations.
 */
int alternatingPushPop() {
	int a = 1;
	int b = 2;
	int c = 3;

	ASSERT_TRUE(BlockingStack_push(stack, &a));
	ASSERT_PTR_EQ(&a, BlockingStack_pop(stack));
	ASSERT_TRUE(BlockingStack_push(stack, &b));
	ASSERT_TRUE(BlockingStack_push(stack, &c));
	ASSERT_PTR_EQ(&c, BlockingStack_pop(stack));
	ASSERT_PTR_EQ(&b, BlockingStack_pop(stack));
	ASSERT_TRUE(BlockingStack_isEmpty(stack));
	ASSERT_INT_EQ(0, BlockingStack_size(stack));
	return TEST_SUCCESS;
}

/*
 * Checks that two separate stacks operate independently.
 */
int multipleStacksIndependent() {
	BlockingStack *first = new_BlockingStack(2);
	BlockingStack *second = new_BlockingStack(3);
	int a = 1;
	int b = 2;
	int c = 3;

	ASSERT_NOT_NULL(first);
	ASSERT_NOT_NULL(second);
	ASSERT_TRUE(BlockingStack_push(first, &a));
	ASSERT_TRUE(BlockingStack_push(second, &b));
	ASSERT_TRUE(BlockingStack_push(second, &c));
	ASSERT_INT_EQ(1, BlockingStack_size(first));
	ASSERT_INT_EQ(2, BlockingStack_size(second));
	ASSERT_PTR_EQ(&a, BlockingStack_pop(first));
	ASSERT_PTR_EQ(&c, BlockingStack_pop(second));
	ASSERT_PTR_EQ(&b, BlockingStack_pop(second));
	ASSERT_TRUE(BlockingStack_isEmpty(first));
	ASSERT_TRUE(BlockingStack_isEmpty(second));

	BlockingStack_destroy(first);
	BlockingStack_destroy(second);
	return TEST_SUCCESS;
}

// Testing Thread Safe Behaviour //

/*
 * Checks that a pop blocks until another thread pushes an element.
 */
int popBlocksUntilPush() {
	pthread_t thread;
	PopThreadArgs args = {stack, 0, NULL};
	int value = 99;

	ASSERT_INT_EQ(0, pthread_create(&thread, NULL, popThreadMain, &args));
	ASSERT_FALSE(args.completed);

	ASSERT_TRUE(BlockingStack_push(stack, &value));
	ASSERT_INT_EQ(0, pthread_join(thread, NULL));

	ASSERT_TRUE(args.completed);
	ASSERT_PTR_EQ(&value, args.result);
	ASSERT_INT_EQ(0, BlockingStack_size(stack));
	return TEST_SUCCESS;
}

/*
 * Checks that a push blocks until another thread pops an element from a full stack.
 */
int pushBlocksUntilPop() {
	BlockingStack *local_stack = new_BlockingStack(1);
	pthread_t thread;
	PushThreadArgs args;
	int first = 11;
	int second = 22;

	ASSERT_NOT_NULL(local_stack);
	ASSERT_TRUE(BlockingStack_push(local_stack, &first));

	args.stack = local_stack;
	args.element = &second;
	args.completed = 0;
	args.result = false;

	ASSERT_INT_EQ(0, pthread_create(&thread, NULL, pushThreadMain, &args));
	ASSERT_FALSE(args.completed);

	ASSERT_PTR_EQ(&first, BlockingStack_pop(local_stack));
	ASSERT_INT_EQ(0, pthread_join(thread, NULL));

	ASSERT_TRUE(args.completed);
	ASSERT_TRUE(args.result);
	ASSERT_INT_EQ(1, BlockingStack_size(local_stack));
	ASSERT_PTR_EQ(&second, BlockingStack_pop(local_stack));

	BlockingStack_destroy(local_stack);
	return TEST_SUCCESS;
}

/*
 * Checks that multiple blocked poppers can each receive a later push.
 */
int multipleBlockedPoppersReceiveItems() {
	pthread_t threads[2];
	PopThreadArgs args[2];
	int first = 100;
	int second = 200;

	for (int i = 0; i < 2; i++) {
		args[i].stack = stack;
		args[i].completed = 0;
		args[i].result = NULL;
		ASSERT_INT_EQ(0, pthread_create(&threads[i], NULL, popThreadMain, &args[i]));
	}

	ASSERT_FALSE(args[0].completed);
	ASSERT_FALSE(args[1].completed);

	ASSERT_TRUE(BlockingStack_push(stack, &first));
	ASSERT_TRUE(BlockingStack_push(stack, &second));

	ASSERT_INT_EQ(0, pthread_join(threads[0], NULL));
	ASSERT_INT_EQ(0, pthread_join(threads[1], NULL));

	ASSERT_TRUE(args[0].completed);
	ASSERT_TRUE(args[1].completed);
	ASSERT_TRUE(args[0].result == &first || args[0].result == &second);
	ASSERT_TRUE(args[1].result == &first || args[1].result == &second);
	ASSERT_TRUE(args[0].result != args[1].result);
	ASSERT_INT_EQ(0, BlockingStack_size(stack));
	return TEST_SUCCESS;
}

/*
 * Checks that clearing a full stack allows a blocked push to complete.
 */
int clearUnblocksWaitingPush() {
	BlockingStack *local_stack = new_BlockingStack(1);
	pthread_t thread;
	PushThreadArgs args;
	int first = 1;
	int second = 2;

	ASSERT_NOT_NULL(local_stack);
	ASSERT_TRUE(BlockingStack_push(local_stack, &first));

	args.stack = local_stack;
	args.element = &second;
	args.completed = 0;
	args.result = false;

	ASSERT_INT_EQ(0, pthread_create(&thread, NULL, pushThreadMain, &args));
	ASSERT_FALSE(args.completed);

	BlockingStack_clear(local_stack);
	ASSERT_INT_EQ(0, pthread_join(thread, NULL));

	ASSERT_TRUE(args.completed);
	ASSERT_TRUE(args.result);
	ASSERT_INT_EQ(1, BlockingStack_size(local_stack));
	ASSERT_PTR_EQ(&second, BlockingStack_pop(local_stack));

	BlockingStack_destroy(local_stack);
	return TEST_SUCCESS;
}


/*
 * Checks that concurrent readers safely observe an empty shared stack.
 */
int sizeAndIsEmptyAreSafeOnEmptyStack() {
	pthread_t threads[READER_THREAD_COUNT];
	ReadOnlyThreadArgs args[READER_THREAD_COUNT];
	int startFlag = 0;

	for (int i = 0; i < READER_THREAD_COUNT; i++) {
		args[i].stack = stack;
		args[i].startFlag = &startFlag;
		args[i].iterations = READER_ITERATIONS;
		args[i].expectedSize = 0;
		args[i].expectedEmpty = true;
		args[i].failed = 0;
		ASSERT_INT_EQ(0, pthread_create(&threads[i], NULL, readOnlyThreadMain, &args[i]));
	}

	startFlag = 1;

	for (int i = 0; i < READER_THREAD_COUNT; i++) {
		ASSERT_INT_EQ(0, pthread_join(threads[i], NULL));
		ASSERT_FALSE(args[i].failed);
	}

	ASSERT_INT_EQ(0, BlockingStack_size(stack));
	ASSERT_TRUE(BlockingStack_isEmpty(stack));
	return TEST_SUCCESS;
}

/*
 * Checks that concurrent readers safely observe a non-empty shared stack.
 */
int sizeAndIsEmptyAreSafeOnNonEmptyStack() {
	pthread_t threads[READER_THREAD_COUNT];
	ReadOnlyThreadArgs args[READER_THREAD_COUNT];
	int startFlag = 0;
	int values[3] = {10, 20, 30};

	for (int i = 0; i < 3; i++) {
		ASSERT_TRUE(BlockingStack_push(stack, &values[i]));
	}

	for (int i = 0; i < READER_THREAD_COUNT; i++) {
		args[i].stack = stack;
		args[i].startFlag = &startFlag;
		args[i].iterations = READER_ITERATIONS;
		args[i].expectedSize = 3;
		args[i].expectedEmpty = false;
		args[i].failed = 0;
		ASSERT_INT_EQ(0, pthread_create(&threads[i], NULL, readOnlyThreadMain, &args[i]));
	}

	startFlag = 1;

	for (int i = 0; i < READER_THREAD_COUNT; i++) {
		ASSERT_INT_EQ(0, pthread_join(threads[i], NULL));
		ASSERT_FALSE(args[i].failed);
	}

	ASSERT_INT_EQ(3, BlockingStack_size(stack));
	ASSERT_FALSE(BlockingStack_isEmpty(stack));

	for (int i = 0; i < 3; i++) {
		ASSERT_NOT_NULL(BlockingStack_pop(stack));
	}

	ASSERT_TRUE(BlockingStack_isEmpty(stack));
	return TEST_SUCCESS;
}



/*
 * Main function for the BlockingStack tests which will run each user-defined test in turn.
 */
int main() {
	runTest("newStackIsNotNull", newStackIsNotNull);
	runTest("newStackSizeZero", newStackSizeZero);
	runTest("newStackIsEmpty", newStackIsEmpty);
	runTest("newStackInvalidZero", newStackInvalidZero);
	runTest("newStackInvalidNegative", newStackInvalidNegative);
	runTest("pushOneElement", pushOneElement);
	runTest("pushAndPopOneElement", pushAndPopOneElement);
	runTest("pushNullFails", pushNullFails);
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
	runTest("popBlocksUntilPush", popBlocksUntilPush);
	runTest("pushBlocksUntilPop", pushBlocksUntilPop);
	runTest("multipleBlockedPoppersReceiveItems", multipleBlockedPoppersReceiveItems);
	runTest("clearUnblocksWaitingPush", clearUnblocksWaitingPush);
	runTest("sizeAndIsEmptyAreSafeOnEmptyStack", sizeAndIsEmptyAreSafeOnEmptyStack);
	runTest("sizeAndIsEmptyAreSafeOnNonEmptyStack", sizeAndIsEmptyAreSafeOnNonEmptyStack);
	printf("BlockingStack Tests complete: %d / %d tests successful.\n----------------\n", success_count, total_count);
	return 0;
}
