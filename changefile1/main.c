#include <stdio.h>
#include <stdlib.h>


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
       //  nbyte_out  = fwrite(data,1,BUF_SIZE,dest_file);

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

int addchecksum(char* fc, unsigned short checknum)
{
    FILE *fp=NULL;
    int len=0;
    char buf[2];
//    buf[0]=(char)0x12;
//    buf[1]=(char)0x34;
    buf[1] = (char)checknum & 0xff;
    buf[0] = (char)((checknum >>8)&0xff);
//    unsigned char buf[2] = {0x67,0x89};
//    unsigned short buf[] ={0x1234};

    if((fp=fopen(fc,"rb+"))==NULL)
    {
        perror(fc);
        return 1;
    }
    rewind(fp);
    len = ftell(fp);
    printf("len:0x%x(%d)\n",len,len);
    fseek(fp,0,SEEK_END);
    len = ftell(fp);
    printf("len:0x%x(%d)\n",len,len);

    fseek(fp,-2,SEEK_END);
    len = ftell(fp);
    printf("len:0x%x(%d)\n",len,len);

    fwrite(buf,1,2,fp);

    len = ftell(fp);
    printf("len:0x%x(%d)\n",len,len);



    fclose(fp);
    return 0;
}

int changefile(char *fn)
{
    char *fo = "mcu-check.bin";
    unsigned short checksum = 0x1234;

    printf("---changefile: %s\n",fn);

    //cp file
    cpfile(fn,fo);

    //change file
    addchecksum(fo,checksum);

    return 0;
}
int main()
{
    FILE *fp=NULL;
    char fname[]="mcu.bin";

    changefile(fname);

    return 0;
}
