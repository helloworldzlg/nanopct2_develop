#include <string.h>
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include "uart.h"

#ifdef __cplusplus
extern "C"
{
#endif


#define   UART_SUCCESS                             (0)
#define   UART_FAIL                                (-1)	
#define   TRANSMIT_DATA_SIZE                       (256)


/*******************************************************
 * 串口设备打开
********************************************************/
int SerialPort_open(char* path, unsigned int oflag)
{
    int fd;

    fd = open(path, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1)
    {
        perror("Open Serial Port Error!\n");
        return fd;
    }

    struct termios options;
    tcgetattr(fd, &options);

    //115200, 8N1
    options.c_cflag     = B115200 | CS8 | CLOCAL | CREAD;
    options.c_iflag     = IGNPAR;
    options.c_oflag     = 0;
    options.c_lflag     = 0;
    options.c_cc[VTIME] = 0;
    options.c_cc[VMIN]  = 1;
    tcflush(fd, TCIFLUSH);

    tcsetattr(fd, TCSANOW, &options);

    return fd;    
}

/*******************************************************
 * 串口设备关闭
********************************************************/
int SerialPort_close(const int fd)
{
    close(fd);
    
    return UART_SUCCESS;
}

/*******************************************************
 * 串口设备发送
********************************************************/
int SerialPort_write(const int fd, char* tx_data, int tx_size)
{
    int ret = UART_FAIL;

    char tx_buf[TRANSMIT_DATA_SIZE];
    memset(tx_buf, '\0', sizeof(tx_buf));
    memcpy(tx_buf, tx_data, tx_size);

    ret = write(fd, (void*)tx_buf, (int)tx_size);
    if (ret == -1)
    {
        return ret;
    }

    return UART_SUCCESS;  
}

/*******************************************************
 * 串口设备接收
********************************************************/
int SerialPort_read(const int fd, char* rx_data)
{
    int rx_size;
    int ret = UART_FAIL;
    char buf[TRANSMIT_DATA_SIZE];

    memset(buf, '\0', sizeof(buf));

    rx_size = read(fd, (void*)buf, sizeof(buf));
    if (rx_size < 0)
    {
        printf("");
    }

    return UART_SUCCESS;  
}

int Robot_Visual_Data_Write(const int fd, ROBOT_VISUAL_S *visual)
{
    int ret;
    char buf[TRANSMIT_DATA_SIZE];
    memset(buf, '\0', sizeof(buf));
    
    buf[0] = visual->sync_quality;
    buf[1] = visual->angle_q6_checkbit & 0xFF;
    buf[2] = (visual->angle_q6_checkbit >> 8) & 0xFF;
    buf[3] = visual->distance_q2 & 0xFF;
    buf[4] = (visual->distance_q2 >> 8) & 0xFF;
    
    ret = SerialPort_write(fd, buf, sizeof(ROBOT_VISUAL_S));
    if (ret != UART_SUCCESS)
    {
        return ret;        
    }
    
    return UART_SUCCESS; 
}



#ifdef __cplusplus
}
#endif







