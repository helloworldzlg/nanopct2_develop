

struct fb_dev
{
        //for frame buffer
        int fb;
        void *fb_mem;   //frame buffer mmap
        int fb_width, fb_height, fb_line_len, fb_size;
        int fb_bpp;
} fbdev;

//�õ�framebuffer�ĳ������λ���ɹ��򷵻�0��ʧ�ܷ��أ�1 
int fb_stat(int fd)
{
        struct fb_fix_screeninfo fb_finfo;
        struct fb_var_screeninfo fb_vinfo;

        if (ioctl(fd, FBIOGET_FSCREENINFO, &fb_finfo))
        {
                perror(__func__);
                return (-1);
        }

        if (ioctl(fd, FBIOGET_VSCREENINFO, &fb_vinfo))
        {
                perror(__func__);
                return (-1);
        }

        fbdev.fb_width = fb_vinfo.xres;
        fbdev.fb_height = fb_vinfo.yres;
        fbdev.fb_bpp = fb_vinfo.bits_per_pixel;
        fbdev.fb_line_len = fb_finfo.line_length;
        fbdev.fb_size = fb_finfo.smem_len;

        return (0);
}

//ת��RGB888ΪRGB565����Ϊ��ǰLCD�ǲ��õ�RGB565��ʾ�ģ�
unsigned short RGB888toRGB565(unsigned char red, unsigned char green, unsigned char blue)
{
        unsigned short B = (blue >> 3) & 0x001F;
        unsigned short G = ((green >> 2) << 5) & 0x07E0;
        unsigned short R = ((red >> 3) << 11) & 0xF800;

        return (unsigned short) (R | G | B);
}

//�ͷ�framebuffer��ӳ��
int fb_munmap(void *start, size_t length)
{
        return (munmap(start, length));
}

//��ʾһ�����ص��ͼ��framebuffer��
int fb_pixel(void *fbmem, int width, int height, int x, int y, unsigned short color)
{
        if ((x > width) || (y > height))
                return (-1);

        unsigned short *dst = ((unsigned short *) fbmem + y * width + x);

        *dst = color;
        return 0;
}

int main(int argc, char **argv)
{
     int fb;
     FILE *infile;
     struct jpeg_decompress_struct cinfo;
     int x,y;
     unsigned char *buffer;
     char s[15];
     struct jpeg_error_mgr jerr;

      if ((fb = open("/dev/fb0", O_RDWR)) < 0)                        //���Կ��豸
      {
                perror(__func__);
                return (-1);
      }

        //��ȡframebuffer��״̬
        fb_stat(fb);                                                    //��ȡ�Կ������еĳ��������ʾλ��
        printf("frame buffer: %dx%d,  %dbpp, 0x%xbyte= %d\n",
                fbdev.fb_width, fbdev.fb_height, fbdev.fb_bpp, fbdev.fb_size, fbdev.fb_size);

        //ӳ��framebuffer�ĵ�ַ
        fbdev.fb_mem = mmap (NULL, fbdev.fb_size, PROT_READ|PROT_WRITE,MAP_SHARED,fb,0);

        if ((infile = fopen("lcd.jpg", "rb")) == NULL)
        {
                fprintf(stderr, "open %s failed\n", s);
                exit(-1);
        }
        ioctl(fb, FBIOBLANK,0);                        //��LCD����

        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_decompress(&cinfo);

        //����Ҫ��ѹ��Jpeg�ļ�infile
        jpeg_stdio_src(&cinfo, infile);

        //��ȡjpeg�ļ����ļ�ͷ
        jpeg_read_header(&cinfo, TRUE);

        //��ʼ��ѹJpeg�ļ�����ѹ�󽫷����scanline��������
        jpeg_start_decompress(&cinfo);

        buffer = (unsigned char *) malloc(cinfo.output_width
                                        * cinfo.output_components);
       	y = 0;
        while (cinfo.output_scanline < cinfo.output_height)
        {
                jpeg_read_scanlines(&cinfo, &buffer, 1);
                if(fbdev.fb_bpp == 16)
                {
                       unsigned short color;
                       for (x = 0; x < cinfo.output_width; x++)
                       {
                                color = RGB888toRGB565(buffer[x * 3],
                                                            buffer[x * 3 + 1], buffer[x * 3 + 2]);
                                fb_pixel(fbdev.fb_mem, fbdev.fb_width, fbdev.fb_height, x, y, color);
                       }
                 }
                 else if(fbdev.fb_bpp == 24)
                 {
                       memcpy((unsigned char *)fbdev.fb_mem + y * fbdev.fb_width * 3, buffer,
                                                        cinfo.output_width * cinfo.output_components);
                 }
                y++;
        }
        
        //���Jpeg���룬�ͷ�Jpeg�ļ�
        jpeg_finish_decompress(&cinfo);
       	jpeg_destroy_decompress(&cinfo);

       	//�ͷ�֡������
        free(buffer);

        //�ر�Jpeg�����ļ�
        fclose(infile);

       fb_munmap(fbdev.fb_mem, fbdev.fb_size);                                 //�ͷ�framebufferӳ��

       close(fb); 
}