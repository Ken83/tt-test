/*
*  tt-test - Tiny and Training xUnit Testing Framework for C.
*  This source code is available under a Public Domain.
*
*  A test case template is the following.
*
#include "tt-test.h"

static void setup(void) {}
static void teardown(void) {}

TEST("add() failed for typical input") {
	assert_equal(3, add(1, 2));
}

int main(void) {
	return run_all_tests();
}
*/
#ifndef TT_TEST_H
#define TT_TEST_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>

/* Generate function name with line no. TT_FUNC(xxx)(void) => xxx_000(void) */
#define TT_FUNC(str) TT_FUNC_(str##_, __LINE__)
#define TT_FUNC_(str, line) TT_FUNC__(str, line)
#define TT_FUNC__(str, line) str##line

#define TT_MAX_TEST_REGISTRATION 1024
static int tt_num_registed_tests = 0;
static int(*tt_registed_tests[TT_MAX_TEST_REGISTRATION])(void);
static const char *tt_test_message;
static jmp_buf tt_assert_jmpbuf;

/* Catch SIGILL, SIGFPE (not for VC) or SIGSEGV */
static void tt_handler_abort(int signal_id) {
	printf("[%d] TEST(\"%s\") was aborted\n", signal_id, tt_test_message);
	abort();
}

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

/* tt_constructor_000() will be called before main() and regist tt_caller_000() */
#define TEST(mes)\
static void TT_FUNC(tt_autogen_func)(void);\
int TT_FUNC(tt_caller)(void) {\
	signal(SIGILL, &tt_handler_abort);\
	signal(SIGFPE, &tt_handler_abort);\
	signal(SIGSEGV, &tt_handler_abort);\
	tt_test_message = mes;\
	if (setjmp(tt_assert_jmpbuf) != 0)\
		return 1;\
	setup();\
	TT_FUNC(tt_autogen_func)();\
	teardown();\
	return 0;\
}\
TT_ATTRIBUTE static void TT_FUNC(tt_constructor)(void) {\
	if (tt_num_registed_tests < TT_MAX_TEST_REGISTRATION)\
		tt_registed_tests[tt_num_registed_tests++] = TT_FUNC(tt_caller);\
}\
TT_SECTION \
static void TT_FUNC(tt_autogen_func) (void)

/* Use this function in a TEST() */
#define assert_equal(lval, rval)\
do {\
	if (lval == rval)\
		break;\
	printf("%s:%d: %s\n input:    %s\n actual:   %d\n expected: %d\n",\
		TT_FILE, __LINE__, tt_test_message, #rval, rval, lval);\
	longjmp(tt_assert_jmpbuf, 1);\
}while (0)

/* Call order: run_all_tests_() => tt_caller_000() => tt_autogen_func_000() */
static int run_all_tests_(const char *filename) {
	int i, num_pass = 0;
	for (i = 0; i < tt_num_registed_tests; i++)
		num_pass += !tt_registed_tests[i]();
	printf("%s: %d/%d passed\n", filename, num_pass, tt_num_registed_tests);
	return !(num_pass == tt_num_registed_tests);
}
#define run_all_tests() run_all_tests_(TT_FILE)

#endif
