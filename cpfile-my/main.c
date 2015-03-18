#include <stdio.h>
#include <stdlib.h>

#define BUF_SIZE 256

int cpfile(char* fs, char* fd)
{
    FILE *source_file=NULL, *dest_file=NULL;
    size_t nbyte_in,nbyte_out,len=0;
    char data[BUF_SIZE];

    printf("--- cp file! ---\n");

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

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        printf("\n------ERROR:\nUsage: %s source_file dest_file \n",argv[0]);
        return 0;
    }

    cpfile(argv[1],argv[2]);
    return 0;
}
