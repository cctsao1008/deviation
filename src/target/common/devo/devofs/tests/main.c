#include "devofs.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char *image_file;
static DIR   dir;
int FS_OpenDir(const char *path)
{
    
    FRESULT res = pf_opendir(&dir, path);
    printf("Opendir: %d\n", res);
    return (res == FR_OK);
}

int FS_ReadDir(char *path)
{
    FILINFO fi;
    if (pf_readdir(&dir, &fi) != FR_OK || ! fi.fname[0])
        return 0;
    printf("Read: %s %d\n", fi.fname, fi.fattrib);
    strncpy(path, fi.fname, 13);
    return fi.fattrib & AM_DIR ? 2 : 1;
}

int main(int argc, char *argv[]) {
	FATFS fat, fat1;
	FILE *fh;
	unsigned char data[1024 * 1024];
        unsigned char *ptr = data;
        unsigned int len;
	int res;
        if (argc != 2) {
            printf("%s <devofs image>\n", argv[1]);
            exit(0);
        }
        image_file = argv[1];
	res = pf_mount(&fat);
	printf("pf_mount: %d\n", res);
        FS_OpenDir("media");
        while((res = FS_ReadDir(data))) {
            fat1 = fat;
            printf("ReadDir: %d: %s\n", res, data);
            char filename[256];
            sprintf(filename, "media/%s", data);
            res = pf_open(filename, 0);
            printf("pf_open: %d\n", res);
            if (res) {
                fat= fat1;
                continue;
            }
            char *ptr = data;
            while(1) {
                u16 bytes;
                res = pf_read(ptr, 1024, &bytes);
                printf("pf_read: %d %dbytes\n", res, bytes);
                ptr += bytes;
                if(res || bytes != 1024)
                    break;
            }
            *ptr = 0;
            printf("%s\n", data);
            fat= fat1;
        }

#if 0
	res = pf_open("model/model1.ini");
	printf("pf_open: %d\n", res);
	pf_maximize_file_size();
	res = pf_open("bmp/devo8.bmp");
	printf("pf_open: %d\n", res);
        while(1) {
            WORD b;
            res = pf_read(ptr, 0x1000, &b);
            len += b;
            if(res || b != 0x1000)
                break;
            ptr+= 0x1000;
        }
	printf("pf_read: %d\n", res);
	printf("Read %d\n", len);
	fh = fopen("out.bmp", "w");
	fwrite(data, len, 1, fh);
	fclose(fh);
#endif
	return 0;
}
        
