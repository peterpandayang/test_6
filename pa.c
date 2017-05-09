#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>


#define process_count 3
#define param_count 3

// #ifndef max
//     #define max(x, y) ((x) > (y) ? (x) : (y))
// #endif

// int main(int argc, char* argv[]) {
//    pid_t pid1, pid2;
//    int pipefd[2];
//    // The two commands we'll execute.  In this simple example, we will pipe
//    // the output of `ls` into `wc`, and count the number of lines present.
//    char *argv1[] = {"ls", "-l", "-h", NULL};
//    char *argv2[] = {"wc", "-l", NULL};
//    // Create a pipe.
//    pipe(pipefd);
//    // Create our first process.
//    pid1 = fork();
//    if (pid1 == 0) {
//       // Hook stdout up to the write end of the pipe and close the read end of
//       // the pipe which is no longer needed by this process.
//       dup2(pipefd[1], STDOUT_FILENO);
//       close(pipefd[0]);
//       // Exec `ls -l -h`.  If the exec fails, notify the user and exit.  Note
//       // that the execvp variant first searches the $PATH before calling execve.
//       execvp(argv1[0], argv1);
//       perror("exec");
//       return 1;
//    }
//    // Create our second process.
//    pid2 = fork();
//    if (pid2 == 0) {
//       // Hook stdin up to the read end of the pipe and close the write end of
//       // the pipe which is no longer needed by this process.
//       dup2(pipefd[0], STDIN_FILENO);
//       close(pipefd[1]);
//       // Similarly, exec `wc -l`.
//       execvp(argv2[0], argv2);
//       perror("exec");
//       return 1;
//    }
//    // Close both ends of the pipe.  The respective read/write ends of the pipe
//    // persist in the two processes created above (and happen to be tying stdout
//    // of the first processes to stdin of the second).
//    close(pipefd[0]);
//    close(pipefd[1]);
//    // Wait for everything to finish and exit.
//    waitpid(pid1);
//    waitpid(pid2);
//    return 0;
// }

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

int do_pipeline_0(struct value_st *input){
    char *root = "/usr/bin/";
    char *cmd = input->argv1[0];
    char *path = malloc(strlen(root) + strlen(cmd) + 1);
    strcpy(path, root);
    strcat(path, cmd);
    if(input->process[0] - 1 == 1){
       execl(path, cmd, input->argv1[1], NULL);
    }
    else if(input->process[0] - 1 == 2){
        execl(path, cmd, input->argv1[1], input->argv1[2], (char *)NULL);
    }
    return 0;
}

int do_pipeline_1(struct value_st *input){
    pid_t id;
    int count, nbytes;
    char buf[100];
    int pipe1[2];
    int pipe2[2];
    pipe(pipe1);
    pipe(pipe2);
    id = fork();
    char *root = "/usr/bin/";

    if (id == 0) {
        /* we are in the child */
        sleep(1);
        close(pipe1[0]);
        close(1);
        dup(pipe1[1]);
        char *cmd = input->argv1[0];
        char *path = malloc(strlen(root) + strlen(cmd) + 1);
        strcpy(path, root);
        strcat(path, cmd);
        close(pipe1[0]);
        close(1);
        dup(pipe1[1]);
        if(input->process[0] - 1 == 1){
            execl(path, cmd, input->argv1[1], NULL);
        }
        else if(input->process[0] - 1 == 2){
            execl(path, cmd, input->argv1[1], input->argv1[2], (char *)NULL);
        }
        
    } else {
       /* we are in the parent */
        close(pipe1[1]);
        close(0);
        dup(pipe1[0]);
        id = wait(NULL);
        if (read(0, buf, 100) < 0) {
            fprintf(stderr, "cannot read from pipe\n");
        }
        if (write(pipe2[1], buf, strlen(buf)) < 0) {
           fprintf(stderr, "cannot write to pipe\n");
        }
        close(pipe2[1]);
        close(0);
        dup(pipe2[0]);
        close(pipe2[0]);
        printf("buf is: %s\n", buf);
        char *cmd = input->argv2[0];
        char *path = malloc(strlen(root) + strlen(cmd) + 1);
        strcpy(path, root);
        strcat(path, cmd);
        // printf("param num: %d\n", input->process[1]);
        printf("cmd is: %s\n", cmd);
        if(input->process[1] - 1 == 1){
            execl(path, cmd, input->argv2[1], NULL);
        }
        else if(input->process[1] - 1 == 2){
            execl(path, cmd, input->argv2[1], input->argv2[2], (char *)NULL);
        }
        // execl("/usr/bin/wc", "wc", "-l", NULL);
    }

    return 0;
}   

int exec(struct value_st *input){
    if(input->pipe_count == 0){
        do_pipeline_0(input);
    }
    else if(input->pipe_count == 1){
        do_pipeline_1(input);
    }

}


int main(int argc, char* argv[]) {

    struct value_st input;
    input.pipe_count = 0;

    // restrained up to 2 pipelines
    parse_input(argc, argv, &input);

    // execute the program
    exec(&input);

}






