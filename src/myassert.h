/*
 * myassert.h
 *
 * Shared assertion macros for unit tests.
 *
 */

#ifndef SRC_MYASSERT_H_
#define SRC_MYASSERT_H_

#define ASSERTION_FAILURE 0
#define TEST_SUCCESS 1

#define ASSERT_TRUE(condition) do {\
	if (!(condition)) {\
		printf("Assertion failed in %s (%s line %d): %s is false\n", __FUNCTION__, __FILE__, __LINE__, #condition);\
		return ASSERTION_FAILURE;\
	}\
} while(0)

#define ASSERT_FALSE(condition) do {\
	if (condition) {\
		printf("Assertion failed in %s (%s line %d): %s is true\n", __FUNCTION__, __FILE__, __LINE__, #condition);\
		return ASSERTION_FAILURE;\
	}\
} while(0)

#define ASSERT_INT_EQ(expected, actual) do {\
	int exp_val = (expected);\
	int act_val = (actual);\
	if (exp_val != act_val) {\
		printf("Assertion failed in %s (%s line %d): expected %d, got %d\n", __FUNCTION__, __FILE__, __LINE__, exp_val, act_val);\
		return ASSERTION_FAILURE;\
	}\
} while(0)

#define ASSERT_PTR_EQ(expected, actual) do {\
	void *exp_ptr = (void *)(expected);\
	void *act_ptr = (void *)(actual);\
	if (exp_ptr != act_ptr) {\
		printf("Assertion failed in %s (%s line %d): expected pointer %p, got %p\n", __FUNCTION__, __FILE__, __LINE__, exp_ptr, act_ptr);\
		return ASSERTION_FAILURE;\
	}\
} while(0)

#define ASSERT_NOT_NULL(ptr) do {\
	if ((ptr) == NULL) {\
		printf("Assertion failed in %s (%s line %d): %s is NULL\n", __FUNCTION__, __FILE__, __LINE__, #ptr);\
		return ASSERTION_FAILURE;\
	}\
} while(0)

#define ASSERT_NULL(ptr) do {\
	if ((ptr) != NULL) {\
		printf("Assertion failed in %s (%s line %d): %s is not NULL\n", __FUNCTION__, __FILE__, __LINE__, #ptr);\
		return ASSERTION_FAILURE;\
	}\
} while(0)

#endif /* SRC_MYASSERT_H_ */