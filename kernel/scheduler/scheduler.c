#include "scheduler.h"
#include "vga.h"    // Use your VGA driver for output
#include "serial.h" // Use Serial for debug logs
#include "timer.h"  // Use your kernel tick

extern volatile uint32_t tick;
struct gt gt_table[MaxGThreads];
struct gt *gt_current;
static uint32_t next_random = 1;
int k_rand() {
    next_random = next_random * 1103515245 + 12345;
    return (uint32_t)(next_random / 65536) % 32768;
}

void scheduler_tick() {
    gt_yield();
}

int gt_strategy = 0; 

static void gt_stop(void) {
    gt_exit(0);
}

void gt_init(void) {
    for (int i = 0; i < MaxGThreads; i++) {
        gt_table[i].state = Unused;
        gt_table[i].switches = 0;
        gt_table[i].total_run_time = 0;
        gt_table[i].total_wait_time = 0;
    }
    gt_current = &gt_table[0];
    gt_current->state = Running;
    gt_current->priority = 5;      
    gt_current->current_priority = 5;
    gt_current->tickets = 10;      
    gt_current->last_change = (double)tick; // Use kernel tick instead of double time
    
    next_random = tick; // Seed our random with the current tick
}

int gt_create(void (*f)(void), int priority) {
    struct gt *p = 0;
    for (int i = 1; i < MaxGThreads; i++) {
        if (gt_table[i].state == Unused) {
            p = &gt_table[i];
            break;
        }
    }
    if (!p) return -1;
    serial_print("Creating thread with priority: ");
    serial_print_hex(priority);
    serial_print("\n");
    p->total_run_time = 0;
    p->total_wait_time = 0;
    p->switches = 0;

    if (priority < 0) priority = 0;
    if (priority > 10) priority = 10;
    p->priority = priority;
    p->current_priority = priority;
    p->tickets = (11 - priority) * 2; 

    // 32-BIT STACK INITIALIZATION[cite: 1, 4]
    uint32_t *stack_ptr = (uint32_t *)&p->stack[StackSize];
    
    *(--stack_ptr) = (uint32_t)gt_stop;
    *(--stack_ptr) = (uint32_t)f;
    
    // Initial register state for gt_switch[cite: 4]
    *(--stack_ptr) = 0; // ebp
    *(--stack_ptr) = 0; // ebx
    *(--stack_ptr) = 0; // esi
    *(--stack_ptr) = 0; // edi

    p->ctx.esp = (uint32_t)stack_ptr;
    p->state = Ready;
    p->last_change = (double)tick;

    return 0;
}

void gt_schedule(void) {
    struct gt *p = 0;
    struct gt_context *old_ctx, *new_ctx;
    double now = (double)tick;

    double delta = now - gt_current->last_change;
    if (gt_current->state == Running) {
        gt_current->total_run_time += delta;
    }
    gt_current->last_change = now;

    switch (gt_strategy) {
        case 1: { // Priority Scheduling
            for (int i = 0; i < MaxGThreads; i++) {
                if (gt_table[i].state == Ready && gt_table[i].current_priority > 0) {
                    gt_table[i].current_priority--;
                }
            }
            int highest_prio = 11;
            for (int i = 0; i < MaxGThreads; i++) {
                if (gt_table[i].state == Ready && gt_table[i].current_priority < highest_prio) {
                    highest_prio = gt_table[i].current_priority;
                    p = &gt_table[i];
                }
            }
            if (p) p->current_priority = p->priority;
            break;
        }
        case 2: { // Lottery Scheduling
            int total_tickets = 0;
            for (int i = 0; i < MaxGThreads; i++) {
                if (gt_table[i].state == Ready) total_tickets += gt_table[i].tickets;
            }
            if (total_tickets > 0) {
                int winner = k_rand() % total_tickets; // Using our internal k_rand
                int count = 0;
                // Use serial_print for debug so it doesn't clutter the main screen
                serial_print("Lottery winner selected.\n"); 
                for (int i = 0; i < MaxGThreads; i++) {
                    if (gt_table[i].state == Ready) {
                        count += gt_table[i].tickets;
                        if (count > winner) {
                            p = &gt_table[i];
                            break;
                        }
                    }
                }
            }
            break;
        }
        default: // Round Robin
            p = gt_current;
            while (1) {
                if (++p == &gt_table[MaxGThreads]) p = &gt_table[0];
                if (p->state == Ready) break;
                if (p == gt_current) return; 
            }
            break;
    }

    if (p == gt_current && gt_current->state == Running) return;
    if (!p) return; 

    p->total_wait_time += (now - p->last_change);
    p->last_change = now;
    p->switches++;

    if (gt_current->state != Unused) gt_current->state = Ready;
    p->state = Running;

    old_ctx = &gt_current->ctx;
    new_ctx = &p->ctx;
    gt_current = p;

    serial_print("Switching to thread with priority: ");
    serial_print_hex(p->priority);
    serial_print("\n");
    serial_print("process number:");
    serial_print_hex(p - gt_table); // Print thread index for debugging
    gt_switch(old_ctx, new_ctx);
  
}

void gt_yield(void) {
    gt_schedule();
}

void gt_exit(int code) {
    (void)code; 
    gt_current->state = Unused;
    gt_schedule();
    while(1); 
}