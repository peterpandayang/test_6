#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


#define process_count 2
#define param_count 16
#define buf_size 64

struct value_st{
    int pipe_count;
    int process[process_count];
    char *argv1[param_count];
    char *argv2[param_count];
};


void parse_input(int argc, char* argv[], struct value_st *input){
    int i;
    for(i = 0; i < argc; i++){
        if(strcmp(argv[i], "|") == 0){
            input->pipe_count += 1;
        }
    }
    if(input->pipe_count >= 2){
        printf("does not support more than 2 pipes\n");
        exit(0);
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
    int arg_add = input->process[0] + 1;
    char *args[arg_add];
    args[0] = cmd;
    args[arg_add - 1] = (char *)NULL;
    int i = 1;
    for(i = 1 ; i < arg_add - 1; i++){
        args[1] = input->argv1[i];
    }
    execvp(path, args);

    // if(input->process[0] - 1 == 1){
    //     char *args[3];
    //     args[0] = cmd;
    //     args[1] = input->argv1[1];
    //     args[2] = (char *)NULL;
    //     execvp(path, args);
    // }
    // else if(input->process[0] - 1 == 2){
    //     char *args[4];
    //     args[0] = cmd;
    //     args[1] = input->argv1[1];
    //     args[2] = input->argv1[2];
    //     args[3] = (char *)NULL;
    //     execvp(path, args);
    // }
}

int exec_process_2(struct value_st *input){
    char *root = "/usr/bin/";
    char *cmd = input->argv2[0];
    char *path = malloc(strlen(root) + strlen(cmd) + 1);
    strcpy(path, root);
    strcat(path, cmd);

    int arg_add = input->process[1] + 1;
    char *args[arg_add];
    args[0] = cmd;
    args[arg_add - 1] = (char *)NULL;
    int i = 1;
    for(i = 1 ; i < arg_add - 1; i++){
        args[1] = input->argv2[i];
    }
    execvp(path, args);

    // if(input->process[1] - 1 == 1){
    //     char *args[3];
    //     args[0] = cmd;
    //     args[1] = input->argv2[1];
    //     args[2] = (char *)NULL;
    //     execvp(path, args);
    // }
    // else if(input->process[1] - 1 == 2){
    //     char *args[4];
    //     args[0] = cmd;
    //     args[1] = input->argv2[1];
    //     args[2] = input->argv2[2];
    //     args[3] = (char *)NULL;
    //     execvp(path, args);
    // }
}

int do_without_pipe(struct value_st *input){
    pid_t id;
    int pipe1[2];
    char read_buf[buf_size];
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
    char read_buf[buf_size];
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
        close(pipe_1_m1[0]);
        close(1);
        dup(pipe_1_m1[1]);
        close(pipe_1_m1[1]);
        if (exec_process_1(input) < 0) {
            write(2, "execlp() failed for prog1\n", 27);
            exit(-1);
        }
        exit(0);
    }
    // entering m1
    id = fork();
    if (id < 0) {
        printf("fork() for second process failed\n");
        exit(-1);
    }
    if (id == 0) {
        close(pipe_1_m1[1]);
        close(0);
        dup(pipe_1_m1[0]);
        memset(read_buf, 0, buf_size);
        if(read(0, read_buf, buf_size) < 0) {
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
    memset(read_buf, 0, buf_size);
    char buf[32];
    int i = 0;
    int bytes = 0;
    int lines = 0;
    int is_asc = 1;
    while(read(0, &buf[i], 1) > 0) {
        bytes += 1;
        if(buf[i] == '\n'){
            lines += 1;
        }
        if(buf[i] - '0' > 127){
            is_asc = 0;
        }
        i += 1;
    }
    FILE *f = fopen("pa.log", "w");

    if (f == NULL){
        printf("Error opening file!\n");
        exit(1);
    }
    fprintf(f, "[1] %s -> %s\n", input->argv1[0], input->argv2[0]);
    fprintf(f, "%d bytes\n", bytes);
    fprintf(f, "%d lines\n", lines);
    if(is_asc == 1){
        fprintf(f, "ASCII data\n");
    }
    else{
        fprintf(f, "Binary data\n");
    }
    fclose(f);
    return 0;
}

int exec(struct value_st *input){
    if(input->pipe_count == 0){
        do_without_pipe(input);
    }
    else if(input->pipe_count == 1){
        do_with_one_pipe(input);
    }
    else if(input->pipe_count >= 2){
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

