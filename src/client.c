#include "../includes/ping.h"

static int resolve_host(t_ping_client* client, char* address)
{
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET; // IPv4
    hints.ai_socktype = SOCK_RAW;

    int status = getaddrinfo(address, NULL, &hints, &res);
    if (status != 0)
    {
        fprintf(stderr, "Could not resolve hostname %s: %s\n", address, gai_strerror(status));
        return (ERROR);
    }

    memcpy(&client->sockaddr, res->ai_addr, sizeof(struct sockaddr_in));
    freeaddrinfo(res);

    // Convertir l'adresse IP en chaîne de caractères pour l'affichage
    client->ip = inet_ntoa(client->sockaddr.sin_addr);

    return (SUCCESS);
}

static int create_socket(t_ping_client* client)
{
    client->fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (client->fd < 0)
    {
        perror("Creation Socket");
        return (ERROR);
    }

    if (client->args.all_args & OPT_TTL)
    {
        if (setsockopt(
                client->fd, IPPROTO_IP, IP_TTL, &client->args.ttl, sizeof(client->args.ttl)) < 0)
        {
            perror("Setsockopt TTL: ");
            return (ERROR);
        }
    }

    return (SUCCESS);
}

int create_client(t_ping_client* client, char* address)
{
    if (resolve_host(client, address) == ERROR)
        return (ERROR);

    if (create_socket(client) == ERROR)
        return (ERROR);

    client->delay_bt_pings.tv_sec = SECOND_PAUSE_BT_PINGS;
    client->time_stats.min        = -1;
    client->time_stats.max        = -1;

    client->name = address;

    client->packets = malloc(sizeof(t_icmp_packet) * MAX_PING_SAVES);
    if (!client->packets)
    {
        perror("Malloc: ");
        return (ERROR);
    }
    memset(client->packets, 0, sizeof(t_icmp_packet) * MAX_PING_SAVES);
    return (SUCCESS);
}

void update_client_time_stats(t_time_stats* time_stats, double new_rtt, int count)
{
    if (time_stats->min == -1 || new_rtt < time_stats->min)
    {
        time_stats->min = new_rtt;
    }

    if (time_stats->max == -1 || new_rtt > time_stats->max)
    {
        time_stats->max = new_rtt;
    }

    time_stats->total += new_rtt;

    double tmp_delta = new_rtt - time_stats->average;
    time_stats->average += tmp_delta / count;
    time_stats->delta += tmp_delta * (new_rtt - time_stats->average);
    time_stats->stddev = (count > 1) ? sqrt(time_stats->delta / (count - 1)) : 0.0;
}