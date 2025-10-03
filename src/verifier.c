#include "../includes/ping.h"

extern t_ping_client client;

void verify_packet(t_icmp_packet* packet)
{
    fprintf(stderr, "Verifying packets for timeouts...\n");
    struct timeval now;
    gettimeofday(&now, NULL);

    double now_time = now.tv_sec * 1000000 + now.tv_usec;
    double packet_time;
    for (int i = 0; i < client.seq; i++)
    {
        if (packet[i].received == false)
        {
            packet_time = packet[i].send_time.tv_sec * 1000000 + packet[i].send_time.tv_usec;
            if (now_time - packet_time > TIMEOUT_SEC * 1000000 + TIMEOUT_USEC)
            {
                // fprintf(stderr, "Request timeout for icmp_seq %d\n", i);
                client.counter.lost++;
                packet[i].received = -1; // Mark as processed to avoid repeated messages
            }
        }
    }
}