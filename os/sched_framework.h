#ifndef SCHED_FRAMEWORK_H
#define SCHED_FRAMEWORK_H
#include <proc.h>

#define MAX_ALGORITHM_NAME 32
struct sched_algorithm {
    char name[MAX_ALGORITHM_NAME];
    int (*init)();
    void (*push_queue)(struct proc *p);
    void (*pop_queue)(struct proc *p);
    struct proc* (*pick_next)();
    void (*process_tick)();
};

#endif