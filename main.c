#include <main.h>

struct logger_c *logger_head = NULL;

char *construct_file_path(char *path, char **file_name){
    time_t t ;
    struct tm *tmp ;
    char curr_time[TIMESIZE];
    time(&t);
    strftime(curr_time, sizeof(curr_time), "%d-%m-%Y-%H-%M", localtime(&t));
    *file_name = (char *)malloc(strlen(path) + strlen(log_file_name) + strlen(curr_time) + 3); // +1 for the null-terminator
    sprintf(*file_name,"%s/%s-%s.txt",path,log_file_name,curr_time);
    printf("%s\n",*file_name);
    return 0;
}

int logger_init(char *path){
    struct stat s;
    int err = stat(path, &s);
    
    if(err == -1) {
        printf("wrong path");
        if(ENOENT == errno) {
            return 1;
        } else {
            perror("stat");
            return 1;
        }
    } 
    else {
        if(!S_ISDIR(s.st_mode)) {
            /* exists but is no dir */
            return 2;
        }
    }

    /* it's a dir */
    char *file_name;
    construct_file_path(path, &file_name);
    log_fd = open(file_name, O_CREAT |O_RDWR | O_APPEND);

    if(log_fd < 0){
        printf("errno is %d\n", errno );
        return 1;
    }

    logger_head = (struct logger_c *)malloc(sizeof(struct logger_c));
    STAILQ_INIT(logger_head);
    
    /* Creating new thread */
    if(pthread_create(&log_t, NULL, &logger_main_loop, NULL) != 0){
        perror("failed to created new thread");
        return 1;
    }

    return 0;
}

int logV1(int level, char *scope, char *msg, ...){
    if(level == OFF){
        return 0;
    }

    char *log_msg;
    va_list args;
    va_start(args, msg);

    if(vasprintf(&log_msg, level, args) < 0){
        log_msg = NULL;
    }

    va_end(args);

    // if(is_queue_full()){
    //     perror("logger queue full");
    //     return 1;
    // }

    struct logger *log_entry = (struct logger *)malloc(sizeof(struct logger));
    log_entry->data = log_msg;
    log_entry->log_level = level;
    log_entry->scope = scope;

    STAILQ_INSERT_TAIL(logger_head, log_entry, next);
    return 0;
}

int construct_log(struct logger *log_entry, char *msg){
    switch(log_entry->log_level){
        case INFO:
            strcpy(msg, "[INFO]");
            break;
        case WARN:
            strcpy(msg, "[WARN]");
            break;
        case ERROR:
            strcpy(msg, "[ERROR]");
            break;
        default:
            strcpy(msg, "[DEFAULT]");
            perror("wrong log_level");
    }
    int len = strlen(msg);
    printf("scope ");
    return sprintf(msg+len, " %s %s",log_entry->scope,log_entry->data);
}

void* logger_main_loop(){
    while(1){
        if(STAILQ_EMPTY(logger_head)){
            sleep(1);
        }
        else{
            struct logger *log_entry, *next_entry;
            log_entry = STAILQ_FIRST(logger_head);
            while(log_entry!= NULL){
                char msg[LOG_SIZE];
                int len = construct_log(log_entry,msg);
                printf("msg:%s",msg);
                write(log_fd, msg, len);
                next_entry = STAILQ_NEXT(log_entry, next);
                free(log_entry);
                log_entry = next_entry;
            }
        }
    }
}


void main(){
    logger_init(".");
    char msg[] = "testing";
    logV1(INFO,"hello","%s",msg);
    pthread_join(log_t, NULL);
}


