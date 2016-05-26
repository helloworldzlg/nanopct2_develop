

#ifdef __cplusplus
extern "C"
{
#endif


int SerialPort_open(char* path, unsigned int oflag);
int SerialPort_close(const int fd);
int SerialPort_write(const int fd, char* tx_data, int tx_size);
int SerialPort_read(const int fd, char* rx_data);




#ifdef __cplusplus
}
#endif








