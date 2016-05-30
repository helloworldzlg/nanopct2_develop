#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

 /****************************************************************
 * Constants
 ****************************************************************/
 
#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
#define MAX_BUF 64

/****************************************************************
 * gpio_export
 ****************************************************************/
int gpio_export(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];
 
	fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}
 
	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);
 
	return 0;
}

/****************************************************************
 * gpio_unexport
 ****************************************************************/
int gpio_unexport(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];
 
	fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}
 
	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);
	return 0;
}

/****************************************************************
 * gpio_set_dir
 ****************************************************************/
int gpio_set_dir(unsigned int gpio, unsigned int out_flag)
{
	int fd, len;
	char buf[MAX_BUF];
 
	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR  "/gpio%d/direction", gpio);
 
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/direction");
		return fd;
	}
 
	if (out_flag)
		write(fd, "out", 4);
	else
		write(fd, "in", 3);
 
	close(fd);
	return 0;
}

/****************************************************************
 * gpio_set_value
 ****************************************************************/
int gpio_set_value(unsigned int gpio, unsigned int value)
{
	int fd, len;
	char buf[MAX_BUF];
 
	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);
 
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/set-value");
		return fd;
	}
 
	if (value)
		write(fd, "1", 2);
	else
		write(fd, "0", 2);
 
	close(fd);
	return 0;
}

/****************************************************************
 * gpio_get_value
 ****************************************************************/
int gpio_get_value(unsigned int gpio, unsigned int *value)
{
	int fd, len;
	char buf[MAX_BUF];
	char ch;

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);
 
	fd = open(buf, O_RDONLY);
	if (fd < 0) {
		perror("gpio/get-value");
		return fd;
	}
 
	read(fd, &ch, 1);

	if (ch != '0') {
		*value = 1;
	} else {
		*value = 0;
	}
 
	close(fd);
	return 0;
}


/****************************************************************
 * gpio_set_edge
 ****************************************************************/

int gpio_set_edge(unsigned int gpio, char *edge)
{
	int fd, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/edge", gpio);
 
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/set-edge");
		return fd;
	}
 
	write(fd, edge, strlen(edge) + 1); 
	close(fd);
	return 0;
}

/****************************************************************
 * gpio_fd_open
 ****************************************************************/

int gpio_fd_open(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);
 
	fd = open(buf, O_RDONLY | O_NONBLOCK );
	if (fd < 0) {
		perror("gpio/fd_open");
	}
	return fd;
}

/****************************************************************
 * gpio_fd_close
 ****************************************************************/

int gpio_fd_close(int fd)
{
	return close(fd);
}

#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

#define xfm20512_ADDR           (0x47)
#define I2C_BUF_LEN             (256)
#define DELAY_MS(ms)            usleep((ms) * 1000)

/*****************************************************************************
 �?�?�? : i2c_write_proc
 功能描述  : I2C写处�?
 输入参数  : int fd              
             unsigned char addr  
             unsigned char reg   
             unsigned char *val  
             unsigned char len   
 输出参数  : �?
 �?�?�? : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.�?   �?  : 2016�?�?4�?
    �?   �?  : zlg
    修改内容   : 新生成函�?

*****************************************************************************/
static int i2c_write_proc
(
    int fd, 
    unsigned char addr, 
    unsigned char reg, 
    unsigned char *val, 
    unsigned char len
)
{
    unsigned char buf[I2C_BUF_LEN];
    struct i2c_rdwr_ioctl_data cmd;
    struct i2c_msg msg[1];

    memset(buf, 0, sizeof(buf));    // zero memset buffer
    buf[0] = reg;                   // The first byte indicates which register we'll write
    memcpy(&buf[1], val, len);      // The last bytes indicates the values to write
    
    /* Construct the i2c_msg struct */
    msg[0].addr  = addr;                // Device address
    msg[0].flags = 0;               // Write option
    msg[0].len   = len + 1;         // Include register byte
    msg[0].buf   = buf;             // Data buffer

    /* Construct the i2c_rdwr_ioctl_data struct */
    cmd.msgs  = msg;                 // Message
    cmd.nmsgs = sizeof(msg) / sizeof(struct i2c_msg);

    /* Transfer the i2c command packet to the kernel and verify it worked */
    if (ioctl(fd, I2C_RDWR, &cmd) < 0) 
    {
        printf("Unable to send data!\n");
        return 1;
    }

    return 0;
}

/*****************************************************************************
 �?�?�? : i2c_read_proc
 功能描述  : I2C读处�?
 输入参数  : int fd              
             unsigned char addr  
             unsigned char reg   
             unsigned char *val  
             unsigned char len   
 输出参数  : �?
 �?�?�? : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.�?   �?  : 2016�?�?4�?
    �?   �?  : zlg
    修改内容   : 新生成函�?

*****************************************************************************/
static int i2c_read_proc
(
    int fd, 
    unsigned char addr, 
    unsigned char reg, 
    unsigned char *val, 
    unsigned char len
) 
{
    struct i2c_rdwr_ioctl_data cmd;
    struct i2c_msg msg[2];

    msg[0].addr  = addr;
    msg[0].flags = 0;
    msg[0].len   = 1;
    msg[0].buf   = &reg;
    
    msg[1].addr  = addr;
    msg[1].flags = I2C_M_RD /* | I2C_M_NOSTART*/;
    msg[1].len   = len;
    msg[1].buf   = val;
    
    /* Construct the i2c_rdwr_ioctl_data struct */
    cmd.msgs  = msg;
    cmd.nmsgs = sizeof(msg) / sizeof(struct i2c_msg);

    /* Send the request to the kernel and get the result back */
    if (ioctl(fd, I2C_RDWR, &cmd) < 0) 
    {
        printf("Unable to send data!\n");
        return -1;
    }

    return 0;
}

/*****************************************************************************
 �?�?�? : xfm20512_get_version
 功能描述  : 获取版本信息
 输入参数  : int fd                 
             unsigned int *version  
 输出参数  : �?
 �?�?�? : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.�?   �?  : 2016�?�?4�?
    �?   �?  : zlg
    修改内容   : 新生成函�?

*****************************************************************************/
int xfm20512_get_version(int fd, unsigned int *version)
{
    unsigned int data = 0x00000F00;

    /* 1. Send version query command */
    if (i2c_write_proc(fd, xfm20512_ADDR, 0x00, (unsigned char *)&data, 4))
        return -1;
    DELAY_MS(1);    // Delay 1ms at least

    /* 2. Query command status */
    do {
        if (i2c_read_proc(fd, xfm20512_ADDR, 0x00, (unsigned char *)&data, 4))
            return -1;
        DELAY_MS(1);    // Delay 1ms at least
    } while (0x00020001 != data);
    
    if (i2c_read_proc(fd, xfm20512_ADDR, 0x01, (unsigned char *)version, 8))
      return -1;
    
    return 0;
}

/*****************************************************************************
 �?�?�? : xfm20512_get_degree
 功能描述  : 获取角度信息
 输入参数  : int fd                
             unsigned int *degree  
 输出参数  : �?
 �?�?�? : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.�?   �?  : 2016�?�?4�?
    �?   �?  : zlg
    修改内容   : 新生成函�?

*****************************************************************************/
int xfm20512_get_degree(int fd, unsigned int *degree)
{
    unsigned int data = 0x00001000;

    /* 1. Send degree query command */
    if (i2c_write_proc(fd, xfm20512_ADDR, 0x00, (unsigned char *)&data, 4))
        return -1;
    DELAY_MS(1);    // Delay 1ms at least

    /* 2. Query command status */
    do {
        if (i2c_read_proc(fd, xfm20512_ADDR, 0x00, (unsigned char *)&data, 4))
            return -1;
        DELAY_MS(1);    // Delay 1ms at least
    } while (0x00010001 != data);
        
    /* 3. Read wakeup degree */
    if (i2c_read_proc(fd, xfm20512_ADDR, 0x01, (unsigned char *)degree, 4))
        return -1;
    
    return 0;
}

int xfm20512_enter_wakeup(int fd)
{
    unsigned int data = 0x00001100;
    
    /* 1. Send enter wakeup command */
    if (i2c_write_proc(fd, xfm20512_ADDR, 0x00, (unsigned char *)&data, sizeof(unsigned int)))
        return -1;
    DELAY_MS(1);    // Delay 1ms at least

    /* 2. Query command status */
    do {
        if (i2c_read_proc(fd, xfm20512_ADDR, 0x00, (unsigned char *)&data, sizeof(unsigned int)))
            return -1;
        DELAY_MS(1);    // Delay 1ms at least
    } while (0x00000001 != data);
    
    return 0;
}

int xfm20512_exit_wakeup(int fd, unsigned int beam)
{
    unsigned int data = 0x00001200 | ((beam & 0x3) << 16);
    
    /* 1. Send exit wakeup command */
    if (i2c_write_proc(fd, xfm20512_ADDR, 0x00, (unsigned char *)&data, sizeof(unsigned int)))
        return -1;
    DELAY_MS(1);    // Delay 1ms at least

    /* 2. Query command status */
    do {
        if (i2c_read_proc(fd, xfm20512_ADDR, 0x00, (unsigned char *)&data, sizeof(unsigned int)))
            return -1;
        DELAY_MS(1);    // Delay 1ms at least
    } while (0x00030001 != data);
    
    return 0;
}

/****************************************************************
 * i2c_init
 ****************************************************************/
int i2c_init()
{
    int fd = -1;
    int ret;
    int write_count = 0;

    /* 打开master侧的I2C-0接口 */
    fd = open("/dev/i2c-0", O_RDWR);
    if (fd == -1) 
    {
        perror("Open i2c-0 Port Error!\n");
        return -1;
    }

    /* 注册从机 */
    if (ioctl(fd, I2C_SLAVE, xfm20512_ADDR) < 0)
    {
        perror("ioctl error\n");
        return -1;
    }

    /* 查询模块版本信息 */
    unsigned int version;
    
    if (!xfm20512_get_version(fd, &version))
    {
        printf("version = %d\n", version);
    }    
    
    return;
}

/****************************************************************
 * Main
 ****************************************************************/
int main(int argc, char **argv, char **envp)
{
	struct pollfd fdset[2];
	int nfds = 2;
	int gpio_fd, timeout, rc;
	char buf[MAX_BUF];
	unsigned int gpio;
	int len;

	if (argc < 2) {
		printf("Usage: gpio-int <gpio-pin>\n\n");
		printf("Waits for a change in the GPIO pin voltage level or input on stdin\n");
		exit(-1);
	}
    
    /* i2c init */
    i2c_init();

	gpio = atoi(argv[1]);

	gpio_export(gpio);
	gpio_set_dir(gpio, 0);
	gpio_set_edge(gpio, "rising");
	gpio_fd = gpio_fd_open(gpio);

	timeout = POLL_TIMEOUT;
 
	while (1) {
		memset((void*)fdset, 0, sizeof(fdset));

		fdset[0].fd = STDIN_FILENO;
		fdset[0].events = POLLIN;
      
		fdset[1].fd = gpio_fd;
		fdset[1].events = POLLPRI;

		rc = poll(fdset, nfds, timeout);      

		if (rc < 0) {
			printf("\npoll() failed!\n");
			return -1;
		}
      
		if (rc == 0) {
			printf(".");
		}
            
		if (fdset[1].revents & POLLPRI) {
			len = read(fdset[1].fd, buf, MAX_BUF);
			printf("\npoll() GPIO %d interrupt occurred\n", gpio);
		}

		if (fdset[0].revents & POLLIN) {
			(void)read(fdset[0].fd, buf, 1);
			printf("\npoll() stdin read 0x%2.2X\n", (unsigned int) buf[0]);
		}

		fflush(stdout);
	}

	gpio_fd_close(gpio_fd);
	return 0;
}