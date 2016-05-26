

#ifdef __cplusplus
extern "C"
{
#endif


int SerialPort_open(char* path, unsigned int oflag);
int SerialPort_close(const int fd);
int SerialPort_write(const int fd, char* tx_data, int tx_size);
int SerialPort_read(const int fd, char* rx_data);

typedef struct
{
    unsigned char sync_quality;
    unsigned short angle_q6_checkbit;
    unsigned short distance_q2;
}ROBOT_VISUAL_S;


#ifdef __cplusplus
}
#endif








