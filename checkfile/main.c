#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#define _GNU_SOURCE
#include <getopt.h>

char *g_fn = "mcu.bin";
char *g_fn_check = "mcu-check.bin";
unsigned short checksum = -1;
int ret  = -1;

//LPR-------------------------------------------------
#define LPR_PAGE_SIZE      0x00000200     /* Page size */
#define PACKET_SIZE	64

#define MCU_BIN_VER_POS_REVERT 8
#define MCU_BIN_CHKSUM_POS_REVERT 2

#define T0 printf

#if 1
#define ttt short
#else
#define ttt int
#endif

//LPR---------------------------------------------------------
unsigned int   g_packno = 1;
unsigned char  g_sendbuf[PACKET_SIZE];
unsigned long  g_update_aprom_pos = 0;
unsigned short g_cksum = 0;
int            g_fd = -1;
int            g_fd_store = -1;
unsigned long  g_totallen = 0;
char           g_bin_ver[5];

unsigned int bin_chksum = -1;

unsigned ttt lpr_checksum (unsigned char *buf, int len)
{
    int i;
    unsigned ttt c=0;
    static int count=0;
    count++;
    T0("--buf:0x%x,len:0x%x--",(unsigned int)buf,len);


    for (i=0; i < len; i++) {
        c += buf[i];
    }
    T0("--- lpr_checksum return c : (%2d: 0x%4x)",count,c);
    return (c);
}

static unsigned ttt lpr_cal_checksum(int fd, uint32_t len)
{
	int i;
	int ccc=0;
	unsigned ttt lcksum = 0;
    unsigned char buf[LPR_PAGE_SIZE];

    T0("[---lpr_cal_checksum---] fd:%d, len:0x%x \n",fd,len);

    memset(buf, 0, sizeof(buf));
	lseek(fd, 0, SEEK_SET);
	for(i = 0; i < len; i+=LPR_PAGE_SIZE)
	{
	    memset(buf, 0, sizeof(buf));
		if(len - i >= LPR_PAGE_SIZE)
        {
            ccc=read(fd, buf, LPR_PAGE_SIZE);
            T0("---ccc = 0x%4x,%x---",ccc,LPR_PAGE_SIZE);
			lcksum += lpr_checksum(buf, LPR_PAGE_SIZE);
        }
		else
		{
		    read(fd, buf, len - i);
		    lcksum += lpr_checksum(buf, len - i);
		    T0("[---lpr_cal_checksum---] len - i : 0x%x ,(%d)\n",len - i,len - i);
        }

        T0("------ lcksum: 0x%x \n",lcksum);
	}

    T0("[---lpr_cal_checksum---] return lcksum: 0x%x \n",lcksum);
    return lcksum;

}

void count_checksum()
{

   // g_fd = open("mcu.bin", O_RDONLY);

    g_fd = open(g_fn, O_RDONLY|O_BINARY);
    g_fd_store = open(g_fn_check, O_WRONLY|O_BINARY);

    printf("g_fd:%d \n",g_fd);
    if(g_fd < 0)
    {
//        printf("\n Cannot open this file ! g_fd = %d \n",g_fd);
        perror(g_fn);
        exit(2);
    }

   struct stat statBuf;
    if(stat("mcu.bin", &statBuf) == 0)
    {
        g_totallen = (unsigned long)statBuf.st_size;
    }
    T0("\n---g_totallen: 0x%x\n",g_totallen);

     if(g_totallen >= MCU_BIN_VER_POS_REVERT)
        {
            //read bin version
            unsigned long bin_ver_pos = g_totallen - MCU_BIN_VER_POS_REVERT;
            lseek(g_fd, 0, SEEK_SET);
            lseek(g_fd, bin_ver_pos, SEEK_SET);
            unsigned char hex[2];
            memset(hex, 0, sizeof(hex));
            if( read(g_fd, hex, sizeof(hex)) == sizeof(hex))
            {
                sprintf(g_bin_ver, "%02x%02x", hex[0], hex[1]);
                T0("g_totallen 0x%lx, g_bin_ver %s\n", g_totallen, g_bin_ver);
            }

            //read bin file check sum

            unsigned long chksum_pos = g_totallen - MCU_BIN_CHKSUM_POS_REVERT;
            unsigned short bin_file_cksum=0;
            lseek(g_fd, 0, SEEK_SET);
            lseek(g_fd, chksum_pos, SEEK_SET);
            memset(hex, 0, sizeof(hex));
            if( read(g_fd, hex, sizeof(hex)) == sizeof(hex))
            {
                bin_file_cksum = hex[0];
                bin_file_cksum <<=8;
                bin_file_cksum += hex[1];
                T0("bin_file_cksum 0x%x\n", bin_file_cksum);
            }

            // check bin file

            bin_chksum = lpr_cal_checksum(g_fd, (g_totallen - MCU_BIN_CHKSUM_POS_REVERT));
            T0("bin_chksum: (0x%x)\n", bin_chksum);
            if(bin_chksum == bin_file_cksum)
            {
                lseek(g_fd, 0, SEEK_SET);
                T0("\n%s is valid file, can use it! (0x%x)\n", "mcu.bin", bin_chksum);
            }
            else
            {
                T0("\n%s is invalid file, do not use it! (0x%x)\n", "mcu.bin", bin_chksum);
                //lpr_uninit();
                //exit(2);
            }
        }

    T0("\n\n--- close file ! ---\n\n");
    close(g_fd);

}

int addchecksum(char* fc, unsigned short checknum)
{
    FILE *fp=NULL;
    int len=0;
    char buf[2];
    buf[1] = (char)checknum & 0xff;
    buf[0] = (char)((checknum >>8)&0xff);

    if((fp=fopen(fc,"rb+"))==NULL)
    {
        perror(fc);
        return 1;
    }

    fseek(fp,0,SEEK_END);
    len = ftell(fp);
    printf("file len:0x%x(%d)\n",len,len);

    fseek(fp,-2,SEEK_END);
    len = ftell(fp);
    printf("now len:0x%x(%d)\n",len,len);

    fwrite(buf,1,2,fp);

    len = ftell(fp);
    printf("now len:0x%x(%d)\n",len,len);


    fclose(fp);
    return 0;
}

void add_checksum()
{
    T0("\n\n--- add_checksum 11111111111---(bin_chksum:0x%x)\n\n",bin_chksum);
    addchecksum(g_fn_check,bin_chksum);
}

#define BUF_SIZE 256

int cpfile(char* fs, char* fd)
{
    FILE *source_file=NULL, *dest_file=NULL;
    size_t nbyte_in,nbyte_out,len=0;
    char data[BUF_SIZE];

    printf("--- copy file! ---\n");

    if((source_file=fopen(fs,"rb")) == NULL)
    {
        perror(fs);
        goto err1;
    }

    if((dest_file=fopen(fd,"wb")) == NULL)
    {
        perror(fd);
        goto err2;
    }

    while((nbyte_in=fread(data,1,BUF_SIZE,source_file))>0)
    {
        nbyte_out  = fwrite(data,1,nbyte_in,dest_file);
        if(nbyte_out != nbyte_in)
        {
            printf("copy file failed!(%d,%d)\n",nbyte_in,nbyte_out);
            return 1;
        }
        len += nbyte_out;
        printf("copying file... %d bytes copied!(%d,%d)\n",len,nbyte_in,nbyte_out);
    }

    fclose(dest_file);
err2:
    fclose(source_file);
err1:
    return 0;
}

void checkfile()
{
    count_checksum();
    cpfile(g_fn,g_fn_check);
    add_checksum();
}

int main(int argc, char* argv[])
{
    printf("\n--- checkfile ---\n\n");
    argc =2;
    if(argc != 2)
    {
        printf("Usage: %s file\n",argv[0]);
        return 1;
    }

//    g_fn = argv[1];
    checkfile();

    return 0;
}
