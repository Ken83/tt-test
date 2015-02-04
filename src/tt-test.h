/*
*  tt-test - Tiny and Training xUnit Testing Framework for C.
*  This source code is available under a Public Domain.
*
*  A test case template is the following.
*
#include "tt-test.h"

static void setup(void) {}

static void teardown(void) {}

TEST("add() 1 + 2 should be 3") {
	assert_equal(3, add(1, 2));
}

int main(void) {
	return run_all_tests();
}
*/
#ifndef TT_TEST_H
#define TT_TEST_H

#include <stdio.h>

#define MAX_TESTS 1024

#define TT_FUNC(str) TT_FUNC_(str##_, __LINE__)
#define TT_FUNC_(str, line) TT_FUNC__(str, line)
#define TT_FUNC__(str, line) str##line

static int tt_num_tests = 0;
static int(*tt_func_array[MAX_TESTS])(void);

/* gcc2.7 and later (and Clang) are available */
#if defined(__GNUC__) && (__GNUC__ >= 3 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 7))
#define TT_FILE __BASE_FILE__
#define TT_ATTRIBUTE __attribute__((constructor))
#define TT_SECTION

/* VC2008 and later are available */
#elif defined (_MSC_VER) && (_MSC_VER >= 1500)
#include <string.h>
#define TT_FILE (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define TT_ATTRIBUTE
#define TT_SECTION \
	__pragma(section(".CRT$XCU", read))\
	__declspec(allocate(".CRT$XCU"))\
	void(*TT_FUNC(tt_register))(void) = TT_FUNC(tt_constructor);
#endif

#define TEST(mes)\
static void TT_FUNC(tt_autogen_func)(const char*, int*);\
int TT_FUNC(tt_caller)(void) {\
	int failed = 0;\
	setup();\
	TT_FUNC(tt_autogen_func)(mes, &failed);\
	teardown();\
	return failed;\
}\
TT_ATTRIBUTE static void TT_FUNC(tt_constructor)(void) {\
	if (tt_num_tests < MAX_TESTS)\
		tt_func_array[tt_num_tests++] = TT_FUNC(tt_caller);\
}\
TT_SECTION \
static void TT_FUNC(tt_autogen_func) (const char *tt_mes, int *tt_failed)

#define assert_equal(lval, rval)\
do {\
	if (lval == rval)\
		break;\
	printf("%s:%d: %s\n input:    %s\n actual:   %d\n expected: %d\n",\
		TT_FILE, __LINE__, tt_mes, #rval, rval, lval);\
	*tt_failed = 1;\
	return;\
}while (0)

static int run_all_tests_(const char *filename) {
	int i, num_pass = 0;
	for (i = 0; i < tt_num_tests; i++)
		num_pass += !tt_func_array[i]();
	printf("%s: %d/%d passed\n", filename, num_pass, tt_num_tests);
	return !(num_pass == tt_num_tests);
}
#define run_all_tests() run_all_tests_(TT_FILE)

#endif
