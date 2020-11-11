#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

/**
 * 来自ls.c
 * @return
 */
int matchhere(char *, char *);
/**
 * 来自ls.c
 * @return
 */
int matchstar(int, char *, char *);

int matchstar(int c, char *re, char *text) {
    do {
        if (matchhere(re, text))
            return 1;
    } while (*text != '\0' && (*text++ == c || c == '.'));
    return 0;
}

int matchhere(char *re, char *text) {
    if (re[0] == '\0')
        return 1;
    if (re[1] == '*')
        return matchstar(re[0], re + 2, text);
    if (re[0] == '$' && re[1] == '\0')
        return *text == '\0';
    if (*text != '\0' && (re[0] == '.' || re[0] == *text))
        return matchhere(re + 1, text + 1);
    return 0;
}


/**
 * 获取路径的文件名，来自ls.c
 * @param path
 * @return
 */
char *
fmtname(char *path) {
    static char buf[DIRSIZ + 1];
    char *p;
    for (p = path + strlen(path); p >= path && *p != '/'; p--);
    p++;
    if (strlen(p) >= DIRSIZ)
        return p;
    memmove(buf, p, strlen(p));
    memset(buf + strlen(p), ' ', DIRSIZ - strlen(p));
    return buf;
}


/**
 * 字符串匹配，来自ls.c
 * @param re 匹配符
 * @param text 字符串
 * @return 匹配结果
 */
int match(char *re, char *text) {
    if (re[0] == '^')
        return matchhere(re + 1, text);
    do {
        if (matchhere(re, text))
            return 1;
    } while (*text++ != '\0');
    return 0;
}


/**
 * 查找文件
 * @param path 根目录
 * @param key 查找关键字
 */
void find(char *path, char *key) {
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if ((fd = open(path, 0)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch (st.type) {
        case T_FILE:
            if (match(key, fmtname(path))) {
                printf("%s\n", path);
            }
            break;
        case T_DIR:
            if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
                printf("find: path too long\n");
                break;
            }
            strcpy(buf, path);
            p = buf + strlen(buf);
            *p++ = '/';//路径加上/,对其下所有文件递归处理
            while (read(fd, &de, sizeof(de)) == sizeof(de)) {
                if (de.inum == 0)
                    continue;
                memmove(p, de.name, DIRSIZ);
                p[DIRSIZ] = 0;
                if (stat(buf, &st) < 0) {
                    printf("find: cannot stat %s\n", buf);
                    continue;
                }
                //防止进入到.和..产生无限递归
                if ((strlen(de.name) == 1 && de.name[0] == '.') ||
                    (strlen(de.name) == 2 && de.name[0] == '.' && de.name[1] == '.')) {
                    continue;
                }
                find(buf, key);
            }
            break;
    }
    close(fd);
}


int main(int argc, char *argv[]) {
    if (argc <= 2){
        fprintf(2, "find: no params provided\n");
    }else{
        find(argv[1], argv[2]);
    }
    exit(0);
}