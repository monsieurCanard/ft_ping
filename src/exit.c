#include "../includes/ping.h"

void exit_program(t_ping_client* client)
{
    if (client->fd > 0)
    {
        client->counter.transmitted =
            client->counter.received + client->counter.lost + client->counter.error;

        double mdev = (client->counter.transmitted > 0)
                          ? sqrt(client->time_stats.delta / client->counter.transmitted)
                          : 0.0;

        double success_rate = client->counter.transmitted == 0
                                  ? 0.0
                                  : ((client->counter.transmitted - client->counter.received) /
                                     (double)client->counter.transmitted) *
                                        100.0;

        print_ping_infos(client, success_rate, mdev);
        close(client->fd);
    }

    if (client->packet)
        free(client->packet);

    exit(client->status);
}