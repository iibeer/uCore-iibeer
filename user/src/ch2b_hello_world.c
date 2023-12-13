#include <stdio.h>
#include <unistd.h>

/// 正确输出：
/// Hello world from user mode program!
/// Test hello_world OK!

int main()
{
	puts("......ch2b_hello_world start......");
	puts("Hello world from user mode program!\nTest hello_world OK!");
	puts("......ch2b_hello_world finish......");
	return 0;
}