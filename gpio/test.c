#include <stdio.h>
#include <fcntl.h>

#define GPIO_EXPORT          "/sys/class/gpio/export"
#define GPIOC4_DIR           "/sys/class/gpio/gpio68/direction"
#define GPIOC4_VALUE         "/sys/class/gpio/gpio68/value"
#define DELAY_TIME           (50000000)

int init()
{
    int fd;
    
    fd = open(GPIO_EXPORT, O_WRONLY, 0666);
    if (fd < 0)
    {
        printf("Error: can't open export.\n");
        return -1;
    }
    
    write(fd, "68", sizeof(68));
    
    close(fd);
    
    gpio_set(GPIOC4_DIR, "in");
    
    return 0;
}

int gpio_set(const char* gpioport, const char* level)
{
    int fd;
    
    fd = open(gpioport, O_WRONLY, 0666);
    if (fd < 0)
    {
        printf("Error: can't open gpio %s\n", gpioport);
        return -1;
    }    
    
    write(fd, level, sizeof(level));
    
    close(fd);
    
    return 0;
}

int gpio_get(const char* gpioport, const char* value)
{
    int fd;
    int ret;
    
    fd = open(gpioport, O_WRONLY, 0666);
    if (fd < 0)
    {
        printf("Error: can't open gpio %s\n", gpioport);
        return -1;
    }    
    
    ret = read(fd, value, 1);
    if (ret >= 0)
    {
        printf("ch = %d\n", *value);            
    }
    
    close(fd);
    
    return 0;    
    
}

int main()
{
    int ret;
    int count = 0;
    
    ret = init();
    
    while (1)
    {
        char ch;
        
        gpio_get(GPIOC4_VALUE, &ch);        

        if (count >= 5000000)
        {
            printf("runing\n");
            
            count = 0;
        }
        
        count++;
    }
    
    return 0;
}













