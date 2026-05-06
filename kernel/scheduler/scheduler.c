#include "gthr.h"
#include "gthr.h"

void scheduler_tick() {
    gt_yield();
    return;
}




int gt_strategy = 0; // Výchozí strategie je Round Robin

static void gt_stop(void) {
    gt_exit(0);
}

void gt_init(void) {
    for (int i = 0; i < MaxGThreads; i++) {
        gt_table[i].state = Unused;
        gt_table[i].switches = 0;
        // Inicializace statistik na nulu
        gt_table[i].total_run_time = 0;
        gt_table[i].total_wait_time = 0;
    }
    gt_current = &gt_table[0];
    gt_current->state = Running;
    gt_current->priority = 5;      // Nastavit výchozí prioritu pro main
    gt_current->current_priority = 5;
    gt_current->tickets = 10;      // Nastavit lístky pro main
    gt_current->last_change = gt_plat_get_time();
    
    gt_plat_init_timer();
}
int gt_create(void (*f)(void), int priority) {
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

    // Nastavení priorit a lístků
    if (priority < 0) priority = 0;
    if (priority > 10) priority = 10;
    p->priority = priority;
    p->current_priority = priority;
    // Výpočet lístků: čím vyšší priorita (nižší číslo), tím více lístků
    p->tickets = (11 - priority) * 2; 

    // Static stack usage
    uint64_t *stack_ptr = (uint64_t *)&p->stack[StackSize];
    *(--stack_ptr) = (uint64_t)gt_stop;
    *(--stack_ptr) = (uint64_t)f;
    
    p->ctx.rsp = (uint64_t)stack_ptr;
    p->state = Ready;
    p->last_change = gt_plat_get_time();

    return 0;
}

void gt_schedule(void) {
   struct gt *p = 0;
    struct gt_context *old_ctx, *new_ctx;
    double now = gt_plat_get_time();

    gt_plat_reset_timer();

    // --- KLÍČOVÁ OPRAVA: Vždy aktualizovat běžící vlákno ---
    double delta = now - gt_current->last_change;
    if (gt_current->state == Running) {
        gt_current->total_run_time += delta;
    }
    // Pozor: last_change musíme aktualizovat i pro čekající vlákna v cyklu níže,
    // nebo ji aktualizovat plošně pro všechna vlákna, aby Wait(s) odpovídal realitě.
    gt_current->last_change = now;

    // Výběr vlákna podle zvolené strategie
    switch (gt_strategy) {
        case 1: { // Priority Scheduling s prevencí hladovění
            // Prevence hladovění: "povýšíme" (snížíme číslo) priority čekajících vláken
            for (int i = 0; i < MaxGThreads; i++) {
                if (gt_table[i].state == Ready && gt_table[i].current_priority > 0) {
                    gt_table[i].current_priority--;
                }
            }

            int highest_prio = 11;
            for (int i = 0; i < MaxGThreads; i++) {
                if (gt_table[i].state == Ready) {
                    if (gt_table[i].current_priority < highest_prio) {
                        highest_prio = gt_table[i].current_priority;
                        p = &gt_table[i];
                    }
                }
            }
            // Pokud jsme vybrali vlákno, resetujeme mu jeho dynamickou prioritu
            if (p) p->current_priority = p->priority;
            break;
        }

        case 2: { // Lottery Scheduling
            int total_tickets = 0;
            for (int i = 0; i < MaxGThreads; i++) {
                if (gt_table[i].state == Ready) total_tickets += gt_table[i].tickets;
            }

            if (total_tickets > 0) {
                int winner = rand() % total_tickets;
                int count = 0;
                printf("Lottery: Total Tickets = %d, Winner = %d\n", total_tickets, winner);
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

        default: // Výchozí Round Robin
            p = gt_current;
            while (1) {
                if (++p == &gt_table[MaxGThreads]) p = &gt_table[0];
                if (p->state == Ready) break;
                if (p == gt_current) return; 
            }
            break;
    }

   if (p == gt_current && gt_current->state == Running) {
        return;
    }

    if (!p) return; 

    // Přepínáme kontext - update statistik příchozího vlákna
    p->total_wait_time += (now - p->last_change);
    p->last_change = now;
    p->switches++; // Zvýšit čítač přepnutí

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
    while(1); 
}