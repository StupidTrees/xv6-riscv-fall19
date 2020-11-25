#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int EXE = 31;
int REDIR_L = 32;
int REDIR_R = 33;
int REDIR_B = 34;

typedef struct command {
    char args[10][20];
    char *argsPtr[10];
    int type;
    char file1[10];//重定向指令<文件名
    char file2[10];//重定向指令>文件名
    int fd;//重定向指令的fd
    int argNum;
} COMMAND;


int getcmd(char *buf, int nbuf) {
    fprintf(2, "@ ");
    memset(buf, 0, nbuf);
    gets(buf, nbuf);
    if (buf[0] == 0) // EOF
        return -1;
    return 0;
}

void parseCommand(COMMAND *command, char *line, char *end) {
    char *p = line;
    int argIndex = -1;
    int inputState = 0;//0:空格，1：参数字符，2：第一个文件准备，3：第一个文件名读取 4：第二个文件准备 5：第二个文件名读取
    int arrowed = 0;
    char *a = command->args[0];
    command->type = EXE;
    while (*p && p < end) {
        if (*p == ' ' || *p == '\n' || *p == '\r') {
            if (inputState == 1 || inputState == 3 || inputState == 5) *a = '\0';
            if (inputState < 2) inputState = 0;
        } else if (*p == '<' || *p == '>') {//识别到重定向符号，开始读取文件
            if (inputState == 1 || inputState == 3) *a = '\0';
            if (!arrowed) { //第一个箭头
                command->type = (*p == '<') ? REDIR_L : REDIR_R;
                command->fd = (*p == '<') ? 0 : 1;//箭头右边是标准输入/输出
                inputState = 2;
                arrowed = 1;
            } else {//已输入过箭头，说明为双重重定向
                command->type = REDIR_B;
                inputState = 4;
            }
        } else {
            if (inputState == 0) {//从空格状态接收第一个字符，声明新建一个参数
                inputState = 1;
                argIndex++;
                a = command->args[argIndex];//指向下一个区域
                command->argsPtr[argIndex] = a;//同时指定指针
            } else if (inputState == 2) {//进行文件名输入
                inputState = 3;
                a = command->file1;//将复制目标更改为file数组
            } else if (inputState == 4) {//进行第二个文件名输入
                inputState = 5;
                a = command->file2;//将复制目标更改为file数组
            }
            *a = *p;//复制到目标数组
            a++;
        }
        p++;
    }
    command->argsPtr[argIndex + 1] = 0;
    command->argNum = argIndex + 1;
}

void splitPipe(char *line, COMMAND *commands, int *num) {
    char *p = line;
    char *start = line;
    int index = 0;
    while (*p) {
        if (*p == '|') {
            parseCommand(&commands[index], start, p);
            index++;
            start = p + 1;
        }
        p++;
    }
    parseCommand(&commands[index], start, p);
    *num = index + 1;
}


void executeCommand(COMMAND command) {
    if (command.type == EXE) {
//        for (int i = 0; i < command.argNum; i++) {
//            fprintf(2, "execute arg[%d]:%s;\n", i, command.argsPtr[i]);
//        }
        exec(command.args[0], command.argsPtr);
    } else if (command.type == REDIR_L || command.type == REDIR_R) {//重定向指令
        close(command.fd);//关闭原有的fd
        int mode = command.type == REDIR_R ? (O_WRONLY | O_CREATE) : O_RDONLY;
        //打开指定的fd，实现重定向
        if (open(command.file1, mode) < 0) {
            fprintf(2, "open %s failed\n", command.file1);
            exit(1);
        }
        command.type = EXE;//改成EXE型的
        //fprintf(2, "file: %s;\n", command.file1);
        executeCommand(command);
    } else if (command.type == REDIR_B) {
        close(0);//先关掉输入
        if (open(command.file1, O_RDONLY) < 0) {
            fprintf(2, "open %s failed\n", command.file1);
            exit(1);
        }
        close(1);//关掉输出
        if (open(command.file2, O_WRONLY | O_CREATE) < 0) {
            fprintf(2, "open %s failed\n", command.file2);
            exit(1);
        }
        command.type = EXE;//改成EXE型的
//        fprintf(2, "file1: %s;\n", command.file1);
//        fprintf(2, "file2: %s;\n", command.file2);
        executeCommand(command);
    }
}


void executePipe(COMMAND left, COMMAND right) {
    int p[2];
    if (pipe(p) < 0) {
        fprintf(2, "create pipe failed\n");
        return;
    }
    if (fork() == 0) {
        close(1);//关闭标准输出
        dup(p[1]);//复制并打开p的输出端代替标准输出
        close(p[0]); //关掉p管
        close(p[1]);
        executeCommand(left);
    }
    if (fork() == 0) {
        close(0);//关闭标准输入
        dup(p[0]);//复制并打开p的输入端代替标准输入
        close(p[0]);//关掉p管
        close(p[1]);
        executeCommand(right);
    }
    close(p[0]);
    close(p[1]);
    wait(0);
    wait(0);
}

int main(void) {
    static char buf[100];
    while (getcmd(buf, sizeof(buf)) >= 0) {
//        if (buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' ') {
//            // Chdir must be called by the parent, not the child.
//            buf[strlen(buf) - 1] = 0;  // chop \n
//            continue;
//        }
        if (fork() == 0) {
            int number;
            COMMAND commands[2];
            splitPipe(buf, commands, &number);
            if (number > 1) {
                //fprintf(2, "pipe_command\n");
                executePipe(commands[0], commands[1]);
            } else {
                //fprintf(2, "single_command\n");
                executeCommand(commands[0]);
            }
            exit(0);
        } else {
            wait(0);
        }
    }
    exit(0);
}