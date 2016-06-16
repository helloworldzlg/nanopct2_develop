#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>


//14byte�ļ�ͷ
typedef struct
{
	char cfType[2];  //�ļ����ͣ�"BM"(0x4D42)         
	long cfSize;     //�ļ���С���ֽڣ�         
	long cfReserved; //������ֵΪ0     
	long cfoffBits;  //������������ļ�ͷ��ƫ�������ֽڣ�    
}__attribute__((packed)) BITMAPFILEHEADER;
//__attribute__((packed))�������Ǹ��߱�����ȡ���ṹ�ڱ�������е��Ż����룬����ʵ��ռ���ֽ������ж���

//40byte��Ϣͷ
typedef struct
{
	char ciSize[4];           //BITMAPFILEHEADER��ռ���ֽ���
	long ciWidth;       
	long ciHeight;       
	char ciPlanes[2];         //Ŀ���豸��λƽ������ֵΪ1
	int ciBitCount;           //ÿ�����ص�λ��
	char ciCompress[4];       //ѹ��˵��
	char ciSizeImage[4];      //���ֽڱ�ʾ��ͼ���С�������ݱ�����4�ı���    
	char ciXPelsPerMeter[4];  //Ŀ���豸��ˮƽ������/��
	char ciYPelsPerMeter[4];  //Ŀ���豸�Ĵ�ֱ������/��
	char ciClrUsed[4];        //λͼʹ�õ�ɫ�����ɫ��   
	char ciClrImportant[4];   //ָ����Ҫ����ɫ�����������ֵ������ɫ��ʱ�����ߵ���0ʱ������ʾ������ɫ��һ����Ҫ
}__attribute__((packed)) BITMAPINFOHEADER;

typedef struct
{
	unsigned int red   :8;
	unsigned int green :8;
	unsigned int blue  :8;
}__attribute__((packed)) PIXEL;//��ɫģʽ��RGB565

BITMAPFILEHEADER FileHead;
BITMAPINFOHEADER InfoHead;

static char *fbp = 0;
static int xres = 0;
static int yres = 0;
static int bits_per_pixel = 0;

int show_bmp();

int main ( int argc, char *argv[] )
{
	int fbfd = 0;
	int i,j;
	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;
	long int screensize = 0;
	
	//����ʾ�豸
	fbfd = open("/dev/fb0", O_RDWR);
	if (!fbfd)
	{
		printf("Error: cannot open framebuffer device.\n");
		exit(1);
	}
	
	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo))
	{
		printf("Error��reading fixed information.\n");
		exit(2);
	}
	
	if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo))
	{
		printf("Error: reading variable information.\n");
		exit(3);
	}
	
	printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel );
	xres = vinfo.xres;
	yres = vinfo.yres;
	bits_per_pixel = vinfo.bits_per_pixel;
	
	//������Ļ���ܴ�С���ֽڣ�
	screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
	printf("screensize=%d\n",screensize);
	
	//�ڴ�ӳ��
	fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
	if ((int)fbp == -1)
	{
		printf("Error: failed to map framebuffer device to memory.\n");
		exit(4);
	}
	printf("sizeof header=%d\n", sizeof(BITMAPFILEHEADER));
	
	printf("into show_bmp function\n");
	
	show_bmp();
	
	munmap(fbp, screensize);
	
	close(fbfd);
	return 0;
}

int show_bmp()
{
	FILE *fp;
	int rc;
	int line_x, line_y;
	long int location = 0, BytesPerLine = 0;
	char tmp[1024*15];
	
	fp = fopen( "./eye.bmp", "rb");
	if (fp == NULL)
	{
		return( -1 );
	}
	
	rc = fread( &FileHead, sizeof(BITMAPFILEHEADER), 1, fp );
	if ( rc != 1)
	{
		printf("read header error!\n");
		fclose( fp );
		return( -2 );
	}

	if (memcmp(FileHead.cfType, "BM", 2) != 0)
	{
		printf("it's not a BMP file\n");
		fclose( fp );
		return( -3 );
	}

	rc = fread( (char *)&InfoHead, sizeof(BITMAPINFOHEADER), 1, fp );
	if ( rc != 1)
	{
		printf("read infoheader error!\n");
		fclose( fp );
		return( -4 );
	}
	
	//��ת��������
	fseek(fp, FileHead.cfoffBits, SEEK_SET);
	
	//ÿ���ֽ���
	BytesPerLine = (InfoHead.ciWidth * InfoHead.ciBitCount + 31) / 32 * 4;   
	
	line_x = line_y = 0;
	
	//��framebuffer��дBMPͼƬ
	while (!feof(fp))
	{
		PIXEL pix;
		unsigned int tmp;
		rc = fread( (char *)&pix, 1, sizeof(unsigned int), fp);
		if (rc != sizeof(unsigned int))
		{ 
			break; 
		}
		
		location = line_x * bits_per_pixel / 8 + (InfoHead.ciHeight - line_y - 1) * xres * bits_per_pixel / 8;
		
		//��ʾÿһ������
		tmp = (pix.red << 16) | (pix.green << 8) | (pix.blue << 0);
		
		*((unsigned int*)(fbp + location)) = tmp;
		
		line_x++;
		if (line_x == InfoHead.ciWidth )
		{
			line_x = 0;
			line_y++;
			
			if (line_y == InfoHead.ciHeight)
			{
				break;
			}
		}
	}
	
	fclose( fp );
	return( 0 );
}