#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <stdbool.h>

enum {
    MaxGThreads = 5,
    StackSize = 0x4000, 
};

struct gt {
   struct gt_context {
        uint32_t esp, edi, esi, ebp, ebx; 
    } ctx;
    enum { Unused, Running, Ready } state;
    uint8_t stack[StackSize] __attribute__((aligned(16)));
    
    int priority;
    int current_priority;
    int tickets;

    uint32_t last_change;     // Changed from double to uint32_t for kernel stability
    uint32_t total_run_time;
    uint32_t total_wait_time;
    int switches;
};

// CHANGE: Add 'extern' to prevent multiple definitions[cite: 4]
extern struct gt gt_table[MaxGThreads];
extern struct gt *gt_current;
extern int gt_strategy;

// Scheduler API
void gt_init(void);
int  gt_create(void (*f)(void), int priority);
void gt_yield(void);
void gt_exit(int code);
void gt_schedule(void);
void gt_switch(struct gt_context *old, struct gt_context *new);

#endif