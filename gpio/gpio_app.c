#include <stdio.h>
//#include <conio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/ioctl.h>
#include <signal.h>


int main(int argc ,char *argv[])
{
    int fd;
    char input;
    fd = open("/dev/PB18_IRQTest", O_RDWR);
    if (fd < 0)
    {       
        perror("open PB18_IRQTest device");
        return 0;
    }
    
    while (1)
    {
        printf("input 0 to trigger int/n");
        scanf("%c",&input);
        switch (input)
        {
        case '0':
            ioctl(fd, 0, 0);
            printf("/n");
        break;
        
        case '1':
            ioctl(fd, 1, 0);
            printf("/n");
        break;
        
        default:
            printf("/n");
        break;                                        
        }
    }                                                          
    return 0;
}