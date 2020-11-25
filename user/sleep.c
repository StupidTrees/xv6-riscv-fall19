#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    if (argc != 2)
        write(2, "Wrong Arguments", strlen("Wrong Arguments"));
    int x = atoi(argv[1]);
    printf("Sleep %d\n",x); //打印sleep信息
    sleep(x); //执行sleep
    exit(0);
}