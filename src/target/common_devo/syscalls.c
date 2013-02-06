/*
 This project is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Deviation is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Deviation.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <libopencm3/stm32/usart.h>

#include "petit_fat.h"
#include "common.h"

//#define dbgprintf(args...) printf(args)
#define dbgprintf(args...) 
static DIR   dir;
#ifdef MEDIA_DRIVE
static FATFS fat[3];
static bool file_open[3]= {false, false, false};
#else
static FATFS fat[2];
static bool file_open[2]= {false, false};
#endif

extern u8 _drive_num;

extern void init_err_handler();

int FS_Mount();
void FS_Unmount();

int _open_r (struct _reent *r, const char *file, int flags, int mode);
int _close_r (struct _reent *r, int fd);


int _read_r (struct _reent *r, int fd, char * ptr, int len);
int _write_r (struct _reent *r, int fd, char * ptr, int len);
int _lseek_r (struct _reent *r, int fd, int ptr, int dir);


caddr_t _sbrk_r (struct _reent *r, int incr);
int _fstat_r (struct _reent *r, int fd, struct stat * st);
int _isatty_r(struct _reent *r, int fd);

int FS_Mount()
{
    int res = pf_mount(&fat[0]);
    dbgprintf("Mount: %d\n", res);
#ifdef MEDIA_DRIVE
    _drive_num = 1;
    int res2 = pf_mount(&fat[2]);
    (void)res2;
    dbgprintf("Mount2: %d\n", res2);
    fat[1] = fat[2];
    _drive_num = 0;
#else
    fat[1] = fat[0];
#endif
    if (res == FR_OK) {
        init_err_handler();
        pf_switchfile(&fat[0]);
    }
    return (res == FR_OK);
}

void FS_Unmount()
{
    pf_mount(0);
}

int FS_OpenDir(const char *path)
{
    if (strncmp(path, "media", 5) == 0) {
        pf_switchfile(&fat[2]);
        _drive_num = 1;
    } else {
        pf_switchfile(&fat[0]);
        _drive_num = 0;
    }
    FRESULT res = pf_opendir(&dir, path);
    dbgprintf("Opendir: %d\n", res);
    return (res == FR_OK);
}

/* Return:
   0 : Error
   1 : File
   2 : Dir
*/
int FS_ReadDir(char *path)
{
    FILINFO fi;
    if (pf_readdir(&dir, &fi) != FR_OK || ! fi.fname[0])
        return 0;
    dbgprintf("Read: %s %d\n", fi.fname, fi.fattrib);
    strncpy(path, fi.fname, 13);
    return fi.fattrib & AM_DIR ? 2 : 1;
}

void FS_CloseDir()
{
}

int _open_r (struct _reent *r, const char *file, int flags, int mode) {
    (void)r;
    (void)flags;
    (void)mode;

    int fd;
#ifdef MEDIA_DRIVE
    if (strncmp(file, "media/", 6) == 0) {
        _drive_num = 1;
        if (strcmp(file + strlen(file) - 4, ".fon") == 0 || strncmp(file, "protocol/", 9)== 0) {
            fd = 4;
        } else {
            fd = 5;
        }
    } else {
        _drive_num = 0;
        fd = 3;
    }
#else
    fd = (strcmp(file + strlen(file) - 4, ".fon") == 0) || strncmp(file, "protocol/", 9)== 0 ? 4 : 3;
#endif
    if(file_open[fd-3]) {
        dbgprintf("_open_r: file already open.\n");
        return -1;
    } else {
        pf_switchfile(&fat[fd-3]);
        int res=pf_open(file);
        if(res==FR_OK) {
            dbgprintf("_open_r: pf_open (%s) flags: %d, mode: %d ok\r\n", file, flags, mode);
            if (flags & O_CREAT)
                pf_maximize_file_size();
            file_open[fd-3]=true;
            return fd;  
        } else {
            dbgprintf("_open_r: pf_open failed: %d\r\n", res);
            return -1;
        }
    }
}

int _close_r (struct _reent *r, int fd) {
    (void)r;
    if(fd>2) {
       file_open[fd-3]=false;
       dbgprintf("_close_r: file closed.\r\n");
    }
    return 0;
}

int _read_r (struct _reent *r, int fd, char * ptr, int len)
{
    (void)r;
    if(fd>2 && file_open[fd-3]) {
#ifdef MEDIA_DRIVE
        _drive_num = fd == 3 ? 0 : 1;
#endif
        if(len <= 0xffff) {
            WORD bytes_read;
            pf_switchfile(&fat[fd-3]);
            int res=pf_read(ptr, len, &bytes_read);
            dbgprintf("_read_r: len %d, bytes_read %d, result %d\r\n", len, bytes_read, res); 
            if(res==FR_OK) return bytes_read;
        }
    }    

    errno=EINVAL;
    return -1;
}

int _write_r (struct _reent *r, int fd, char * ptr, int len)
{  
    (void)r;
    if(fd==1 || fd==2) {
        int index;

        if (0 == (USART_CR1(USART1) & USART_CR1_UE))
            return len; //Don't send if USART is disabled

        for(index=0; index<len; index++) {
            if (ptr[index] == '\n') {
                usart_send_blocking(USART1,'\r');
            }  
            usart_send_blocking(USART1, ptr[index]);
        }    
        return len;
    } else if(fd>2) {
        if(file_open[fd-3]) {
#ifdef MEDIA_DRIVE
            _drive_num = fd == 3 ? 0 : 1;
#endif
            WORD bytes_written;
            pf_switchfile(&fat[fd-3]);
            int res=pf_write(ptr, len, &bytes_written);
            dbgprintf("_write_r: len %d, bytes_written %d, result %d\r\n",len, bytes_written, res);
            if(res==FR_OK) return bytes_written;
        }
    }
    errno=EINVAL;
    return -1;
}


int _lseek_r (struct _reent *r, int fd, int ptr, int dir)
{
    (void)r;
    
    if(fd>2 && file_open[fd-3]) {
#ifdef MEDIA_DRIVE
        _drive_num = fd == 3 ? 0 : 1;
#endif
        if(dir==SEEK_CUR) {
            ptr += fat[fd-3].fptr;
        } else if (dir==SEEK_END) {
            ptr += fat[fd-3].fsize;
        }
        pf_switchfile(&fat[fd-3]);
        int res=pf_lseek(ptr);
        if(res==FR_OK) {
           return fat[fd-3].fptr;
        }
    }
    errno=EINVAL;
    return -1;
}



int _fstat_r (struct _reent *r, int fd, struct stat * st)
{
    (void)r;
    (void)fd;
    (void)st;
    errno=EINVAL;
    return -1;
}

int _isatty_r(struct _reent *r, int fd)
{
    (void)r;
    if(fd<3) return 1;
    return 0;  
}



register char * stack_ptr asm ("sp");

caddr_t _sbrk_r (struct _reent *r, int incr)
{
  extern char   end asm ("end"); /* Defined by the linker.  */
  static char * heap_end;
  char *        prev_heap_end;

   r=r;

  if (heap_end == NULL)
    heap_end = & end;
  
  prev_heap_end = heap_end;
  
  if (heap_end + incr > stack_ptr)
  {
    errno = ENOMEM;
    return (caddr_t) -1;
  }
  
  heap_end += incr;

  return (caddr_t) prev_heap_end;
}
