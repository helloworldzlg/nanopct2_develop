#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

#define ROBOT_VISION_DOT_NUM              (640)
//#define UART_RX_BUFF_SIZE                 (ROBOT_VISION_DOT_NUM*10)
#define UART_RX_BUFF_SIZE                 (480)


#define VISION_INFO_EFFECTIVE             (1)
#define VISION_INFO_UNEFFECTIVE           (0)

#define VISION_FRAME_START                ("START\n")
#define VISION_FRAME_END                  ("END\n")


int g_uart3_fileid;
char g_uart3_rx_buff[UART_RX_BUFF_SIZE];
char g_dot_distance[ROBOT_VISION_DOT_NUM];


int SerialPort_init()
{
	printf("\n********  UART3 TEST!!  ********\n");
	int fd = -1;
	fd = open("/dev/ttyAMA3", O_RDWR | O_NOCTTY | O_NDELAY);
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

void AnalysisVisionInfo()
{
    
}

char* SearchStrFormStr(char* src, int src_len, char* search, int search_len)
{
    int i;
    int ret;

    for (i = 0; i < src_len; i++)
    {
        if (src[i] == search[0])
        {
            ret = memcmp(&src[i], &search[0], search_len);
            if (0 == ret)
            {
                return &src[i];
            }
        }
    }

    if (i == src_len)
    {
        return NULL;
    }
}
   
void SerialPort_read()
{    
    int count;
    int index = 0;
    int size;
    char* pstart = NULL;
    int info_effective = VISION_INFO_UNEFFECTIVE;
    unsigned char rx_buffer[256];
    unsigned int timeout = 0xFFFFFFFF;
    char start[] = "5a5a5a5a5a5a5a5a\n";
    char end[]   = "a5a5a5a5a5a5a5a5\n";
    
    printf("***************************************\n");
    
    memset((void*)g_uart3_rx_buff, '\0', sizeof(g_uart3_rx_buff));

    while ((!info_effective) && (timeout > 0))
    {
        count = 0;
        memset((void*)rx_buffer, '\0', sizeof(rx_buffer));
        
        size = read(g_uart3_fileid, (void*)rx_buffer, sizeof(rx_buffer));
        if (size > 0)
        {
            pstart = SearchStrFormStr(rx_buffer, size, start, strlen(start));
            if (pstart)
                continue;
                        
            memcpy((void*)g_uart3_rx_buff, (void*)rx_buffer, size);
            count += size;
            
            while (count < UART_RX_BUFF_SIZE)
            {
                size = read(g_uart3_fileid, (void*)rx_buffer, sizeof(rx_buffer));
                if (size > 0) 
                {
                    memcpy((void*)g_uart3_rx_buff + count, (void*)rx_buffer, size);
                    count += size;
                }
            }

            info_effective = VISION_INFO_EFFECTIVE;            
        }
        else
        {
            timeout--;
        }
    }
    
    printf("%s", g_uart3_rx_buff);
    
    AnalysisVisionInfo();
    
    return;
}

int main()
{            
    SerialPort_init();

    struct itimerval value, ovalue;
            
    signal(SIGALRM, SerialPort_read);
    
    value.it_value.tv_sec     = 5;
    value.it_value.tv_usec    = 0;
    value.it_interval.tv_sec  = 5;
    value.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &value, &ovalue);
    
    for (;;);
            
    return 0;     
}



