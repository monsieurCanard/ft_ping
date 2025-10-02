#include "../includes/ping.h"

int create_client(t_ping_client* client, struct sockaddr_in* sockaddr, char* address)
{
    client->infos = gethostbyname(address);
    if (!client->infos)
    {
        fprintf(stderr, "Could not resolve hostname %s\n", address);
        return (ERROR);
    }

    client->ip = inet_ntoa(*(struct in_addr*)client->infos->h_addr_list[0]);

    client->fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (client->fd < 0)
    {
        perror("Creation Socket");
        return (ERROR);
    }

    // Gestion du timeout sur la reception
    struct timeval timeout;
    timeout.tv_sec  = TIMEOUT_SEC;
    timeout.tv_usec = TIMEOUT_USEC;
    setsockopt(client->fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    // int ttl_value = 1;
    // setsockopt(client->fd, IPPROTO_IP, IP_TTL, &ttl_value, sizeof(ttl_value));

    // Initialisation du delai entre chaque ping
    client->delay_bt_pings.tv_sec  = SECOND_PAUSE_BT_PINGS;
    client->delay_bt_pings.tv_nsec = NANOSECOND_PAUSE_BT_PINGS;

    memset(sockaddr, 0, sizeof(*sockaddr));
    sockaddr->sin_family      = AF_INET;
    sockaddr->sin_port        = 0;
    sockaddr->sin_addr.s_addr = inet_addr(client->ip);

    client->seq                 = 0;
    client->counter.transmitted = 0;
    client->counter.received    = 0;
    client->counter.lost        = 0;
    client->counter.error       = 0;
    client->rtt.min             = -1;
    client->rtt.max             = -1;
    client->rtt.average         = 0;
    client->rtt.mdev            = 0;
    client->rtt.total           = 0;
    client->rtt.delta           = 0;
    client->status              = EXIT_SUCCESS;

    client->start_time = malloc(sizeof(struct timeval));
    if (!client->start_time)
    {
        perror("Malloc: ");
        return (ERROR);
    }
    gettimeofday(client->start_time, NULL);

    client->recv_time = malloc(sizeof(struct timeval));
    if (!client->recv_time)
    {
        perror("Malloc: ");
        return (ERROR);
    }

    return (SUCCESS);
}

void update_time_stats(t_rtt* rtt, double new_rtt, int count)
{
    if (rtt->min == -1 || new_rtt < rtt->min)
    {
        rtt->min = new_rtt;
    }

    if (rtt->max == -1 || new_rtt > rtt->max)
    {
        rtt->max = new_rtt;
    }

    rtt->total += new_rtt;
    double delta = new_rtt - rtt->average;
    rtt->average += delta / count;
    rtt->delta += delta * (new_rtt - rtt->average);
}