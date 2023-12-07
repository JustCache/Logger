#include <main.h>

struct logger_c *logger_head = NULL;

char *construct_file_path(char *path, char **file_name){
    time_t t ;
    struct tm *tmp ;
    char curr_time[TIMESIZE];
    time(&t);
    strftime(curr_time, sizeof(curr_time), "%d-%m-%Y-%H-%M", localtime(&t));
    size_t file_name_len = strlen(path) + strlen(log_file_name) + strlen(curr_time) + 7; // +1 for the null-terminator +2 for '/' +4 for .txt
    *file_name = (char *)malloc(file_name_len); 
    snprintf(*file_name, file_name_len, "%s/%s-%s.txt", path, log_file_name, curr_time);
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
    log_fd = open(file_name, O_CREAT | O_RDWR | O_APPEND);
    free(file_name);
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

    if(vasprintf(&log_msg, msg, args) < 0){
        log_msg = NULL;
    }
    va_end(args);
    char *log_scope = (char *)malloc(10);
    strcpy(log_scope,scope);
    struct logger *log_entry = (struct logger *)malloc(sizeof(struct logger));
    log_entry->data = log_msg;  
    log_entry->log_level = level;
    log_entry->scope = log_scope;

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
    int len = strlen(log_entry->scope) + strlen(log_entry->data) + 4; // +2 for spaces +1 for the null-terminator +1 for \n
    size_t log_len = strlen(msg);
    if(len+log_len>LOG_SIZE){
        perror("log size exceded");
        len = LOG_SIZE-log_len;
    }
    log_len += snprintf(msg+strlen(msg), len, " %s %s\n", log_entry->scope, log_entry->data);
    return log_len;
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
                char *msg = (char *)malloc(LOG_SIZE);
                int len = construct_log(log_entry,msg);
                write(log_fd, msg, len);
                STAILQ_REMOVE_HEAD(logger_head, next);
                next_entry = STAILQ_FIRST(logger_head);
                free(log_entry->data);
                free(log_entry->scope);
                free(log_entry);
                free(msg);
                log_entry = next_entry;
            }
        }
        if(log_shutdown_signal == 2){
            free(logger_head);
            break;
        }
        if(log_shutdown_signal == 1){
            log_shutdown_signal = 2;
        } 
    }
}



