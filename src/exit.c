#include "../includes/ping.h"

void exit_program(t_ping_client* client)
{
    if (client->fd > 0)
    {
        struct timeval end_time;
        gettimeofday(&end_time, NULL);
        client->counter.transmitted =
            client->counter.received + client->counter.lost + client->counter.error;

        double total_time = (end_time.tv_sec - client->start_time->tv_sec) * 1000.0 +
                            (end_time.tv_usec - client->start_time->tv_usec) / 1000.0;

        double mdev = (client->counter.transmitted > 0)
                          ? sqrt(client->rtt.delta / client->counter.transmitted)
                          : 0.0;

        double success_rate = client->counter.transmitted == 0
                                  ? 0.0
                                  : ((client->counter.transmitted - client->counter.received) /
                                     (double)client->counter.transmitted) *
                                        100.0;

        print_ping_infos(client, total_time, success_rate, mdev);
        close(client->fd);
    }
    if (client->start_time)
        free(client->start_time);

    if (client->packet)
        free(client->packet);

    exit(client->status);
}