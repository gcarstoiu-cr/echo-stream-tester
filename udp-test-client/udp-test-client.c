#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define DEBUG_USE_LOCAL_IP  (0)

#define EXAMPLE_DEFAULT_PORT        (5570)
#if (DEBUG_USE_LOCAL_IP)
#define EXAMPLE_DEFAULT_SERVER_IP   "192.168.0.100"
#else
#define EXAMPLE_DEFAULT_SERVER_IP   "86.127.191.87"
#endif
#define EXAMPLE_DEFAULT_PKTSIZE     (1440)

#define SEND_THROUGHPUT_BPS         (5*1024*1024)

static int32_t mysocket = -1;
static struct sockaddr_in remote_addr;
// static uint32_t socklen;
static uint8_t databuff[EXAMPLE_DEFAULT_PKTSIZE] = {0};

void InitUdpSocket(void)
{
    mysocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (mysocket < 0)
    {
        //
    }
    /*for client remote_addr is also server_addr*/
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(EXAMPLE_DEFAULT_PORT);
    remote_addr.sin_addr.s_addr = inet_addr(EXAMPLE_DEFAULT_SERVER_IP);
}

int main(void)
{
    InitUdpSocket();

    int32_t sendCounter = 100;
    int32_t sentSize = 0;
    // Thread sleep time in us, calculated to achieve the desired throughput in bits/sec.
    uint32_t sleepUs = 1000000u / ((SEND_THROUGHPUT_BPS / 8u) / sizeof(databuff));
    while (true)
    {
        if (sendCounter > 0)
        {
            sentSize = sendto(mysocket, databuff, sizeof(databuff), 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
            sendCounter -= 1;
        }

        usleep(sleepUs);
    }
    return 0;
}
