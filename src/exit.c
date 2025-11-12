#include "../includes/ping.h"

void exit_program(t_ping_client* client)
{
    int msg_transmitted = 0;
    if (client->fd > 0)
    {
        msg_transmitted = client->counter.received + client->counter.lost + client->counter.error;

        double mdev =
            (msg_transmitted > 0) ? sqrt(client->time_stats.delta / msg_transmitted) : 0.0;

        double success_rate =
            msg_transmitted == 0
                ? 0.0
                : ((msg_transmitted - client->counter.received) / (double)msg_transmitted) * 100.0;

        print_ping_infos(client, success_rate, mdev, msg_transmitted);
        close(client->fd);
    }

    if (client->packets)
        free(client->packets);

    exit(client->status);
}