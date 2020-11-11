#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    int fd1[2], fd2[2];
    pipe(fd1);
    pipe(fd2);
    char buf[64];

    if (fork()) {
        // 父进程
        write(fd1[1], "ping", strlen("ping"));
        read(fd2[0], buf, 4);
        printf("%d: received %s\n", getpid(), buf);
    } else {
        // 子进程
        read(fd1[0], buf, 4);
        printf("%d: received %s\n", getpid(), buf);
        write(fd2[1], "pong", strlen("pong"));
    }

    exit(0);
}


