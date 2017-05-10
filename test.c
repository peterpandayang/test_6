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

int do_without_pipeline(struct value_st *input){
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

// int do_pipeline_1(struct value_st *input){
//     pid_t id;
//     int count, nbytes;
//     char buf[100];
//     int pipe1[2];
//     int pipe2[2];
//     pipe(pipe1);
//     pipe(pipe2);
//     id = fork();
//     char *root = "/usr/bin/";

//     if (id == 0) {
//         /* we are in the child */
//         sleep(1);
//         close(pipe1[0]);
//         close(1);
//         dup(pipe1[1]);
//         char *cmd = input->argv1[0];
//         char *path = malloc(strlen(root) + strlen(cmd) + 1);
//         strcpy(path, root);
//         strcat(path, cmd);
//         close(pipe1[0]);
//         close(1);
//         dup(pipe1[1]);
//         if(input->process[0] - 1 == 1){
//             execl(path, cmd, input->argv1[1], NULL);
//         }
//         else if(input->process[0] - 1 == 2){
//             execl(path, cmd, input->argv1[1], input->argv1[2], (char *)NULL);
//         }
        
//     } else {
//        /* we are in the parent */
//         close(pipe1[1]);
//         close(0);
//         dup(pipe1[0]);
//         id = wait(NULL);
//         if (read(0, buf, 100) < 0) {
//             fprintf(stderr, "cannot read from pipe\n");
//         }
//         if (write(pipe2[1], buf, strlen(buf)) < 0) {
//            fprintf(stderr, "cannot write to pipe\n");
//         }
//         close(pipe2[1]);
//         close(0);
//         dup(pipe2[0]);
//         close(pipe2[0]);
//         printf("buf is: %s\n", buf);
//         char *cmd = input->argv2[0];
//         char *path = malloc(strlen(root) + strlen(cmd) + 1);
//         strcpy(path, root);
//         strcat(path, cmd);
//         // printf("param num: %d\n", input->process[1]);
//         // printf("cmd is: %s\n", cmd);
//         if(input->process[1] - 1 == 1){
//             execl(path, cmd, input->argv2[1], NULL);
//         }
//         else if(input->process[1] - 1 == 2){
//             execl(path, cmd, input->argv2[1], input->argv2[2], (char *)NULL);
//         }
//         // execl("/usr/bin/wc", "wc", "-l", NULL);
//     }

//     return 0;
// }   

int do_with_pipeline(struct value_st *input){
    pid_t id;
    //int count, nbytes;
    char buf[100];
    int pipe1[2];
    int pipe2[2];
    // pipe(pipe1);
    // pipe(pipe2);
    char *root = "/usr/bin/";

    int i;
    
    for(i = 0; i < 2; i++){
        id = fork();
        pipe(pipe1);
        pipe(pipe2);
        if(id != 0){
            printf("ParentI: %d\n", i);
            printf("Parent id: %d\n", getpid());
            // write first
            // close(pipe2[0]);
            // close(1);
            // dup(pipe2[1]);
            // if (write(pipe2[1], buf, strlen(buf)) < 0) {
            //     fprintf(stderr, "cannot write to pipe\n");
            // }

            // read first
            // close(pipe1[1]);
            // close(0);
            // dup(pipe1[0]);
            // if (read(0, buf, strlen(buf)) < 0) {
            //     fprintf(stderr, "cannot read from pipe\n");
            // }
            // write 
            // close(pipe2[0]);

            // write first
            // if (read(0, buf, strlen(buf)) < 0) {
            //     fprintf(stderr, "cannot read from pipe\n");
            // }
            // write("buf is: %s\n", buf);
            // close(1);
            // dup(pipe2[1]);
            // if (write(pipe2[1], buf, strlen(buf)) < 0) {
            //     perror("write error");
            // }

            wait(NULL);

            // read
            printf("reading from child...\n");
            // printf("close stdin for process: %d\n", getpid());
            close(0);
            dup(pipe1[0]);
            if (read(0, buf, strlen(buf)) < 0) {
                perror("cannot read");
            }

            close(pipe1[0]);
            close(pipe1[1]);
            close(pipe2[0]);
            close(pipe2[1]);


            // write 


            // close(pipe1[1]);
            // close(0);
            // dup(pipe1[0]);

            // if (read(0, buf, 100) < 0) {
            //     fprintf(stderr, "cannot read from pipe\n");
            // }


            // printf("buf is: %s\n", buf);
            // if (write(pipe2[1], buf, strlen(buf)) < 0) {
            //     fprintf(stderr, "cannot write to pipe\n");
            // }
        }
        else{
            sleep(1);
            close(pipe2[1]);
            printf("close stdin for process: %d\n", getpid());
            close(0);
            dup(pipe2[0]);
            if(i == 0){
                printf("ChildI: %d\n", i);
                char *cmd = input->argv1[0];
                char *path = malloc(strlen(root) + strlen(cmd) + 1);
                strcpy(path, root);
                strcat(path, cmd);
                close(pipe1[0]);
                printf("close stdout for process: %d\n", getpid());
                close(1);
                dup(pipe1[1]);
                if (read(0, buf, 100) < 0) {
                    fprintf(stderr, "cannot read from pipe\n");
                }
                if(input->process[0] - 1 == 1){
                    if(execl(path, cmd, input->argv1[1], NULL) < 0){
                        perror("execl1");
                    }
                }
                else if(input->process[0] - 1 == 2){
                    if(execl(path, cmd, input->argv1[1], input->argv1[2], (char *)NULL) < 0){
                        perror("execl2");
                    }
                }
            }
            if(i == 1){
                char *cmd = input->argv2[0];
                char *path = malloc(strlen(root) + strlen(cmd) + 1);
                strcpy(path, root);
                strcat(path, cmd);
                // printf("param num: %d\n", input->process[1]);
                // printf("cmd is: %s\n", cmd);
                close(pipe1[0]);
                printf("close stdout for process: %d\n", getpid());
                close(1);
                dup(pipe1[1]);
                if (read(0, buf, 100) < 0) {
                    fprintf(stderr, "cannot read from pipe\n");
                }
                if(input->process[1] - 1 == 1){
                    if(execl(path, cmd, input->argv2[1], NULL) < 0){
                        perror("execl3");
                    }
                }
                else if(input->process[1] - 1 == 2){
                    if( execl(path, cmd, input->argv2[1], input->argv2[2], (char *)NULL) < 0){
                        perror("execl4");
                    }
                }
            }
            exit(0);
        }
    }

    return 0;
}

int exec(struct value_st *input){
    if(input->pipe_count == 0){
        do_without_pipeline(input);
    }
    else{
        do_with_pipeline(input);
    }

    return 0;

}


int main(int argc, char* argv[]) {

    struct value_st input;
    input.pipe_count = 0;

    // support up to 2 pipelines and up to 2 parameters
    parse_input(argc, argv, &input);

    // execute the program
    exec(&input);

}






