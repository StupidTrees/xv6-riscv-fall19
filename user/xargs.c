#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {

    //保存每次要传入exec的参数
    char args[32][32];

    if (argc < 2) {
        fprintf(2, "xargs: no params provided\n");
    }

    //将输入参数保存
    for (int i = 1; i < argc; i++) {
        strcpy(args[i - 1], argv[i]);
    }

    //每次输入命令的buffer
    char input[512];
    //exec的传入参数,为字符串指针
    char *argsPtr[32];

    //指向字符串数组
    for (int i = 0; i < 32; i++) {
        argsPtr[i] = args[i];
    }

    int n;
    int index = -1;
    while ((n = read(0, input, sizeof(input))) > 0) {
        if (index > 0) {
            argsPtr[index + 1] = args[index + 1];//恢复指针
        }
        index = argc - 1;

        char *a = args[index];

        //将输入字符串切分，塞入args数组
        char *p = input;
        int state = 1;//1表示读入空位，0表示读入内容
        while (*p) {
            if (*p == ' ' || *p == '\n') {
                if (state == 0) { //如果刚读完内容
                    *a = '\0';
                    index++;
                    a = args[index];
                }
                state = 1;
            } else {
                state = 0;
                *a = *p;//拷贝字符
                a++;
            }
            p++;
        }

        *a = '\0';
        argsPtr[index + 1] = 0; //最后以0结尾，否则exec执行失败
        if (fork()) {
            wait(0);
            memset(input, '\0', sizeof(input));//清空input
        } else {
            //紫禁城
            exec(argsPtr[0], argsPtr);
        }

    }

    if (n < 0) {
        printf("xargs: read error\n");
        exit(0);
    }

    exit(0);
}