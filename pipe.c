/* pipe.c - example of a pipe */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
        pid_t id;
        int count;
        char buf[100];
        int fildes[2];

        pipe(fildes);

        id = fork();

        if (id == 0) {
                //sleep(1);
                /* we are in the child */
                /* close "write" end of pipe */
                close(fildes[1]);
                if ((count = read(fildes[0], buf, 100)) < 0) {
                        fprintf(stderr, "cannot read from pipe\n");
                        exit(1);
                }
                printf("buf[] = %s\n", buf);
                close(fildes[0]);
                exit(0);
        } else {
                /* we are in the parent */
                /* close "read" end of pipe */
                close(fildes[0]);
                if (write(fildes[1], "Hello child!", 13) < 0) {
                        fprintf(stderr, "cannot write to pipe\n");
                }
                close(fildes[1]);
                id = wait(NULL);
        }

        return 0;
}