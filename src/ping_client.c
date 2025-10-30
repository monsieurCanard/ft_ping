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

    // Sauvegarder l'adresse IP binaire pour les comparaisons
    client->target_addr = client->sockaddr.sin_addr.s_addr;

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

    // Gestion du timeout sur la reception
    struct timeval timeout;
    timeout.tv_sec  = (client->args.timeout != 0) ? client->args.timeout : TIMEOUT_SEC;
    timeout.tv_usec = TIMEOUT_USEC;

    if (setsockopt(client->fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
    {
        perror("Setsockopt options: ");
        return (ERROR);
    }

    // if (client->args.debug_level > 0)
    // {
    //     if (setsockopt(client->fd,
    //                    SOL_SOCKET,
    //                    SO_DEBUG,
    //                    &client->args.debug_level,
    //                    sizeof(client->args.debug_level)) < 0)
    //     {
    //         perror("Setsockopt debug: ");
    //         return (ERROR);
    //     }
    // }

    fprintf(stdout, "timeout = %d sec, %d usec\n", (int)timeout.tv_sec, (int)timeout.tv_usec);

    fprintf(stdout, "ttl = %d\n", (client->args.ttl != 0) ? client->args.ttl : 64);
    if (client->args.ttl != 0)
    {
        if (setsockopt(
                client->fd, IPPROTO_IP, IP_TTL, &client->args.ttl, sizeof(client->args.ttl)) < 0)
        {
            perror("Setsockopt TTL: ");
            return (ERROR);
        }
    }
    // int ttl_value = 1;
    // setsockopt(client->fd, IPPROTO_IP, IP_TTL, &ttl_value, sizeof(ttl_value));

    return (SUCCESS);
}

int create_client(t_ping_client* client, char* address)
{

    memset(client, 0, sizeof(t_ping_client));

    if (resolve_host(client, address) == ERROR)
        return (ERROR);

    if (create_socket(client) == ERROR)
        return (ERROR);

    // Initialisation du delai entre chaque ping
    client->delay_bt_pings.tv_sec  = SECOND_PAUSE_BT_PINGS;
    client->delay_bt_pings.tv_nsec = NANOSECOND_PAUSE_BT_PINGS;
    client->time_stats.min         = -1;
    client->time_stats.max         = -1;
    client->status                 = EXIT_SUCCESS;

    client->name = address;

    client->packet = malloc(sizeof(t_icmp_packet) * 1024);
    if (!client->packet)
    {
        perror("Malloc: ");
        return (ERROR);
    }
    memset(client->packet, 0, sizeof(t_icmp_packet) * 1024);
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

    double delta = new_rtt - time_stats->average;
    time_stats->average += delta / count;
    time_stats->delta += delta * (new_rtt - time_stats->average);
}