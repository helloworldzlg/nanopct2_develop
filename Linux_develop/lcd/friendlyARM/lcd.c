#include <unistd.h>   
#include <stdio.h>   
#include <fcntl.h>   
#include <linux/fb.h>   
#include <sys/mman.h>   
  
#define RED_COLOR565    0x0F100   
#define GREEN_COLOR565  0x007E0   
#define BLUE_COLOR565   0x0001F   
  
int main(void)  
{  
    int fd_fb = 0;  
    struct fb_var_screeninfo vinfo;  
    struct fb_fix_screeninfo finfo;  
    long int screen_size = 0;  
    int *fbp565 = NULL;  
  
    int x = 0, y = 0;  
  
    fd_fb = open("/dev/fb0", O_RDWR);  
    if (!fd_fb)  
    {  
        printf("Error: cannot open framebuffer device.\n");  
        exit(1);  
    }  
  
    // Get fixed screen info   
    if (ioctl(fd_fb, FBIOGET_FSCREENINFO, &finfo))  
    {  
        printf("Error reading fixed information.\n");  
        exit(2);  
    }  
  
    // Get variable screen info   
    if (ioctl(fd_fb, FBIOGET_VSCREENINFO, &vinfo))  
    {  
        printf("Error reading variable information.\n");  
        exit(3);  
    }  
  
    // the size of the screen in bytes   
    screen_size = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;  
  
    printf("%dx%d, %dbpp, screen_size = %d\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel, screen_size );  

    ioctl(fd_fb, FBIOBLANK,0);                        //´ò¿ªLCD±³¹â
    
    // map framebuffer to user memory   
    fbp565 = (int *)mmap(0, screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_fb, 0);  
  
    if ((int)fbp565 == -1)  
    {  
        printf("Error: failed to map framebuffer device to memory.\n");  
        exit(4);  
    }  
  
    if (vinfo.bits_per_pixel == 32)  
    {  
        printf("16 bpp framebuffer\n");  
  
        // Red Screen   
        printf("Red Screen\n");        

        while (1)
        {
            for(y = 0; y < vinfo.yres;  y++)  
            {  
                for(x = 0; x < vinfo.xres; x++)  
                {  
                    //*(fbp565 + y * vinfo.xres + x) = RED_COLOR565;
                    *(fbp565 + y * vinfo.xres + x) = 0x0000FF;
                }  
            }

            usleep(100000);
            
            for(y = 0; y < vinfo.yres;  y++)  
            {  
                for(x = 0; x < vinfo.xres; x++)  
                {  
                    //*(fbp565 + y * vinfo.xres + x) = RED_COLOR565;
                    *(fbp565 + y * vinfo.xres + x) = 0x00FF00;
                }  
            }

            usleep(100000);

            for(y = 0; y < vinfo.yres;  y++)  
            {  
                for(x = 0; x < vinfo.xres; x++)  
                {  
                    //*(fbp565 + y * vinfo.xres + x) = RED_COLOR565;
                    *(fbp565 + y * vinfo.xres + x) = 0xFF0000;
                }  
            } 

            usleep(100000);
        }
        
  

  #if 0
        // Green Screen   
        printf("Green Screen\n");  
        for(y = vinfo.yres/3; y < (vinfo.yres*2)/3; y++)  
        {  
            for(x = 0; x < vinfo.xres/2; x++)  
            {  
                *(fbp565 + y * vinfo.xres + x) =GREEN_COLOR565;  
            }  
        }  
  
        // Blue Screen   
        printf("Blue Screen\n");  
        for(y = (vinfo.yres*2)/3; y < vinfo.yres; y++)  
        {  
            for(x = 0; x < vinfo.xres/2; x++)  
            {  
                *(fbp565 + y * vinfo.xres + x) = BLUE_COLOR565;  
            }  
        }  

  #endif
    }        
    else  
    {  
        printf("warnning: bpp is not 16\n");  
    }  
  
    munmap(fbp565, screen_size);  
    close(fd_fb);  
    return 0;  
}  