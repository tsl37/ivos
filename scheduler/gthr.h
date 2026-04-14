enum {
	MaxGThreads = 5,            // Maximum number of threads, used as array size for gttbl
	StackSize = 0x400000,       // Size of stack of each thread
};

struct gt {
    struct gt_context {
        uint64_t rsp, r15, r14, r13, r12, rbx, rbp;
    } ctx;
    enum { Unused, Running, Ready } state;

    // --- Nové položky pro statistiku ---
    struct timeval last_change; // Čas poslední změny stavu
    double total_run_time;      // Celková doba v Running (sekundy)
    double total_wait_time;     // Celková doba v Ready (sekundy)
    
    // Pro výpočet průměru a rozptylu délky jednoho "kvanta" (Running)
    int switches;               // Počet naplánování vlákna
    double min_run;             // Minimální doba jednoho běhu
    double max_run;             // Maximální doba jednoho běhu
    double sum_sq_run;          // Součet čtverců dob běhu (pro rozptyl)
};


void gt_init(void);                                                     // initialize gttbl
void gt_return(int ret);                                                // terminate thread
void gt_switch(struct gt_context * old, struct gt_context * new);       // declaration from gtswtch.S
bool gt_schedule(void);                                                 // yield and switch to another thread
void gt_stop(void);                                                     // terminate current thread
int gt_create(void( * f)(void));                                        // create new thread and set f as new "run" function
void gt_reset_sig(int sig);                                             // resets signal
void gt_alarm_handle(int sig);                                          // periodically triggered by alarm
int gt_uninterruptible_nanosleep(time_t sec, long nanosec);             // uninterruptible sleep
