#include "syscall.h"
#include "defs.h"
#include "loader.h"
#include "syscall_ids.h"
#include "timer.h"
#include "trap.h"

uint64 sys_write(int fd, uint64 va, uint len)
{
	debugf("sys_write fd = %d va = %x, len = %d", fd, va, len);
	if (fd != STDOUT)
		return -1;
	struct proc *p = curr_proc();
	char str[MAX_STR_LEN];
	int size = copyinstr(p->pagetable, str, va, MIN(len, MAX_STR_LEN));
	debugf("size = %d", size);
	for (int i = 0; i < size; ++i) {
		console_putchar(str[i]);
	}
	return size;
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

uint64 sys_gettimeofday(uint64 va, int _tz) // TODO: implement sys_gettimeofday in pagetable. (VA to PA)
{
	// YOUR CODE
	struct proc *p = curr_proc();
	TimeVal val;
	uint64 cycle = get_cycle();
	val.sec = cycle / CPU_FREQ;
	val.usec = (cycle % CPU_FREQ) * 1000000 / CPU_FREQ;
	copyout(p->pagetable, va, (char *)&val, sizeof(val));
	return 0;
}

// TODO: add support for mmap and munmap syscall.
// hint: read through docstrings in vm.c. Watching CH4 video may also help.
// Note the return value and PTE flags (especially U,X,W,R)
int sys_mmap(uint64 va, unsigned long long len, int port, int flag, int fd) {
	if (len == 0) {
		return 0;
	}
	if (len > 1024 * 1024 * 1024) {
		errorf("sys_mmap: the maxinum len is 1G");
		return -1;
	}
	if ((va & (PAGE_SIZE - 1)) !=0 ) {
		errorf("start address must be aligned to page size");
		return -1;
	}
	if ((port & ~0x7) != 0 || (port & 0x7) == 0) {
		errorf("not allow port: 0x%x", port);
		return -1;
	}

	struct proc *p = curr_proc();
	int perm = (port << 1) | PTE_U;
	uint32 page_num = (len + PAGE_SIZE - 1) / PAGE_SIZE;
	for (uint32 i = 0; i < page_num; i++) {
		uint64 pa;
		if ((pa = (uint64)kalloc()) <= 0) {
			errorf("sys_mmap: kalloc physical page failed");
			return -1;
		}
		if (mappages(p->pagetable, va + PAGE_SIZE * i,
				PAGE_SIZE, pa, perm) != 0) {
			errorf("sys_mmap: mappages failed");
			return -1;
		}
	}
	return 0;
} 

extern pte_t *walk(pagetable_t, uint64, int);
int sys_munmap(uint64 va, unsigned long long len) {
	if (len == 0) {
		return 0;
	}
	if (len > 1024 * 1024 * 1024) {
		errorf("sys_mmap: the maxinum len is 1G");
		return -1;
	}
	if ((va & (PAGE_SIZE - 1)) !=0 ) {
		errorf("start address must be aligned to page size");
		return -1;
	}

	struct proc *p = curr_proc();
	uint32 page_num = (len + PAGE_SIZE - 1) / PAGE_SIZE;
	
	pte_t *pte;
	for (uint8 i = 0; i < page_num; i++) {
		if ((pte = walk(p->pagetable, va + i * PAGE_SIZE, 0)) == 0) {
			errorf("virtual address: 0x%x not have PTE", va + i * PAGE_SIZE);
			return -1;
		}
		if ((*pte & PTE_V) == 0) {
			errorf("virtual address: 0x%x PTE no mapping Physical address", va + i * PAGE_SIZE);
			return -1;
		}
		if ((*pte & PTE_V) != 0) {
			if (PTE_FLAGS(*pte) == PTE_V)
				panic("uvmunmap: not a leaf");
			uint64 pa = PTE2PA(*pte);
			kfree((void *)pa);
		}
		*pte = 0;
	}

	return 0;
}

/*
* LAB1: you may need to define sys_task_info here
*/
uint64 sys_task_info(uint64 va) {
	struct proc *p = curr_proc();
	uint64 curr_cycle = get_cycle();
	debugf("curr_cycle = %d, start_cycle = %d", curr_cycle, p->start_cycle);
	TaskInfo ti;
	ti.state = Running;
	ti.time = (curr_cycle - p->start_cycle) * 1000000 / CPU_FREQ / 1000;
	for(uint32 i = 0; i < MAX_SYSCALL_NUM; i++) {
		ti.syscall_times[i] = p->syscall_times[i];
	}
	copyout(p->pagetable, va, (char *)&ti, sizeof(ti));
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
		ret = sys_write(args[0], args[1], args[2]);
		break;
	case SYS_exit:
		sys_exit(args[0]);
		// __builtin_unreachable();
	case SYS_sched_yield:
		ret = sys_sched_yield();
		break;
	case SYS_gettimeofday:
		ret = sys_gettimeofday(args[0], args[1]);
		break;
	/*
	* LAB1: you may need to add SYS_taskinfo case here
	*/
	case SYS_task_info:
		ret = sys_task_info(args[0]);
		break;
	case SYS_mmap:
		ret = sys_mmap(args[0], args[1], args[2], args[3], args[4]);
		break;
	case SYS_munmap:
		ret = sys_munmap(args[0], args[1]);
		break;
	default:
		ret = -1;
		errorf("unknown syscall %d", id);
	}

OUT:
	trapframe->a0 = ret;
	tracef("syscall ret %d", ret);
}
