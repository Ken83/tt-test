#!/bin/bash

# tt-test test script only for bash and gcc.
# This script is available under a Public Domain.

CC='gcc -xc - -o a.out'

# Check for TEST() macro
test[0]='#include "../tt-test.h"
static void setup(void) {}
static void teardown(void) {}
TEST("test") {printf("pass");}
int main(void) {return run_all_tests();}'
expect[0]='pass: 1/1 passed'

# Check for some TEST() macro
test[1]='#include "../tt-test.h"
static void setup(void) {}
static void teardown(void) {}
TEST("test1") {printf("pass1");}
TEST("test2") {printf("pass2");}
int main(void) {return run_all_tests();}'
expect[1]='pass1pass2: 2/2 passed'

# Check for 1024 TEST() macro
test[2]='#include "../tt-test.h"
static void setup(void) {}
static void teardown(void) {}\n'
for ((i = 0; i < 1024; i++)) {
	test[2]="${test[2]}TEST(\"\") {}\n"
}
test[2]="${test[2]}int main(void) {return run_all_tests();}"
expect[2]=': 1024/1024 passed'

# Check for 1025 TEST() macro (ignore 1 test case)
test[3]='#include "../tt-test.h"
static void setup(void) {}
static void teardown(void) {}\n'
for ((i = 0; i < 1025; i++)) {
	test[3]="${test[3]}TEST(\"\") {}\n"
}
test[3]="${test[3]}int main(void) {return run_all_tests();}"
expect[3]=': 1024/1024 passed'

# Check for assert_equal() success
test[4]='#include "../tt-test.h"
static void setup(void) {}
static void teardown(void) {}
TEST("test") {assert_equal(0, 0);}
int main(void) {return run_all_tests();}'
expect[4]=': 1/1 passed'

# Check for assert_equal() fail
test[5]='#include "../tt-test.h"
static void setup(void) {}
static void teardown(void) {}
TEST("test") {assert_equal(0, 1);}
int main(void) {return run_all_tests();}'
expect[5]=':4: test\n input:    1\n actual:   1\n expected: 0\n: 0/1 passed'

# Check for assert_equal() fail in sub function
test[6]='#include "../tt-test.h"
static void setup(void) {}
static void teardown(void) {}
void foo(void) {assert_equal(0, 1);}
TEST("test") {foo();}
int main(void) {return run_all_tests();}'
expect[6]=':4: test\n input:    1\n actual:   1\n expected: 0\n: 0/1 passed'

# Check for setup(), teardown() available
test[7]='#include "../tt-test.h"
static void setup(void) {printf("begin");}
static void teardown(void) {printf("end");}
TEST("test1") {printf("test1");}
TEST("test2") {printf("test2");}
int main(void) {return run_all_tests();}'
expect[7]='begintest1endbegintest2end: 2/2 passed'

# Check for catching SIGFPE fault
test[8]='#include "../tt-test.h"
static void setup(void) {}
static void teardown(void) {}
int foo(int lval, int rval) {return lval / rval;}
TEST("test1") {foo(1, 0);}
TEST("test2") {printf("no arrival");}
int main(void) {return run_all_tests();}'
expect[8]='[8] TEST("test1") was aborted'

# Check for catching SIGSEGV fault
test[9]='#include "../tt-test.h"
static void setup(void) {}
static void teardown(void) {}
TEST("test") {static int g[4096]; g[4096*4096] = 1;}
int main(void) {return run_all_tests();}'
expect[9]='[11] TEST("test") was aborted'

# Check for assert_equal() args are executed just one time
test[10]='#include "../tt-test.h"
static void setup(void) {}
static void teardown(void) {}
int foo(void) {static int n = 0; return n++;}
int bar(void) {static int n = 0; return n++;}
TEST("test") {foo(); bar(); assert_equal(1, foo()); assert_equal(1, bar());}
int main(void) {return run_all_tests();}'
expect[10]=': 1/1 passed'

for ((i = 0; i < ${#test[@]}; i++)) {
	# Compile
	echo -e "${test[i]}" | $CC 2> /dev/null
	if [ "$?" -ne 0 ]; then
		echo test["$i"]: Compile Error
		continue
	fi

	# Execute
	actual=`./a.out` 2> /dev/null
	diff <(echo -e "$actual") <(echo -e "${expect[i]}")
	if [ "$?" -eq 0 ]; then
		echo test["$i"]: Pass
	else
		echo test["$i"]: Fail
	fi
}

rm a.out

