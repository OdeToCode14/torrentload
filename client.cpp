#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <thread>

/*
bool readdata(SOCKET sock, void *buf, int buflen)
{
    unsigned char *pbuf = (unsigned char *) buf;

    while (buflen > 0)
    {
        int num = recv(sock, pbuf, buflen, 0);
        if (num == SOCKET_ERROR)
        {
            if (WSAGetLastError() == WSAEWOULDBLOCK)
            {
                // optional: use select() to check for timeout to fail the read
                continue;
            }
            return false;
        }
        else if (num == 0)
            return false;

        pbuf += num;
        buflen -= num;
    }

    return true;
}

bool readlong(SOCKET sock, long *value)
{
    if (!readdata(sock, value, sizeof(value)))
        return false;
    *value = ntohl(*value);
    return true;
}

bool readfile(SOCKET sock, FILE *f)
{
    long filesize;
    if (!readlong(sock, &filesize))
        return false;
    if (filesize > 0)
    {
        char buffer[1024];
        do
        {
            int num = min(filesize, sizeof(buffer));
            if (!readdata(sock, buffer, num))
                return false;
            int offset = 0;
            do
            {
                size_t written = fwrite(&buffer[offset], 1, num-offset, f);
                if (written < 1)
                    return false;
                offset += written;
            }
            while (offset < num);
            filesize -= num;
        }
        while (filesize > 0);
    }
    return true;
}

*/

int main()
{
int sockfd;
int len;
struct sockaddr_in address;
int result;
char ch = 'A';

sockfd = socket(AF_INET, SOCK_STREAM, 0);

address.sin_family = AF_INET;
address.sin_addr.s_addr = inet_addr("127.0.0.1");
address.sin_port = 9735;
len = sizeof(address);

result = connect(sockfd, (struct sockaddr *)&address, len);
if(result == -1) {
perror("oops: client1");
exit(1);
}
/*
write(sockfd, &ch, 1);
read(sockfd, &ch, 1);
printf("char from server = %c\n", ch);
*/
/*
size_t datasize;
FILE* fd = fopen("file", "wb");
char buffer[256];
int BUFFER_SIZE=256;
while (datasize)
{
    datasize = recv(sockfd, buffer, BUFFER_SIZE, 0);
    fwrite(&buffer, 1, datasize, fd);
}
fclose(fd);
*/
char *buffer="hi";
send(sockfd, buffer, 3, 0);
close(sockfd);
exit(0);
}