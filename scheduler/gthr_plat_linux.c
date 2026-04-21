#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "gthr.h"

double gt_plat_get_time(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
}

void gt_plat_init_timer(void) {
    signal(SIGALRM, (void (*)(int))gt_schedule);
    signal(SIGINT, gt_plat_print_stats); 
    ualarm(500, 500);
}

void gt_plat_reset_timer(void) {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    sigprocmask(SIG_UNBLOCK, &set, NULL);
    ualarm(500, 500);
}

int gt_uninterruptible_nanosleep(time_t sec, long nanosec) {
    struct timespec req;
    req.tv_sec = sec;
    req.tv_nsec = nanosec;
    do {
        if (0 != nanosleep(&req, &req)) {
            if (errno != EINTR) return -1;
        } else break;
    } while (req.tv_sec > 0 || req.tv_nsec > 0);
    return 0;
}

void gt_plat_print_stats(int sig) {
    printf("\n--- THREAD STATISTICS ---\n");
    printf("ID\tState\tRunTime\tWaitTime\tAvgRun\tMaxRun\tVariance\n");
    for (int i = 0; i < MaxGThreads; i++) {
        struct gt *t = &gt_table[i];
        if (t->state == Unused && t->switches == 0) continue;
        double avg = (t->switches > 0) ? (t->total_run_time / t->switches) : 0;
        double var = (t->switches > 0) ? (t->sum_sq_run / t->switches) - (avg * avg) : 0;
        printf("%d\t%d\t%.4fs\t%.4fs\t%.4f\t%.4f\t%.6f\n", 
               i, t->state, t->total_run_time, t->total_wait_time, avg, t->max_run, var);
    }
    exit(0);
}