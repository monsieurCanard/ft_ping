#include <bits/types/struct_timeval.h>

#include "../includes/ping.h"

struct timeval add_timestamp(struct timeval* time1, struct timeval* time2)
{
    struct timeval temp;

    temp.tv_sec  = time1->tv_sec + time2->tv_sec;
    temp.tv_usec = time1->tv_usec + time2->tv_usec;
    while (temp.tv_usec >= 1000000)
    {
        temp.tv_sec += 1;
        temp.tv_usec %= 1000000;
    }
    return temp;
}

struct timeval sub_timestamp(struct timeval* time1, struct timeval* time2)
{
    struct timeval temp;

    temp.tv_sec  = time1->tv_sec - time2->tv_sec;
    temp.tv_usec = time1->tv_usec - time2->tv_usec;
    if (temp.tv_usec < 0)
    {
        temp.tv_sec -= 1;
        temp.tv_usec += 1000000;
    }

    while (temp.tv_usec >= 1000000)
    {
        temp.tv_sec += 1;
        temp.tv_usec %= 1000000;
    }
    return temp;
}

bool resend_packet(t_ping_client* client, struct timeval* now, struct timeval* time)
{
    int interval =
        (client->args.all_args & OPT_INTERVAL) ? client->args.interval : SECOND_PAUSE_BT_PINGS;

    struct timeval diff         = sub_timestamp(now, time);
    float          elapsed_time = diff.tv_sec + diff.tv_usec / 1000000.0;
    return (elapsed_time >= (float)interval);
}