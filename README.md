# Thread-Safe Stack Implementation

This project delivers two fixed-capacity stack implementations in C:

- `Stack`: a generic array-backed LIFO stack storing `void *` elements
- `BlockingStack`: a thread-safe blocking stack that layers synchronization on top of `Stack`

The codebase focuses on clean API boundaries, predictable memory ownership, and correct concurrent behaviour using POSIX mutexes and semaphores.

## Highlights

- Fixed-size generic stack using dynamic allocation
- Blocking producer/consumer semantics without busy-waiting
- Internal synchronisation only: callers do not manage locks or semaphores
- Unit tests for both single-threaded and multi-threaded behaviour
- Clear separation between storage and coordination

## Architecture

### `Stack`

`Stack` is the underlying storage abstraction. It maintains:

- a dynamically allocated array of `void *`
- a maximum capacity
- a current size counter

Core characteristics:

- LIFO ordering
- fixed capacity after construction
- push rejects `NULL`
- pop returns `NULL` when empty
- no internal concurrency support

This module keeps the storage logic independent from any synchronisation concerns.

### `BlockingStack`

`BlockingStack` wraps a `Stack` instance and adds safe concurrent access plus blocking semantics for full and empty states.

Internally it uses:

- `pthread_mutex_t` to serialize access to the shared stack
- `sem_t items` to count elements available for pop
- `sem_t spaces` to count free capacity available for push

Operationally:

- `BlockingStack_push` waits on `spaces`, locks the stack, performs the push, then signals `items`
- `BlockingStack_pop` waits on `items`, locks the stack, performs the pop, then signals `spaces`

This design avoids busy-waiting and gives the module bounded-buffer style behaviour while preserving the stack interface.

## Why This Design

The implementation deliberately reuses the non-thread-safe `Stack` inside `BlockingStack` rather than duplicating storage logic.

That split has a few benefits:

- storage correctness and concurrency correctness are handled separately
- the blocking implementation stays small and easier to reason about
- the underlying stack can be tested independently
- changes to synchronisation do not require reworking the storage layer

## API Behaviour

### `Stack`

- `new_Stack(max_size)` returns `NULL` when `max_size <= 0`
- `Stack_push` returns `false` for `NULL` elements or when the stack is full
- `Stack_pop` returns `NULL` when the stack is empty
- `Stack_clear` removes all stored elements but does not free the pointed-to memory

### `BlockingStack`

- `new_BlockingStack(max_size)` returns `NULL` when `max_size <= 0`
- `BlockingStack_push` blocks when the stack is full
- `BlockingStack_pop` blocks when the stack is empty
- `BlockingStack_size` and `BlockingStack_isEmpty` are synchronized internally
- callers do not need to coordinate with any mutexes or semaphores directly

## Project Layout

- src/Stack.h
- src/Stack.c
- src/BlockingStack.h
- src/BlockingStack.c
- src/TestStack.c
- src/TestBlockingStack.c
- src/Makefile

## Build

From `src/`:

```bash
make
```

## Test

From `src/`:

```bash
./TestStack
./TestBlockingStack
```

The test suites cover:

- constructor validation
- push/pop correctness
- size and empty-state transitions
- reuse after clearing
- pointer handling for multiple data shapes
- LIFO ordering
- blocking behaviour across multiple threads

## Engineering Notes

- The stacks store pointers, not object copies
- Memory ownership for pointed-to data remains with the caller
- `BlockingStack_destroy` assumes no other threads are still blocked on or using the same instance

## Example

```c
BlockingStack *stack = new_BlockingStack(10);
int value = 42;

BlockingStack_push(stack, &value);
int *result = (int *) BlockingStack_pop(stack);

BlockingStack_destroy(stack);
```
