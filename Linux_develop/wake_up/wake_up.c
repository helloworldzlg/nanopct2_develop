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

unsigned int i2c_fileId;

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

#define xfm20512_ADDR           (0x47)
#define I2C_BUF_LEN             (256)
#define DELAY_MS(ms)            usleep((ms) * 1000)

/*****************************************************************************
 å‡?æ•?å? : i2c_write_proc
 åŠŸèƒ½æè¿°  : I2Cå†™å¤„ç?
 è¾“å…¥å‚æ•°  : int fd              
             unsigned char addr  
             unsigned char reg   
             unsigned char *val  
             unsigned char len   
 è¾“å‡ºå‚æ•°  : æ—?
 è¿?å›?å€? : static
 è°ƒç”¨å‡½æ•°  : 
 è¢«è°ƒå‡½æ•°  : 
 
 ä¿®æ”¹åŽ†å²      :
  1.æ—?   æœ?  : 2016å¹?æœ?4æ—?
    ä½?   è€?  : zlg
    ä¿®æ”¹å†…å®¹   : æ–°ç”Ÿæˆå‡½æ•?

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
 å‡?æ•?å? : i2c_read_proc
 åŠŸèƒ½æè¿°  : I2Cè¯»å¤„ç?
 è¾“å…¥å‚æ•°  : int fd              
             unsigned char addr  
             unsigned char reg   
             unsigned char *val  
             unsigned char len   
 è¾“å‡ºå‚æ•°  : æ—?
 è¿?å›?å€? : static
 è°ƒç”¨å‡½æ•°  : 
 è¢«è°ƒå‡½æ•°  : 
 
 ä¿®æ”¹åŽ†å²      :
  1.æ—?   æœ?  : 2016å¹?æœ?4æ—?
    ä½?   è€?  : zlg
    ä¿®æ”¹å†…å®¹   : æ–°ç”Ÿæˆå‡½æ•?

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
 å‡?æ•?å? : xfm20512_get_version
 åŠŸèƒ½æè¿°  : èŽ·å–ç‰ˆæœ¬ä¿¡æ¯
 è¾“å…¥å‚æ•°  : int fd                 
             unsigned int *version  
 è¾“å‡ºå‚æ•°  : æ—?
 è¿?å›?å€? : 
 è°ƒç”¨å‡½æ•°  : 
 è¢«è°ƒå‡½æ•°  : 
 
 ä¿®æ”¹åŽ†å²      :
  1.æ—?   æœ?  : 2016å¹?æœ?4æ—?
    ä½?   è€?  : zlg
    ä¿®æ”¹å†…å®¹   : æ–°ç”Ÿæˆå‡½æ•?

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
 å‡?æ•?å? : xfm20512_get_degree
 åŠŸèƒ½æè¿°  : èŽ·å–è§’åº¦ä¿¡æ¯
 è¾“å…¥å‚æ•°  : int fd                
             unsigned int *degree  
 è¾“å‡ºå‚æ•°  : æ—?
 è¿?å›?å€? : 
 è°ƒç”¨å‡½æ•°  : 
 è¢«è°ƒå‡½æ•°  : 
 
 ä¿®æ”¹åŽ†å²      :
  1.æ—?   æœ?  : 2016å¹?æœ?4æ—?
    ä½?   è€?  : zlg
    ä¿®æ”¹å†…å®¹   : æ–°ç”Ÿæˆå‡½æ•?

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

    i2c_fileId = open("/dev/i2c-0", O_RDWR);
    if (i2c_fileId == -1) 
    {
        perror("Open i2c-0 Port Error!\n");
        return -1;
    }
#if 0
    if (ioctl(i2c_fileId, I2C_SLAVE, xfm20512_ADDR) < 0)
    {
        perror("ioctl error\n");
        return -1;
    }

    unsigned int version;    
    if (!xfm20512_get_version(i2c_fileId, &version))
    {
        printf("version = %d\n", version);
    }    
#endif    
    return 0;
}

/****************************************************************
 * Main
 ****************************************************************/
int main()
{
    int gpio_fd;
	unsigned int gpio = 68;
	   
    //i2c_init();
    
    //system("chmod 777 /sys/class/gpio");
#if 0    
    system("chmod 777 /sys/class/gpio/export");
    system("echo 68 > export");
    system("chmod 777 /sys/class/gpio/gpio68/direction");
    system("chmod 777 /sys/class/gpio/gpio68/edge");
    system("chmod 777 /sys/class/gpio/gpio68/value");

	gpio_export(gpio);
	gpio_set_dir(gpio, 0);
	gpio_set_edge(gpio, "rising");
	
	gpio_fd = gpio_fd_open(gpio);
	gpio_fd_close(gpio_fd);
#endif
    printf("main finished!");
    	
	return 0;
}