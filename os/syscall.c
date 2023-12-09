#include "syscall.h"
#include "defs.h"
#include "loader.h"
#include "syscall_ids.h"
#include "timer.h"
#include "trap.h"
#include "string.h"

uint64 sys_write(int fd, char *str, uint len)
{
	debugf("sys_write fd = %d str = %x, len = %d", fd, str, len);
	if (fd != STDOUT)
		return -1;
	for (int i = 0; i < len; ++i) {
		console_putchar(str[i]);
	}
	return len;
}

__attribute__((noreturn)) void sys_exit(int code)
{
	exit(code);
	__builtin_unreachable();
}

uint64 sys_sched_yield()
{
	yield();
	return 0;
}

uint64 sys_gettimeofday(TimeVal *val, int _tz)
{
	uint64 cycle = get_cycle();
	val->sec = cycle / CPU_FREQ;
	val->usec = (cycle % CPU_FREQ) * 1000000 / CPU_FREQ;
	return 0;
}

/*
* LAB1: you may need to define sys_task_info here
*/
uint64 sys_task_info(TaskInfo *ti) {
	struct proc *curr = curr_proc();
	ti->state = Running;
	for(uint32 i = 0; i < MAX_SYSCALL_NUM; i++) {
		ti->syscall_times[i] = curr->syscall_times[i];
	}
	uint64 curr_cycle = get_cycle();
	debugf("curr_cycle = %d, start_cycle = %d", curr_cycle, curr->start_cycle);
	ti->time = (curr_cycle - curr->start_cycle) * 1000000 / CPU_FREQ / 1000;
	return 0;
}

extern char trap_page[];

void syscall()
{
	struct proc *curr = curr_proc();
	struct trapframe *trapframe = curr->trapframe;
	int id = trapframe->a7, ret;
	uint64 args[6] = { trapframe->a0, trapframe->a1, trapframe->a2,
			   trapframe->a3, trapframe->a4, trapframe->a5 };
	tracef("syscall %d args = [%x, %x, %x, %x, %x, %x]", id, args[0],
	       args[1], args[2], args[3], args[4], args[5]);
	/*
	* LAB1: you may need to update syscall counter for task info here
	*/
	if (id >= MAX_SYSCALL_NUM) {
		errorf("syscall err id: %d", id);
		ret = -1;
		goto OUT;
	}
	curr->syscall_times[id]++;

	switch (id) {
	case SYS_write:
		ret = sys_write(args[0], (char *)args[1], args[2]);
		break;
	case SYS_exit:
		sys_exit(args[0]);
		// __builtin_unreachable();
	case SYS_sched_yield:
		ret = sys_sched_yield();
		break;
	case SYS_gettimeofday:
		ret = sys_gettimeofday((TimeVal *)args[0], args[1]);
		break;
	/*
	* LAB1: you may need to add SYS_taskinfo case here
	*/
	case SYS_task_info:
		ret = sys_task_info((TaskInfo *)args[0]);
		break;
	default:
		ret = -1;
		errorf("unknown syscall %d", id);
	}

OUT:
	trapframe->a0 = ret;
	tracef("syscall ret %d", ret);
}
