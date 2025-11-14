#include "../includes/ping.h"

void exit_program(t_ping_client* client)
{
    int msg_transmitted = client->counter.transmitted;
    if (client->fd > 0)
    {

        double success_rate =
            msg_transmitted == 0
                ? 0.0
                : ((msg_transmitted - client->counter.received) / (double)msg_transmitted) * 100.0;

        print_ping_final_stats(client, success_rate);
        close(client->fd);
    }

    if (client->packets)
        free(client->packets);

    exit(client->status);
}