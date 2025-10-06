#include "../includes/ping.h"

void verify_packet(t_ping_client* client)
{
    // fprintf(stderr, "Verifying packets for timeouts...\n");
    struct timeval now;
    gettimeofday(&now, NULL);

    double now_time = now.tv_sec * 1000000 + now.tv_usec;
    double packet_time;
    for (int i = 1; i < client->seq; i++)
    {
        if (client->packet[i].received == false)
        {
            packet_time =
                client->packet[i].send_time.tv_sec * 1000000 + client->packet[i].send_time.tv_usec;
            if (now_time - packet_time > TIMEOUT_SEC * 1000000 + TIMEOUT_USEC)
            {
                fprintf(stderr, "Request timeout for icmp_seq %d\n", i);
                client->counter.lost++;
                client->packet[i].received = -1; // Mark as processed to avoid repeated messages
            }
        }
    }
}