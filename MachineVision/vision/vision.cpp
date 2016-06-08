#include <string.h>
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <android/log.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include "com_robot_et_core_hardware_vision_Vision.h"

#ifdef __cplusplus
extern "C"
{
#endif

//Java字符串的类和获取方法ID
jclass    gStringClass;
jmethodID gmidStringInit;
jmethodID gmidStringGetBytes;

#define ROBOT_VISION_DOT_NUM              (640)

#define VISION_DOT_LEFT_START             (0)
#define VISION_DOT_LEFT_END               (ROBOT_VISION_DOT_NUM/3)
#define VISION_DOT_FORWARD_START          (ROBOT_VISION_DOT_NUM/3+1)
#define VISION_DOT_FORWARD_END            (2*ROBOT_VISION_DOT_NUM/3)
#define VISION_DOT_RIGHT_START            (2*ROBOT_VISION_DOT_NUM/3+1)
#define VISION_DOT_RIGHT_END              (ROBOT_VISION_DOT_NUM)

#define UART_RX_BUFF_SIZE                 (4096)

#define VISION_INFO_EFFECTIVE             (1)
#define VISION_INFO_UNEFFECTIVE           (0)

int g_uart4_fileid;
char g_VisionInfo[UART_RX_BUFF_SIZE];
unsigned int g_dot_distance[ROBOT_VISION_DOT_NUM];

#define MOVE_FORWARD                      (1)
#define MOVE_BACK                         (2)
#define MOVE_LEFT                         (3)
#define MOVE_RIGHT                        (4)
#define MOVE_STOP                         (5)

void set_uart4_fileid(int fd)
{
    g_uart4_fileid = fd;
    return;
}

int get_uart4_fileid()
{
    return g_uart4_fileid;
}

char* get_vision_info_addr()
{
    return g_VisionInfo;
}

unsigned int* get_dot_distance_addr()
{
    return g_dot_distance;
}

/*************************************************
 * 核心板与视觉板通信串口资源初始化
*************************************************/
static int SerialPort_init()
{
    int fd;
    struct termios options;
    
    fd = open("/dev/ttyAMA4", O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) 
    {
        return fd;
    }
        
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

    set_uart4_fileid(fd);

    return 0;
}

/*************************************************
 * 视觉扫描信息获取
*************************************************/
static void vision_info_read()
{
    int size;
    int count = 0;
    char rx_buffer[512];
    void* pVisionInfo = (void*)get_vision_info_addr();

    while (count < UART_RX_BUFF_SIZE)
    {
        size = read(get_uart4_fileid(), (void*)rx_buffer, sizeof(rx_buffer));
        if (size > 0)
        {
            if (size + count <= UART_RX_BUFF_SIZE)
            {
                memcpy((void*)pVisionInfo + count, (void*)rx_buffer, size);
                count += size;
            }
            else
            {
                memcpy((void*)pVisionInfo + count, (void*)rx_buffer, UART_RX_BUFF_SIZE - count);
                count = UART_RX_BUFF_SIZE;
            }
        }
    }

    /* 替换最后的字符为" \n"  */
    pVisionInfo[UART_RX_BUFF_SIZE-2] = ' ';
    pVisionInfo[UART_RX_BUFF_SIZE-1] = '\n';
    
    return;
}


/*************************************************
 * 视觉扫描信息解析
*************************************************/
static void analysis_vision_info(char* vision_info, const unsigned int size)
{
    unsigned int index;
    unsigned int dot_index;
    char temp[20];
    char* p_head = NULL;
    char* p_tail = NULL;

    /* 搜索并跳过第一个\n */
    p_head = strchr(vision_info, '\n');

    index = (unsigned int)p_head - (unsigned int)vision_info;
    index++;

    memset((void*)g_dot_distance, 0, sizeof(g_dot_distance));

    while (index < size-1)
    {
        /* 抓取像素点 */
        p_head = vision_info + index;
        p_tail = strchr(p_head, ' ');

        memset((void*)temp, '\0', sizeof(temp));
        memcpy((void*)temp, p_head, (unsigned int)p_tail - (unsigned int)p_head);

        dot_index = atoi(temp);

        index += (unsigned int)p_tail - (unsigned int)p_head;

        /* 抓取像素点对应距离 */
        p_head = vision_info + index;
        p_tail = strchr(p_head, '\n');

        memset((void*)temp, '\0', sizeof(temp));
        memcpy((void*)temp, p_head, (unsigned int)p_tail - (unsigned int)p_head);

        g_dot_distance[dot_index] = atoi(temp);

        index += (unsigned int)p_tail - (unsigned int)p_head;
        index += 1;
    }

    return;
}

/*************************************************
 * 视觉避障算法
*************************************************/
int obstacle_avoidance_algorithm()
{
    int i;
    int dot_index = 0;
    int max = g_dot_distance[0];

    for (i = 1; i < sizeof(g_dot_distance)/sizeof(g_dot_distance[0]); i++)
    {
        if (g_dot_distance[i] > max)
        {
            max = g_dot_distance[i];
            dot_index = i;
        }
    }

    if (0 == max)
    {
        return MOVE_STOP;
    }
    else if ((dot_index >= VISION_DOT_LEFT_START) && (dot_index <= VISION_DOT_LEFT_END))
    {
        return MOVE_LEFT;
    }
    else if ((dot_index >= VISION_DOT_FORWARD_START) && (dot_index <= VISION_DOT_FORWARD_END))
    {
        return MOVE_FORWARD;
    }
    else if ((dot_index >= VISION_DOT_RIGHT_START) && (dot_index <= VISION_DOT_RIGHT_END))
    {
        return MOVE_RIGHT;
    }

    return MOVE_FORWARD;
}

JNIEXPORT jint JNICALL Java_com_robot_et_core_hardware_vision_Vision_init
(JNIEnv *env, jclass cls)
{
    return SerialPort_init();
}

/*************************************************
 * 根据视觉扫描信息计算避障方向
*************************************************/
JNIEXPORT jstring JNICALL Java_com_robot_et_core_hardware_vision_Vision_getMoveDirection
(JNIEnv *env, jclass cls)
{
    char buf[10];
    int dir;
    
    vision_info_read();
    
    analysis_vision_info(g_VisionInfo, sizeof(g_VisionInfo));

    dir = obstacle_avoidance_algorithm();

    memset((void*)buf, '\0', sizeof(buf));
    buf[0] = dir + '0';
    buf[1] = '\n';
    
    return env->NewStringUTF(buf);
}
  




#ifdef __cplusplus
}
#endif