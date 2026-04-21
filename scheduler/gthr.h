#ifndef GTHR_H
#define GTHR_H

#include <stdint.h>
#include <stdbool.h>

enum {
    MaxGThreads = 5,
    StackSize = 0x4000, 
};

struct gt {
    struct gt_context {
        uint64_t rsp, r15, r14, r13, r12, rbx, rbp;
    } ctx;
    enum { Unused, Running, Ready } state;
    uint8_t stack[StackSize] __attribute__((aligned(16)));
    
    int priority; // For priority scheduling (not implemented yet)
    int tickets;  // For lottery scheduling (not implemented yet)

    // Statistics
    double last_change;
    double total_run_time;
    double total_wait_time;
    int switches;
    double min_run;
    double max_run;
    double sum_sq_run;
};

// External access to thread table for platform layer
extern struct gt gt_table[MaxGThreads];
extern struct gt *gt_current;

// Scheduler API
void gt_init(void);
int  gt_create(void (*f)(void));
void gt_yield(void);
void gt_exit(int code);
void gt_schedule(void);

// Missing declaration for assembly context switch
void gt_switch(struct gt_context *old, struct gt_context *new);

// Platform Abstraction Layer (HAL)
double gt_plat_get_time(void);
void   gt_plat_init_timer(void);
void   gt_plat_reset_timer(void);
void   gt_plat_print_stats(int sig);

#endif