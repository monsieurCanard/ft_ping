#include "../includes/ping.h"

extern bool g_exit_program;

static bool paquet_for_me(struct icmphdr* icmp)
{
    return (ntohs(icmp->un.echo.id) == (getpid() & 0xFFFF));
}

int timeout_or_resend(t_ping_client*  client,
                      struct timeval* start_time,
                      struct timeval* now,
                      struct timeval* send_time)
{
    if (client->args.all_args & OPT_TIMEOUT)
    {
        struct timeval elapsed = sub_timestamp(now, start_time);
        if (elapsed.tv_sec >= client->args.timeout)
        {
            g_exit_program = true;
        }
    }

    if ((client->packet[client->seq % MAX_PING_SAVES].status == 0) &&
        (is_timeout(client, now, send_time) == true))
    {
        printf("Request timeout for icmp_seq %d\n", client->seq);
        client->packet[client->seq % MAX_PING_SAVES].status = -1;
        client->counter.lost++;
    }

    if (client->packet[client->seq % MAX_PING_SAVES].status != 0 &&
        resend_packet(client, now, send_time) == true)
    {
        return RESEND;
    }

    return SUCCESS;
}

float verify_response(t_ping_client* client, unsigned char* recv_buff, struct timeval recv_time)
{
    // buff contient tout le paquet IP recu
    // On doit sauter l'entete IP pour acceder a l'entete ICMP
    // La taille de l'entete IP est variable, on recupere sa taille
    // dans les 4 premiers bits de l'entete IP
    struct iphdr* ip  = (struct iphdr*)recv_buff;
    int           ttl = ip->ttl;

    int             ip_header_len = ip->ihl * 4;
    struct icmphdr* icmp          = (struct icmphdr*)(recv_buff + ip_header_len);

    if (!paquet_for_me(icmp))
        return (ERROR);

    if (icmp->type != ICMP_ECHOREPLY)
    {
        handle_error_icmp(icmp, ip, client);
        return (ERROR);
    }

    unsigned char icmp_buf[8 + PAYLOAD_SIZE];
    memcpy(icmp_buf, icmp, 8 + PAYLOAD_SIZE);

    uint16_t original_checksum = icmp->checksum;
    uint16_t recv_seq          = ntohs(icmp->un.echo.sequence);

    struct timeval* send_time = (struct timeval*)(icmp_buf + 8);

    float rtt = (recv_time.tv_sec - send_time->tv_sec) * 1000.0 +
                (recv_time.tv_usec - send_time->tv_usec) / 1000.0;

    struct icmphdr* icmp_check = (struct icmphdr*)icmp_buf;
    icmp_check->checksum       = 0;
    uint16_t recv_checksum     = icmp_checksum((unsigned char*)icmp_check, 8 + PAYLOAD_SIZE);

    if (client->packet[recv_seq % MAX_PING_SAVES].status == -1)
    {
        if (client->args.all_args & OPT_VERBOSE)
            fprintf(stderr, "Late reply for icmp_seq %d (previously timed out)\n", recv_seq);
        client->counter.error++;
        return (-rtt);
    }
    else if (client->packet[recv_seq % MAX_PING_SAVES].status == true)
    {
        if (client->args.all_args & OPT_VERBOSE)
            fprintf(stderr, "Duplicate reply for icmp_seq %d\n", recv_seq);
        client->counter.error++;
        return (-rtt);
    }

    if (recv_checksum != original_checksum || recv_seq > client->seq || icmp->code != 0)
    {
        if (client->args.all_args & OPT_VERBOSE)
            fprintf(stderr,
                    "From %s icmp_seq=%d type=%d code=%d\n",
                    inet_ntoa(*(struct in_addr*)&ip->saddr),
                    recv_seq,
                    icmp->type,
                    icmp->code);
        client->counter.error++;
        return (-rtt);
    }

    if (ntohs(icmp->un.echo.id) != (getpid() & 0xFFFF))
    {
        if (client->args.all_args & OPT_VERBOSE)
            fprintf(stderr, "Received packet with unknown ID %d\n", ntohs(icmp->un.echo.id));
        return (-rtt);
    }

    client->packet[recv_seq % MAX_PING_SAVES].status = true;
    print_ping_line(ip, icmp, rtt, ttl, client->packet);

    update_client_time_stats(
        &client->time_stats, rtt, client->counter.received + client->counter.lost + 1);

    return (rtt);
}