#include "kernel/types.h"
#include "kernel/stat.h"  // struct stat、T_FILE、T_DIR
#include "kernel/fs.h"    // struct dirent、DIRSIZ
#include "kernel/fcntl.h" // O_RDONLY
#include "user/user.h"
char *
basename(char *path)
{
    char *p;

    p = path + strlen(path);

    while (p > path && *(p - 1) != '/')
        p--;

    return p;
}
void find(char *path, char *target)
{
    int fd, n;
    struct stat st;
    struct dirent de;
    char buf[512];
    char *p;
    fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }
    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }
    if (st.type == T_FILE)
    {
        if (strcmp(basename(path), target) == 0)
            printf("%s\n", path);

        close(fd);
        return;
    }
    if (st.type != T_DIR)
    {
        close(fd);
        return;
    }
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf))
    {
        fprintf(2, "find: path too long\n");
        close(fd);
        return;
    }
    strcpy(buf, path);
    p = buf + strlen(buf);
    if (p == buf || *(p - 1) != '/')
        *p++ = '/';

    while ((n = read(fd, &de, sizeof(de))) == sizeof(de))
    {
        if (de.inum == 0)
            continue;

        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = '\0';

        if (strcmp(p, ".") == 0 || strcmp(p, "..") == 0)
            continue;

        if (stat(buf, &st) < 0)
        {
            fprintf(2, "find: cannot stat %s\n", buf);
            continue;
        }

        if (st.type == T_FILE)
        {
            if (strcmp(p, target) == 0)
                printf("%s\n", buf);
        }
        else if (st.type == T_DIR)
        {
            find(buf, target);
        }
    }
    if (n < 0)
        fprintf(2, "find: read failed %s\n", path);
    close(fd);
}
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(2, "Usage: find path filename\n");
        exit(1);
    }

    find(argv[1], argv[2]);

    exit(0);
}