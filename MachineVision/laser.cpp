
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include "stdio.h"
#include <string.h>
#include <locale.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

using namespace cv;
using namespace std;

const int n = 200;
const float PI = 3.1415926536;

#define   UART_SUCCESS                             (0)
#define   UART_FAIL                                (-1) 
#define   TRANSMIT_DATA_SIZE                       (512)

#define   TRANSMIT_FREQ                            (2)

struct MeasureResult
{
    float distance;
    float yaw;
    float pitch;
};

typedef struct
{
    unsigned char sync_quality;
    unsigned char reserve;
    unsigned short angle_q6_checkbit;
    unsigned short distance_q2;
}ROBOT_VISUAL_S;


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
    options.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    options.c_cc[VTIME] = 0;
    options.c_cc[VMIN] = 1;
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

int Robot_Visual_Data_Write(const int fd, void *visual, unsigned int tx_size)
{
    int ret;
    char buf[TRANSMIT_DATA_SIZE];

    memset((void*)buf, '\0', sizeof(buf));
    memcpy((void*)buf, (void*)visual, tx_size);

    ret = SerialPort_write(fd, buf, tx_size);
    if (ret != UART_SUCCESS)
    {
        return ret;
    }

    return UART_SUCCESS;
}

void calcDistanceByPos(float x, int y, int img_height, MeasureResult * result)
{

    const float a = 1190;
    const float b = -0.030050;
    const float c = 7.482575;

    const float baseline = 70.0;

    const float laser_angle = 0;

    const float rotation_r = 49.2;
    const float focal = a / baseline;

    float center_distance = a / (b*x + c);

    result->distance = center_distance;



}
float findLaserCenterByCol(uchar * col, size_t colsz)
{
    static const unsigned char THRESHOLD_MIN_PWR = 25;
    static const unsigned char THRESHOLD_BLOB_DIFF = 10;
    static const int           THRESHOLD_ALLOWED_BLOB_SZ = 20;
    int centerPos = 0;
    unsigned char maxPwr = 0;
    int centerSize = 0;

    int currentPos = 0;
    while (currentPos<colsz) {
        if (maxPwr<col[currentPos]) {
            centerSize = 1;
            int centerPos_candidate = currentPos;
            unsigned char maxPwr_candidate = col[currentPos];
            maxPwr = maxPwr_candidate;
            centerPos = centerPos_candidate;
        }
        else
        {
            ++currentPos;
        }
    }

    if (maxPwr < THRESHOLD_MIN_PWR) return -1;

    float logicPwr = 0.0f, totalPwr = 0.0f;

    for (currentPos = centerPos - 10; currentPos <= centerPos + 10; currentPos++)
    {
        float currentPwr;
        if (currentPos >= 0 && currentPos<colsz){
            currentPwr = col[currentPos];
        }
        else{
            currentPwr = 0.0f;
        }
        logicPwr += currentPwr;
        totalPwr += currentPwr*currentPos;
    }

    return totalPwr / logicPwr;
}




int main(int argc, char** argv)
{
    Mat currentFrame;
    Mat current;
    Mat grayFrame;
    Mat gaussianblurFrame;
    Mat undistorted;
    Size frameSz;
    Mat dst;
    Mat dst2;
    Mat dst3;
    Mat dst4;
    Mat dst5;
    Mat dst6;

    Mat frame;
    VideoCapture cap(0);
    cap.set(CV_CAP_PROP_FRAME_WIDTH, 640);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
    ROBOT_VISUAL_S Robot_Visual;
    int fileid;

    /**串口资源初始化**/
    fileid = SerialPort_open("/dev/ttyAMA3", 0);
    if (fileid < 0)
    {
        return fileid;
    }

    cap >> frame;

    const int IMAGE_HEIGHT = 480;


    Mat huitu;
    huitu.create(Size(640, 640), CV_8UC3);
    frameSz.height = 640;
    frameSz.width = 480;
    Mat ImgUndistort = frame.clone();
    Mat CM = Mat(3, 3, CV_32FC1);
    Mat D;
    FileStorage fs2("left.yml", FileStorage::READ);
    fs2["cameraMatrix"] >> CM;
    fs2["distortionCoefficients"] >> D;
    fs2.release();
    float * laserDotArr = new float[frameSz.height];
    grayFrame.create(Size(frameSz.width, frameSz.height), CV_8UC1);
    undistorted.create(Size(frameSz.width, frameSz.height), CV_8UC3);
    MeasureResult result = { 0, 0, 0 };
    char buf[1024];
    unsigned int count = 0;
    unsigned int cycle_count = 0;

    while (true)
    {       
        cap >> currentFrame;
        undistort(currentFrame, ImgUndistort, CM, D);
        transpose(ImgUndistort, dst);
        flip(dst, dst2, 1);
        transpose(huitu, dst4);
        flip(dst4, dst5, 1);
        cvtColor(dst2, grayFrame, COLOR_BGR2GRAY);
        GaussianBlur(grayFrame, gaussianblurFrame, Size(3, 3), 0, 0);
        for (int y = 0; y < frameSz.height; ++y)
        {
            laserDotArr[y] = findLaserCenterByCol(gaussianblurFrame.ptr<uchar>(y), frameSz.width);
        }
        
        for (int y = 0; y < frameSz.height; ++y)
        {                      
            if (laserDotArr[y] != -1)
            {
                circle(dst2, cvPoint(laserDotArr[y], y), 2, Scalar(0, 255, 255));
                calcDistanceByPos(laserDotArr[y], y, IMAGE_HEIGHT, &result);
                
                if ((y+1)%TRANSMIT_FREQ != 0)
                {
                    count = sprintf(buf, "%-d ", y);
                    count += sprintf(buf + count, "%-d\n", (unsigned short)result.distance);                    
                }
                                                                
                if ((0 == (y+1)%TRANSMIT_FREQ) && (0 == cycle_count%5))
                {
                    count += sprintf(buf + count, "%-d ", y);
                    count += sprintf(buf + count, "%-d\n", (unsigned short)result.distance);
                    
                    printf("%s", buf);
                    
                    Robot_Visual_Data_Write(fileid, buf, count);
                    
                    count = 0;
                    memset((void*)buf, '\0', sizeof(buf));
                    
                    cycle_count = 0;
                }
                                
                //printf("%d %.2f\n", y, result.distance);

                circle(dst5, cvPoint(result.distance, y), 2, Scalar(255, 255, 255));

                char txtMsg[200];
                if (y == frameSz.height / 2) {
                    sprintf(txtMsg, "y=%.1f dist=%.2f", laserDotArr[y], result.distance);

                    putText(dst2, txtMsg, cvPoint(laserDotArr[y], y),
                        FONT_HERSHEY_SIMPLEX, 0.5, Scalar(200, 200, 200),
                        1, 8,
                        false);
                }
            }
        }
        
        cycle_count++;
        while(1){};
        transpose(dst2, dst);
        flip(dst, dst3, 0);
        transpose(dst5, dst4);
        flip(dst4, dst6, 0);

        //imshow("dst3", dst3);
        //imshow("dst6", dst6);

        waitKey(1);
    }
}
