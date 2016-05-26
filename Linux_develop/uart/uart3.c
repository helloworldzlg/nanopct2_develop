#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

int main()
{
	printf("\n********  UART3 TEST!!  ********\n");
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
	
	unsigned char rx_buffer[256];
	unsigned char tx_buffer[256] = {'h', 'e', 'l', 'l', 'o', 'w', 'o', 'r', 'l', 'd'};
	
    int index1, index2 = 0;
    while (index2 < 50) {
		write(fd, (void*)tx_buffer, 16);
				
		for (index1 = 0; index1 < 100000000; index1++){};

		printf("%2d: ", index2++);
		
		int rx_length = read(fd, (void*)rx_buffer, 255);
		if (rx_length > 0) {
			//Bytes received
			rx_buffer[rx_length] = '\0';
			printf("%i bytes read : %s\n", rx_length, rx_buffer);
		}
    }

	close(fd);
	return 0;
}
