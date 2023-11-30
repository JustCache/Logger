#include <sys/queue.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h> 
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stddef.h>
#include <fcntl.h>
#include <stdarg.h>


#define TIMESIZE 20
#define LOG_SIZE 1000
#define INFO 0
#define WARN 1
#define ERROR 2
#define OFF 3

int log_shutdown_signal = 0;
int log_fd;
char log_file_name[] = "logfile";
pthread_t log_t;

struct logger{
    char *data;
    char *scope;
    int log_level;
    STAILQ_ENTRY(logger) next;
};

STAILQ_HEAD(logger_c, logger);
extern struct logger_c *logger_head;


int logger_init(char *);
void* logger_main_loop();
int logV1(int ,char *,char *, ...)  __attribute__((format(printf, 3, 4)));
int shutdown_logger();