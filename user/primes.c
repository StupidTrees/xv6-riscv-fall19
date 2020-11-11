#include "kernel/types.h"
#include "user/user.h"

/**
 * 从左侧管道获取输入，筛选后输出给右侧管道
 * @param pdLast  左侧管道
 */
void pass(int pdLast[]) {
    //读取左边传来的第一个数
    int p;
    int r = read(pdLast[0], &p, sizeof(p));
    if(!r || p<0){
        return;
    }
    printf("prime %d -- %d\n", p, pdLast);
    if (p == 0) {
        return;
    }
    int pd[2];//新的管道
    pipe(pd);
    if (fork()) {
        wait(0);//等待筛选完成
        close(pdLast[1]);//释放pd
        close(pdLast[0]);
        pass(pd);//开启右边的管道，用于接受
    } else { //开启一个新进程，把左边传来的剩下的过滤、读出
        int n;
        //不断获取左边管道的输出
        while (1) {
            read(pdLast[0], &n, sizeof(n));
            if(n<0){
                break;
            }
            if (n % p != 0) {
                //筛选，若不能被p整除，则传递
                write(pd[1], &n, sizeof(n));
            }
        }
        n = -1;
        write(pd[1], &n, sizeof(n));
        exit(0);
    }
    return;
}

int main(int argc, char *argv[]) {
    int pd[2];
    pipe(pd);
    if (fork()) {
        //父进程
        int status;
        wait(&status);
        pass(pd);//开启右边的管道，用于接受
    } else {
        //子进程
        int i;
        for (i = 2; i < 36; i++) {
            //向输入口传送第一批原始数据
            write(pd[1], &i, sizeof(i));
        }
        i = -1;
        write(pd[1], &i, sizeof(i));
        exit(0);
    }
    exit(0);
}
