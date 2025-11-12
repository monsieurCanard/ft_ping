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

    if ((client->packets[client->seq % MAX_PING_SAVES].status == 0) &&
        (is_timeout(client, now, send_time) == true))
    {
        printf("Request timeout for icmp_seq %d\n", client->seq);
        client->packets[client->seq % MAX_PING_SAVES].status = -1;
        client->counter.lost++;
    }

    if (client->packets[client->seq % MAX_PING_SAVES].status != 0 &&
        resend_packet(client, now, send_time) == true)
    {
        return RESEND;
    }

    return SUCCESS;
}

static struct data_icmp extract_paquet_icmp(unsigned char* buff)
{

    struct data_icmp data;

    data.ip_header    = (struct iphdr*)buff;
    int ip_header_len = data.ip_header->ihl * 4;
    data.data         = (struct icmphdr*)(buff + ip_header_len);
    return data;
}

float verify_response(t_ping_client* client, unsigned char* recv_buff, struct timeval recv_time)
{

    struct data_icmp icmp = extract_paquet_icmp(recv_buff);

    if (icmp.data->type != ICMP_ECHOREPLY)
    {
        handle_error_icmp(icmp.data, icmp.ip_header, client);
        return (ERROR);
    }

    if (!paquet_for_me(icmp.data))
        return (ERROR);

    unsigned char icmp_buf[8 + PAYLOAD_SIZE];
    memcpy(icmp_buf, icmp.data, 8 + PAYLOAD_SIZE);

    uint16_t original_checksum = icmp.data->checksum;
    uint16_t recv_seq          = ntohs(icmp.data->un.echo.sequence);

    struct timeval* send_time = (struct timeval*)(icmp_buf + 8);

    float rtt = (recv_time.tv_sec - send_time->tv_sec) * 1000.0 +
                (recv_time.tv_usec - send_time->tv_usec) / 1000.0;

    struct icmphdr* icmp_check = (struct icmphdr*)icmp_buf;
    icmp_check->checksum       = 0;
    uint16_t recv_checksum     = icmp_checksum((unsigned char*)icmp_check, 8 + PAYLOAD_SIZE);

    if (client->packets[recv_seq % MAX_PING_SAVES].status == -1)
    {
        if (client->args.all_args & OPT_VERBOSE)
            fprintf(stderr, "Late reply for icmp_seq %d (previously timed out)\n", recv_seq);
        client->counter.error++;
        return (-rtt);
    }
    else if (client->packets[recv_seq % MAX_PING_SAVES].status == true)
    {
        if (client->args.all_args & OPT_VERBOSE)
            fprintf(stderr, "Duplicate reply for icmp_seq %d\n", recv_seq);
        client->counter.error++;
        return (-rtt);
    }

    if (recv_checksum != original_checksum || recv_seq > client->seq || icmp.data->code != 0)
    {
        if (client->args.all_args & OPT_VERBOSE)
            fprintf(stderr,
                    "From %s icmp_seq=%d type=%d code=%d\n",
                    inet_ntoa(*(struct in_addr*)&icmp.ip_header->saddr),
                    recv_seq,
                    icmp.data->type,
                    icmp.data->code);
        client->counter.error++;
        return (-rtt);
    }

    if (ntohs(icmp.data->un.echo.id) != (getpid() & 0xFFFF))
    {
        if (client->args.all_args & OPT_VERBOSE)
            fprintf(stderr, "Received packet with unknown ID %d\n", ntohs(icmp.data->un.echo.id));
        return (-rtt);
    }

    client->packets[recv_seq % MAX_PING_SAVES].status = true;
    print_ping_line(icmp.ip_header, icmp.data, rtt, icmp.ip_header->ttl, client->packets);

    update_client_time_stats(
        &client->time_stats, rtt, client->counter.received + client->counter.lost + 1);

    return (rtt);
}