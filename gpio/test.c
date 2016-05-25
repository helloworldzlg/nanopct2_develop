#include <stdio.h>
#include <fcntl.h>

#define GPIO_EXPORT          "/sys/class/export"
#define GPIOC4_DIR           "/sys/class/gpio/gpio68/direction"
#define GPIOC4_VALUE         "/sys/class/gpio/gpio68/value"


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
    
    gpio_set(GPIOC4_VALUE, "low");
    
    return 0;
}

int gpio_set(const char* gpioport, const char* level)
{
    int fd;
    
    fd = open(gpioport, O_WRONLY, 0666);
    if (fd < 0)
    {
        printf("Error: can't open gpio c4.\n");
        return -1;
    }    
    
    write(fd, level, sizeof(level));
    
    close(fd);
    
    return 0;
}

int main()
{
    int ret;
    int count = 0;
    
    ret = init();
    
    while (count++ < 50)
    {
        gpio_set(GPIOC4_VALUE, "high");
        
        sleep(500);
        
        gpio_set(GPIOC4_VALUE, "low");        
    }
    
    return 0;
}













