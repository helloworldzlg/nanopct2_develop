#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>

int g_uart3_fileid;

int SerialPort_init()
{
	printf("\n********  UART3 Write TEST!!  ********\n");
	int fd = -1;
	fd = open("/dev/ttyAMA2", O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1) {
		perror("Open Serial Port Error!\n");
		return -1;
	}

	struct termios options;
	tcgetattr(fd, &options);

	//115200, 8N1
	options.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	options.c_cc[VTIME]=0;
	options.c_cc[VMIN]=1;
	tcflush(fd, TCIFLUSH);

	tcsetattr(fd, TCSANOW, &options);

	g_uart3_fileid = fd;

	return 0;
}

int main()
{
    int count = 0;
    int index = 0;
    unsigned char tx_buffer[512];
    
    memset(tx_buffer, '\0', sizeof(tx_buffer));
    
    SerialPort_init();
   
    while (1) {
		count = 0;
        memset(tx_buffer, '\0', sizeof(tx_buffer));

        count += sprintf(tx_buffer + count, "%s", "5a5a5a5a5a5a5a5a\n");
        for (index = 0; index < 20; index++)
        {
            count += sprintf(tx_buffer + count, "%2d  ", index);
            count += sprintf(tx_buffer + count, "%s\n", "hello");            
        }
        count += sprintf(tx_buffer + count, "%s", "a5a5a5a5a5a5a5a5\n");
                    
        write(g_uart3_fileid, (void*)tx_buffer, count);            
        
        sleep(1);            
    }
    
    return 0;     
}

