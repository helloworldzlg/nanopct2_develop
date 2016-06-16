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
    unsigned char tx_buffer[] = {0xAA, 0x70, 0x00, 0xCC, 0x33, 0xC3, 0x3C};    
    
    SerialPort_init();
   
	tx_buffer[2] = 0;
	
	write(g_uart3_fileid, (void*)tx_buffer, sizeof(tx_buffer));
	
	usleep(15000);
	      
    return 0;     
}

