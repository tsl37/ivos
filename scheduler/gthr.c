#include "gthr.h"
#include "gthr_struct.h"


// --- Core Scheduler Logic (Freestanding) ---

static void gt_stop(void) {
    gt_exit(0);
}

void gt_init(void) {
    for (int i = 0; i < MaxGThreads; i++) {
        gt_table[i].state = Unused;
        gt_table[i].switches = 0;
    }
    gt_current = &gt_table[0];
    gt_current->state = Running;
    gt_current->last_change = gt_plat_get_time();
    
    gt_plat_init_timer();
}

int gt_create(void (*f)(void)) {
    struct gt *p = 0;
    for (int i = 0; i < MaxGThreads; i++) {
        if (gt_table[i].state == Unused) {
            p = &gt_table[i];
            break;
        }
    }
    if (!p) return -1;

    // Reset statistics
    p->total_run_time = 0;
    p->total_wait_time = 0;
    p->switches = 0;
    p->sum_sq_run = 0;
    p->max_run = 0;
    p->min_run = 0;

    // Static stack usage (no malloc)
    uint64_t *stack_ptr = (uint64_t *)&p->stack[StackSize];
    *(--stack_ptr) = (uint64_t)gt_stop;
    *(--stack_ptr) = (uint64_t)f;
    
    p->ctx.rsp = (uint64_t)stack_ptr;
    p->state = Ready;
    p->last_change = gt_plat_get_time();

    return 0;
}

void gt_schedule(void) {
    struct gt *p;
    struct gt_context *old_ctx, *new_ctx;
    double now = gt_plat_get_time();

    gt_plat_reset_timer();

    
    double delta = now - gt_current->last_change;
    if (gt_current->state == Running) {
        gt_current->total_run_time += delta;
        gt_current->switches++;
        if (delta < gt_current->min_run || gt_current->switches == 1) gt_current->min_run = delta;
        if (delta > gt_current->max_run) gt_current->max_run = delta;
        gt_current->sum_sq_run += (delta * delta);
    }
    gt_current->last_change = now;

    int strategy = 0;
    switch (strategy)
    {
        case 0: // Round Robin
            p = gt_current;
            while (1) {
                if (++p == &gt_table[MaxGThreads]) p = &gt_table[0];
                if (p->state == Ready) break;
                if (p == gt_current) return; // No other threads to run
            }
            break;

        case 1: // priority
        break;

        case 2: // lottery
        break;

    }




   

    // 3. Update stats for incoming thread
    p->total_wait_time += (now - p->last_change);
    p->last_change = now;

    if (gt_current->state != Unused) gt_current->state = Ready;
    p->state = Running;

    old_ctx = &gt_current->ctx;
    new_ctx = &p->ctx;
    gt_current = p;

    gt_switch(old_ctx, new_ctx);
}

void gt_yield(void) {
    gt_schedule();
}

void gt_exit(int code) {
    gt_current->state = Unused;
    gt_schedule();
    while(1); // Should never reach here
}