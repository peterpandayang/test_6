#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>


#define process_count 3
#define param_count 3
#define buf_size 64

struct value_st{
    int pipe_count;
    int process[process_count];
    char *argv1[param_count];
    char *argv2[param_count];
    char *argv3[param_count];
};


void parse_input(int argc, char* argv[], struct value_st *input){
    int i;
    for(i = 0; i < argc; i++){
        if(strcmp(argv[i], "|") == 0){
            input->pipe_count += 1;
        }
    }

    int fast = 1;
    int slow = 1;
    for(i = 0; i <= input->pipe_count && fast < argc; i++){
        int j = 0;
        while(fast < argc && strcmp(argv[fast], "|") != 0){
            if(i == 0){
                input->argv1[j] = argv[fast];   
            }
            if(i == 1){
                input->argv2[j] = argv[fast];   
            }
            if(i == 2){
                input->argv3[j] = argv[fast];   
            }
            j += 1;
            fast += 1;
        }
        input->process[i] = fast - slow;
        fast += 1;
        slow = fast;
    }
}

int exec_process_1(struct value_st *input){
    char *root = "/usr/bin/";
    char *cmd = input->argv1[0];
    char *path = malloc(strlen(root) + strlen(cmd) + 1);
    strcpy(path, root);
    strcat(path, cmd);

    if(input->process[0] - 1 == 1){
        char *args[3];
        args[0] = cmd;
        args[1] = input->argv1[1];
        args[2] = (char *)NULL;
        execvp(path, args);
    }
    else if(input->process[0] - 1 == 2){
        char *args[4];
        args[0] = cmd;
        args[1] = input->argv1[1];
        args[2] = input->argv1[2];
        args[3] = (char *)NULL;
        execvp(path, args);
    }
}

int exec_process_2(struct value_st *input){
    char *root = "/usr/bin/";
    char *cmd = input->argv2[0];
    char *path = malloc(strlen(root) + strlen(cmd) + 1);
    strcpy(path, root);
    strcat(path, cmd);

    if(input->process[1] - 1 == 1){
        char *args[3];
        args[0] = cmd;
        args[1] = input->argv2[1];
        args[2] = (char *)NULL;
        execvp(path, args);
    }
    else if(input->process[1] - 1 == 2){
        char *args[4];
        args[0] = cmd;
        args[1] = input->argv2[1];
        args[2] = input->argv2[2];
        args[3] = (char *)NULL;
        execvp(path, args);
    }
}

int count_lines(char* read_buf, int len){
    int i = 0;
    int counter = 0;
    while(i < len - 1){
        if(read_buf[i] == '\n'){
            counter += 1;
        }
        i++;
    }
    return counter + 1;
}

// refer: http://stackoverflow.com/questions/190229/where-is-the-itoa-function-in-linux
char *itoa(int n){
    int lab = labs(n);
    int log_result = 0;
    while(lab / 10 > 0){
        log_result += 1;
        lab /= 10;
    }
    int len = n==0 ? 1 : log_result + 1;
    if (n < 0) len++; // room for negative sign '-'

    char *buf = calloc(sizeof(char), len + 1); // +1 for null
    snprintf(buf, len + 1, "%d", n);
    return buf;
}

int check_asc(char* read_buf){
    int size = strlen(read_buf);
    int i = 0;
    int result = 1;
    for(i = 0; i < size; i++){
        if(read_buf[i] == '\0'){
            break;
        }
        if(read_buf[i] - '0' > 127){
            // printf("char: %d\n", read_buf[i] - '0');
            // printf("char: %c\n", read_buf[i]);
            result = 0;
        }
    }
    return result;
}

int write_to_file(struct value_st *input, char* read_buf, int index, char* cmd){
    int bytes, lines;
    char* bytes_str;
    char* lines_str;
    char* index_str;
    char write_buf[1024];
    int is_asc = 0;

    bytes = strlen(read_buf);
    bytes_str = itoa(bytes);
    lines = count_lines(read_buf, bytes);
    lines_str = itoa(lines);
    is_asc = check_asc(read_buf);
    index_str = itoa(index);

    strcat(write_buf, "[");
    strcat(write_buf, index_str);
    strcat(write_buf, "] ");
    strcat(write_buf, input->argv1[0]);
    strcat(write_buf, " -> ");
    if(cmd){
        strcat(write_buf, cmd);
    }
    strcat(write_buf, "\n");
    strcat(write_buf, bytes_str);
    strcat(write_buf, " bytes");
    strcat(write_buf, "\n");
    strcat(write_buf, lines_str);
    strcat(write_buf, " lines");
    strcat(write_buf, "\n");  
    if(is_asc == 1){
        strcat(write_buf, "ASCII data");  
    }
    else{
        strcat(write_buf, "BINARY data");  
    }

    int file_fd = open("pa.log", O_CREAT | O_WRONLY | O_APPEND);
    if (file_fd < 0) {
        write(2, "file_fd is less than 0\n", 24);
        exit(-1);
    }
    if (write(file_fd, write_buf, strlen(write_buf)) < 0) {
        write(2, "There was an error writing to pa.log.txt\n", 43);
        exit(-1);
    }

    return 0;
}

int do_without_pipe(struct value_st *input){
    pid_t id;
    int pipe1[2];
    char read_buf[64];

    pipe(pipe1);
    id = fork();

    if (id < 0) {
        printf("fork() for first process failed\n");
        exit(-1);
    }
    if (id == 0) {
        close(pipe1[0]);
        close(pipe1[1]);
        if (exec_process_1(input) < 0) {
            write(2, "execlp() failed for prog1\n", 27);
            exit(-1);
        }
    }
    close(pipe1[0]);
    close(pipe1[1]);
    
    id = wait(NULL);
    return 0;
}

int do_with_one_pipe(struct value_st *input){
    pid_t id;
    int pipe_1_m1[2];
    int pipe_m1_p[2];
    int pipe_m1_2[2];
    char read_buf[64];
    char read_buf_c2[64];

    pipe(pipe_1_m1);
    pipe(pipe_m1_p);
    pipe(pipe_m1_2);

    // entering child 1
    id = fork();
    if (id < 0) {
        printf("fork() for first process failed\n");
        exit(-1);
    }
    if (id == 0) {
        close(pipe_m1_2[0]);
        close(pipe_m1_2[1]);
        close(pipe_m1_p[0]);
        close(pipe_m1_p[1]);

        close(pipe_1_m1[0]);
        close(1);
        dup(pipe_1_m1[1]);
        close(pipe_1_m1[1]);
        if (exec_process_1(input) < 0) {
            write(2, "execlp() failed for prog1\n", 27);
            exit(-1);
        }
    }

    // entering m1
    id = fork();
    if (id < 0) {
        printf("fork() for second process failed\n");
        exit(-1);
    }
    if (id == 0) {
        close(pipe_m1_p[0]);
        close(pipe_m1_p[1]);

        close(pipe_1_m1[1]);
        close(0);
        dup(pipe_1_m1[0]);
        if(read(0, read_buf, 64) < 0) {
            write(2, "cannot read from pipe\n", 23);
            exit(-1);
        }

        close(1);
        dup(pipe_m1_p[1]);
        close(pipe_m1_p[1]);

        if (write(1, read_buf, strlen(read_buf)) < 0) {
           write(2, "cannot write to pipe\n", 21);
        }

        close(1);
        dup(pipe_m1_2[1]);
        close(pipe_m1_2[1]);

        if (write(1, read_buf, strlen(read_buf)) < 0) {
           write(2, "cannot write to pipe\n", 21);
        }

        exit(0);
    }

    // entering Child 2
    id = fork();
    if (id < 0) {
        printf("fork() for second process failed\n");
        exit(-1);
    }
    if (id == 0) {
        close(pipe_m1_p[0]);
        close(pipe_m1_p[1]);
        
        sleep(1);
        close(pipe_m1_2[1]);
        close(0);
        dup(pipe_m1_2[0]);
        close(pipe_m1_2[0]);

        if (exec_process_2(input) < 0) {
            write(2, "execlp() failed for prog1\n", 27);
            exit(-1);
        }
        exit(0);
    }

    close(pipe_1_m1[0]);
    close(pipe_1_m1[1]);
    close(pipe_m1_2[0]);
    close(pipe_m1_2[1]);

    id = wait(NULL);
    id = wait(NULL);
    id = wait(NULL);

    close(pipe_m1_p[1]);
    close(0);
    dup(pipe_m1_p[0]);
    close(pipe_m1_p[0]);

    if(read(0, read_buf, 64) < 0) {
        write(2, "cannot read from pipe\n", 23);
        exit(-1);
    }
    write_to_file(input, &read_buf, 1, input->argv2[0]);

    return 0;
}

int exec(struct value_st *input){
    printf("pipe count is: %d\n", input->pipe_count);
    if(input->pipe_count == 0){
        do_without_pipe(input);
    }
    else if(input->pipe_count == 1){
        do_with_one_pipe(input);
    }
    else if(input->pipe_count >= 2){
        // do_pipeline_2(input);
        printf("does not support more than 2 pipes\n");
    }

    return 0;
}

int main(int argc, char* argv[]) {

    struct value_st input;
    input.pipe_count = 0;

    // support up to 1 pipelines and up to 2 parameters
    parse_input(argc, argv, &input);

    // execute the program
    exec(&input);

}

