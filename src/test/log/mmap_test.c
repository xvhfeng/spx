/*
 * =====================================================================================
 *
 *       Filename:  mmap_test.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2014/05/29 12时51分03秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/mman.h>
#include <string.h>
int main()
{
    int fd;
    void *start;
    int err = 0;
    fd = open("mmap.txt",O_RDWR|O_APPEND|O_CREAT);
    if(0 == fd) {
        return err;
    }
    char *s1 = "testtestststtet\n";
    char *s2 = "s2dsfsdfsdfsdf\n";
    size_t sss = strlen(s1) + strlen(s2);
    if(0 != (err = ftruncate(fd,sss))){
    }

    start=mmap(NULL,sss,PROT_READ | PROT_WRITE,MAP_PRIVATE,fd,0);
    if(start== MAP_FAILED) /*判断是否映射成功*/
        return 0;
    memcpy(start,s1,strlen(s1));
    memcpy(((char *) start) + strlen(s1),s2,strlen(s2));
    munmap(start,sss); /*解除映射*/
    close(fd);
    fd = open("mmap.txt",O_RDWR|O_APPEND|O_CREAT);
    if(0 == fd) {
        return err;
    }
    char buf[256] = {0};
    int numb = read(fd,buf,256);
    printf("%s",buf);
    close(fd);

}
