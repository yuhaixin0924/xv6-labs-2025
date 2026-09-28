#include "kernel/types.h"//定义常用的类型别名
#include "kernel/fcntl.h"  // O_RDONLY
#include "user/user.h"//用户程序可以调用哪些函数
int main(int argc,char *argv[]){
    int fd;
    char c;
    int num; 
    int n;
    int bad_token;
    int seen_digit;
    for(int i=1;i<argc;i++){
        num=0;
        bad_token=0;
        seen_digit=0;
        fd=open(argv[i],O_RDONLY);
        while((n=read(fd,&c,1))==1){
            if(c>='0'&&c<='9'){
                if(!bad_token){
                    num=num*10+(c-'0');
                    seen_digit=1;
                }
            }
            else if(strchr(" -\r\t\n./,", c) != 0){
                if(seen_digit&&!bad_token&&(num%5==0||num%6==0)){
                    printf("%d\n",num);
                }
                num=0;
                seen_digit=0;
                bad_token=0;
            }
            else{
                bad_token=1;
            }
        }
        if (n == 0 && seen_digit && !bad_token &&(num % 5 == 0 || num % 6 == 0)){
            printf("%d\n", num);
        }
        if(n<0){
            fprintf(2,"read failed");
        }
        close(fd);
    }
}