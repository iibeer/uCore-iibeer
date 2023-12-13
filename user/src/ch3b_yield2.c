#include <unistd.h>
#include <stdio.h>

const int WIDTH = 10;
const int HEIGHT = 5;

/*
理想结果：三个程序交替输出 ABC
*/

int main()
{
	puts("......ch3b_yield2 start......");
	for (int i = 0; i < HEIGHT; ++i) {
		char buf[WIDTH + 1];
		for (int j = 0; j < WIDTH; ++j)
			buf[j] = 'B';
		buf[WIDTH] = 0;
		printf("%s [%d/%d]\n", buf, i + 1, HEIGHT);
		sched_yield();
	}
	puts("Test write B OK!");
	puts("......ch3b_yield2 finish......");
	return 0;
}