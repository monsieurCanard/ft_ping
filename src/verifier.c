#include <netinet/in.h>

#include "../includes/ping.h"

extern bool g_exit_program;

static t_data_icmp extract_paquet_icmp(unsigned char* buff)
{
    t_data_icmp data;

    data.ip_header    = (struct iphdr*)buff;
    int ip_header_len = data.ip_header->ihl * 4;
    data.data         = (struct icmphdr*)(buff + ip_header_len);

    return data;
}

static bool paquet_valid(t_ping_client* client, t_data_icmp icmp)
{

    unsigned char icmp_buf[8 + PAYLOAD_SIZE];
    memcpy(icmp_buf, icmp.data, 8 + PAYLOAD_SIZE);

    uint16_t        original_checksum = icmp.data->checksum;
    uint16_t        recv_seq          = ntohs(icmp.data->un.echo.sequence);
    struct icmphdr* icmp_check        = (struct icmphdr*)icmp_buf;
    icmp_check->checksum              = 0;
    uint16_t recv_checksum            = icmp_checksum((unsigned char*)icmp_check, 8 + PAYLOAD_SIZE);

    struct in_addr addr;
    addr.s_addr = icmp.ip_header->saddr;

    if (ntohs(icmp.data->un.echo.id) != (getpid() & 0xFFFF))
    {
        printf("From %s : Received packet with invalid identifier\n ", inet_ntoa(addr));
        return false;
    }

    if (recv_checksum != original_checksum)
    {
        printf("From %s: Received packet with invalid checksum\n ", inet_ntoa(addr));
        return false;
    }

    if (recv_seq > client->seq)
    {
        printf("From %s : Received packet with invalid sequence number\n ", inet_ntoa(addr));
        return false;
    }

    if (icmp.data->code != 0)
    {
        if (client->args.all_args & OPT_VERBOSE)
        {
            switch (icmp.data->code)
            {
            case ICMP_NET_UNREACH:
                printf("Network unreachable\n");
                break;
            case ICMP_HOST_UNREACH:
                printf("Host unreachable\n");
                break;
            case ICMP_PROT_UNREACH:
                printf("Protocol unreachable\n");
                break;
            case ICMP_PORT_UNREACH:
                printf("Port unreachable\n");
                break;
            default:
                printf(
                    "From %s : Received packet with code %d\n ", inet_ntoa(addr), icmp.data->code);
            }
        }
        return false;
    }
    return true;
}

float verify_response_and_print(t_ping_client* client,
                                unsigned char* recv_buff,
                                struct timeval recv_time)
{

    t_data_icmp icmp = extract_paquet_icmp(recv_buff);
    bool        dup  = false;

    if (icmp.data->type != ICMP_ECHOREPLY)
    {
        handle_error_icmp(icmp, client);
        return (ERROR);
    }

    if (!paquet_valid(client, icmp))
        return (ERROR);

    unsigned char icmp_buf[8 + PAYLOAD_SIZE];
    memcpy(icmp_buf, icmp.data, 8 + PAYLOAD_SIZE);

    uint16_t        recv_seq  = ntohs(icmp.data->un.echo.sequence);
    struct timeval* send_time = (struct timeval*)(icmp_buf + 8);

    float rtt = (recv_time.tv_sec - send_time->tv_sec) * 1000.0 +
                (recv_time.tv_usec - send_time->tv_usec) / 1000.0;

    if (client->packets[recv_seq % MAX_PING_SAVES].receive == ERROR)
    {
        if (client->args.all_args & OPT_VERBOSE)
            printf("Late reply for icmp_seq %d (previously timed out)\n", recv_seq);
        return (ERROR);
    }
    else if (client->packets[recv_seq % MAX_PING_SAVES].receive == true)
    {
        dup = true;
        client->counter.dup++;
    }

    client->packets[recv_seq % MAX_PING_SAVES].receive = true;

    if ((client->args.all_args & OPT_LINGER && rtt > (float)client->args.linger * 1000.0) ||
        rtt > (float)TIMEOUT_SEC * 1000.0)
    {
        client->packets[recv_seq % MAX_PING_SAVES].receive = ERROR;
        client->counter.lost++;
        return (ERROR);
    }

    client->counter.received++;
    client->packets[recv_seq % MAX_PING_SAVES].receive = true;

    print_ping_line(icmp, rtt, icmp.ip_header->ttl, dup);
    return (rtt);
}

int time_checker(t_ping_client*  client,
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

    if (resend_packet(client, now, send_time) == true)
    {
        return RESEND;
    }

    return SUCCESS;
}
// static uint16_t get_checksum