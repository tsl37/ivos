#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include "gthr.h"

/**
 * Vypíše statistiky o běhu jednotlivých vláken včetně lístků (tickets).
 * Volá se automaticky při stisknutí CTRL+C (SIGINT).
 */
void gt_plat_print_stats(int sig) {
    printf("\n--- GThread Statistics ---\n");
    // Přidán sloupec "Tickets" do záhlaví
    printf("%-3s | %-8s | %-10s | %-10s | %-8s | %-5s | %-5s\n", 
           "ID", "State", "Runtime(s)", "Wait(s)", "Switches", "Prio", "Tix");
    printf("----------------------------------------------------------------------------\n");

    for (int i = 0; i < MaxGThreads; i++) {
        const char* state_str = "Unknown";
        if (gt_table[i].state == Unused) state_str = "Unused";
        else if (gt_table[i].state == Running) state_str = "Running";
        else if (gt_table[i].state == Ready) state_str = "Ready";

        // Vytištění dat včetně gt_table[i].tickets
        printf("%-3d | %-8s | %-10.6f | %-10.6f | %-8d | %-5d | %-5d\n",
               i, 
               state_str,
               gt_table[i].total_run_time,
               gt_table[i].total_wait_time,
               gt_table[i].switches,
               gt_table[i].priority,
               gt_table[i].tickets); // Nový sloupec pro lístky
    }
    exit(0);
}

// --- Zbytek souboru zůstává stejný ---

double gt_plat_get_time(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

static void timer_handler(int sig) {
    gt_schedule();
}

void gt_plat_init_timer(void) {
    struct sigaction sa;
    sa.sa_handler = &timer_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGALRM, &sa, NULL);
    sa.sa_handler = &gt_plat_print_stats;
    sigaction(SIGINT, &sa, NULL);
    gt_plat_reset_timer();
}

void gt_plat_reset_timer(void) {
    struct itimerval it;
    it.it_interval.tv_sec = 0;
    it.it_interval.tv_usec = 100000;
    it.it_value.tv_sec = 0;
    it.it_value.tv_usec = 100000;
    setitimer(ITIMER_REAL, &it, NULL);
}

void gt_uninterruptible_nanosleep(long sec, long nanosec) {
    struct timespec req, rem;
    req.tv_sec = sec;
    req.tv_nsec = nanosec;
    while (nanosleep(&req, &rem) == -1) {
        req = rem;
    }
}