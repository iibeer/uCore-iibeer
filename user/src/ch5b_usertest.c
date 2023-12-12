#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "usertest.h"
/// 辅助测例，运行所有其他测例。

/* 此案例存在BUG， TESTS位NULL，暂时未排查出原因，先规避 
*  经测试，使用char* str = "xxx" 方式定义的字符串变量均是null; 使用 char str[]方式定义的字符串变量则表现正常
*  先规避采用char str_array[][n]方式定义TESTS数组
*/
const char TESTS[][MAX_TEST_CASE_NAME] = {
	"ch2b_hello_world\0", "ch2b_power\0",	  //"ch2b_write1\0",
	"ch3b_sleep\0",	      "ch3b_sleep1\0",	  "ch3b_yield0\0",
	"ch3b_yield1\0",      "ch3b_yield2\0",	  "ch5b_getpid\0",
	"ch5b_forktest0\0",   "ch5b_forktest1\0", "ch5b_forktest2\0",
};

int main()
{
	int num_test = sizeof(TESTS) / sizeof(TESTS[0]);
	int succ = run_tests(TESTS, num_test);
	assert_eq(succ, num_test);
	puts("ch5b Usertests passed!");
	return 0;
}
