// Based on https://c9x.me/articles/gthreads/code0.html
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>

#include "gthr.h"

// Pomocná funkce pro simulaci práce vlákna
void f(void) {
    static int x = 0;
    int i = 0, id;

    id = ++x;
    while (true) {
        // Výpis obsahuje ID vlákna a jeho základní prioritu pro kontrolu v logu
        printf("F Thread id = %d (Prio: %d), val = %d BEGINNING\n", id, gt_current->priority, ++i);
        gt_uninterruptible_nanosleep(0, 50000000);
        printf("F Thread id = %d, val = %d END\n", id, ++i);
        gt_uninterruptible_nanosleep(0, 50000000);
    }
}

// Druhá pomocná funkce pro simulaci práce vlákna
void g(void) {
    static int x = 0;
    int i = 0, id;

    id = ++x;
    while (true) {
        printf("G Thread id = %d (Prio: %d), val = %d BEGINNING\n", id, gt_current->priority, ++i);
        gt_uninterruptible_nanosleep(0, 50000000);
        printf("G Thread id = %d, val = %d END\n", id, ++i);
        gt_uninterruptible_nanosleep(0, 50000000);
    }
}

int main(int argc, char **argv) {
    // 1. Zpracování argumentů pro výběr strategie
    if (argc > 1) {
        if (strcmp(argv[1], "RR") == 0) {
            gt_strategy = 0;
            printf("Strategie: Round Robin\n");
        } else if (strcmp(argv[1], "PRI") == 0) {
            gt_strategy = 1;
            printf("Strategie: Priority Scheduling (s prevenci hladoveni)\n");
        } else if (strcmp(argv[1], "LS") == 0) {
            gt_strategy = 2;
            printf("Strategie: Lottery Scheduling\n");
        } else {
            printf("Pouziti: %s [RR|PRI|LS]\n", argv[0]);
            return 1;
        }
    } else {
        printf("Nebyl vybran planovac, pouzivam vychozi: Round Robin\n");
        gt_strategy = 0;
    }

    // 2. Inicializace generátoru náhodných čísel (pro Lottery Scheduling)
    srand(time(NULL));

    // 3. Inicializace systému vláken
    gt_init();            

    // 4. Vytvoření testovacích vláken s různými prioritami
    // Parametry: (funkce, priorita 0-10)
    // Poznámka: U RR je priorita ignorována, u PRI určuje důležitost, u LS určuje počet lístků.
    gt_create(f, 0);   // Nejvyšší priorita
    gt_create(f, 3);   // Vysoká priorita
    gt_create(g, 5);   // Střední priorita
    gt_create(g, 8);   // Nízká priorita
    gt_create(g, 10);  // Nejnižší priorita
    
    // 5. Hlavní smyčka (idle thread)
    // Plánovač bude díky signálu SIGALRM v gthr_plat_linux.c přepínat vlákna automaticky.
    while(1) {
        gt_yield(); 
    }

    return 0;
}