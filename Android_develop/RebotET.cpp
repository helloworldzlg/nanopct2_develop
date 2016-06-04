#include <string.h>
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <android/log.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <com_rebotetso_utils_SerialPortUtil.h>

#ifdef __cplusplus
extern "C"
{
#endif

//Java字符串的类和获取方法ID
jclass    gStringClass;
jmethodID gmidStringInit;
jmethodID gmidStringGetBytes;

#define   TRANSMIT_DATA_SIZE                       (256)

JNIEXPORT jint JNICALL Java_com_rebotetso_utils_SerialPortUtil_isOpen
(JNIEnv *env, jclass cls, jstring str)
{    
    return JNI_SUCCESS;
}

JNIEXPORT jint JNICALL Java_com_rebotetso_utils_SerialPortUtil_isColse
(JNIEnv *env, jclass cls, jstring str)
{
    return JNI_SUCCESS;        
}

JNIEXPORT jint JNICALL Java_com_rebotetso_utils_SerialPortUtil_open
(JNIEnv *env, jclass cls, jstring path, jint oflag)
{
    int fd;
    const char *dev_path = NULL;

    dev_path = env->GetStringUTFChars(path, NULL);

    fd = open(dev_path, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1)
    {
        //perror("Open Serial Port Error!\n");
        return fd;
    }

    env->ReleaseStringUTFChars(path, dev_path);

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

JNIEXPORT jint JNICALL Java_com_rebotetso_utils_SerialPortUtil_close
(JNIEnv *env, jclass cls, jint fd)
{
    close(fd);
    
    return JNI_SUCCESS;
}

JNIEXPORT jint JNICALL Java_com_rebotetso_utils_SerialPortUtil_write__I_3CI
(JNIEnv *env, jclass cls, jint fd, jcharArray tx_data, jint tx_size)
{
    return JNI_SUCCESS;    
}

JNIEXPORT jint JNICALL Java_com_rebotetso_utils_SerialPortUtil_write__ILjava_lang_String_2
(JNIEnv *env, jclass cls, jint fd, jstring tx_data)
{
    int ret = JNI_FAIL;
    int tx_size;
    char tx_buf[TRANSMIT_DATA_SIZE];
    const char *tx_str = NULL;
    memset(tx_buf, '\0', sizeof(tx_buf));

    tx_str =    env->GetStringUTFChars(tx_data, NULL);
    strcpy(tx_buf, tx_str);

    tx_size = strlen(tx_buf);
    ret = write(fd, (void*)tx_buf, (int)tx_size);
    if (ret == -1)
    {
        return ret;
    }

    env->ReleaseStringUTFChars(tx_data, tx_str);

    return JNI_SUCCESS;    
}

JNIEXPORT jobjectArray JNICALL Java_com_rebotetso_utils_SerialPortUtil_read__I_3CI
(JNIEnv *env, jclass cls, jint fd, jcharArray rx_data, jint rx_size)
{
    return JNI_SUCCESS;        
}

JNIEXPORT jstring JNICALL Java_com_rebotetso_utils_SerialPortUtil_read__ILjava_lang_String_2
(JNIEnv *env, jclass cls, jint fd, jstring rx_data)
{
    int rx_size;
    int ret = JNI_FAIL;
    char buf[TRANSMIT_DATA_SIZE];

    memset(buf, '\0', sizeof(buf));

    rx_size = read(fd, (void*)buf, sizeof(buf));
    if (rx_size >= 0)
    {
        rx_data = env->NewStringUTF(buf);
    }

    return rx_data;    
}



#ifdef __cplusplus
}
#endif







