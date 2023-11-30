#include <sys/queue.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h> 
#include <pthread.h>
#include <string.h>
#include <stddef.h>
#include <fcntl.h>


#define TIMESIZE 20
#define LOG_SIZE 1000
#define INFO 0
#define WARN 1
#define ERROR 2
#define OFF 3

int log_shutdown_signal = 0;
char log_file_name = "logfile.txt"
pthread_t log_t;

typedef struct Logger{
    char *data;
    char *scope;
    int log_level;
    CIRCLEQ_ENTRY(Logger) next;
}logger_t;

CIRCLEQ_HEAD(logger_c, next);
struct logger_c logger_head;


int logger_init(char *path);
void logger_main_loop();
int log(char *scope, int level, ...) __attribute__((format(printf, 1, 2)));
int shutdown_logger();