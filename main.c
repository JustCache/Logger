#include <main.h>

char *construct_file_path(char *path){
    time_t t ;
    struct tm *tmp ;
    char curr_time[TIMESIZE];
    time(&t);
    strftime(curr_time, sizeof(curr_time), "%d-%m-%Y-%H-%M", localtime(&t));
    char *file_name = (char *)malloc(strlen(path) + strlen(log_file_name) + strlen(curr_time) + 3) // +1 for the null-terminator
    strcpy(file_name, path);
    strcat(file_name, "/");
    strcat(file_name, log_file_name);
    strcat(file_name, "-");
    strcat(file_name, curr_time);
    strcat(file_name, "\n");
    return file_name;
}

int logger_init(char *path){
    struct stat s;
    int err = stat(*(path), &s);
    
    if(err == -1) {
        if(ENOENT == errno) {
            return 1;
        } else {
            perror("stat");
            return 1;
        }
    } else {
        if(!S_ISDIR(s.st_mode)) {
            /* exists but is no dir */
            return 2;
    }

    /* it's a dir */
    char *file_name = construct_file_path(path);
    lod_fd = open(file_name, O_CREAT |O_RDWR | O_APPEND);

    if(lod_fd < 0){
        return 1;
    }

    CIRCLEQ_INIT(&logger_head);
    
    /* Creating new thread */
    if(pthread_create(&log_t, NULL, &logger_main_loop, NULL) != 0){
        perror("failed to created new thread");
        return 1;
    }

    return 0;
}

int log(char *scope, int level, ...){
    if(level == OFF){
        return 0;
    }

    char *log_msg;
    va_list args;
    va_start(args, level);

    if(vasprintf(&log_msg, level, args) < 0){
        log_msg = NULL;
    }

    va_end(args);

    if(is_queue_full()){
        perror("logger queue full");
        return 1;
    }

    logger_t log_entry;
    log_entry.data = log_msg;
    log_entry.level = level;
    log_entry.scope = scope;

    CIRCLEQ_INSERT_HEAD(&logger_head, log_entry, next);
    return 0;
}

char *construct_log(logger_t *log_entry){
    char msg[LOG_SIZE];
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
            strcpy(msg, "[DEFAULT]")
            perror("wrong log_level");
    }
    strcat(msg, " ");
    strcat(msg, log_entry->scope);
    strcat(msg, " ");
    strcat(msg, log_entry->data);
    strcat(msg, "\n");
    return msg;
}

void logger_main_loop(){
    while(1){
        if(CIRCLEQ_EMPTY()){
            sleep(1);
        }
        else{
            logger_t *log_entry;
            CIRCLEQ_FOREACH(log_entry, &logger_head, next){
                char *msg = construct_log(log_entry);
                write(lod_fd, msg, sizeof(*msg));
            }
        }

    }
}


