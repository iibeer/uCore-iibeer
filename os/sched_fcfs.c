#include "defs.h"
#include "log.h"
#include "queue.h"

extern struct proc pool[NPROC];
static struct queue task_queue;

static int fcfs_init() {
    init_queue(&task_queue);
    return 0;
}

static void fcfs_push_queue(struct proc *p) {
    push_queue(&task_queue, p - pool);
    debugf("fcfs: add task %d(pid=%d) to task queue\n", p, p->pid);
}

static struct proc* fcfs_pick_next() {
    int index = pop_queue(&task_queue);
    if (index < 0) {
        debugf("No task to pick as next\n");
        return NULL;
    }
    debugf("fcfs: pick next task %d(pid=%d) from task queue\n", index, pool[index].pid);
    return pool + index;
}

static void fcfs_pop_queue(struct proc *p) {
    return;
}

static void fcfs_process_tick() {
    return;
}

struct sched_algorithm algorithm_fcfs = {
    .name = "fcfs",
    .init = fcfs_init,
    .push_queue = fcfs_push_queue,
    .pop_queue = fcfs_pop_queue,
    .pick_next = fcfs_pick_next,
    .process_tick = fcfs_process_tick
};
