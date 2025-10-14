#include "../includes/ping.h"

int verify_response(t_ping_client* client, unsigned char* recv_buff, struct timeval recv_time)
{
    // buff contient tout le paquet IP recu
    // On doit sauter l'entete IP pour acceder a l'entete ICMP
    // La taille de l'entete IP est variable, on recupere sa taille
    // dans les 4 premiers bits de l'entete IP
    struct iphdr* ip  = (struct iphdr*)recv_buff;
    int           ttl = ip->ttl;

    // Taille de l'entete IP en octets
    // On recupere l'entete ICMP
    int             ip_header_len = ip->ihl * 4;
    struct icmphdr* icmp          = (struct icmphdr*)(recv_buff + ip_header_len);
    if (ntohs(icmp->un.echo.id) != (getpid() & 0xFFFF))
    {
        fprintf(stderr, "Received packet with unknown ID %d\n", ntohs(icmp->un.echo.id));
        return (ERROR);
    }

    unsigned char icmp_buf[8 + PAYLOAD_SIZE];
    memcpy(icmp_buf, icmp, 8 + PAYLOAD_SIZE);

    uint16_t original_checksum = icmp->checksum;
    uint16_t recv_seq          = ntohs(icmp->un.echo.sequence);

    client->send_time = (struct timeval*)(icmp_buf + 8);
    float new_rtt     = (recv_time.tv_sec - client->send_time->tv_sec) * 1000.0 +
                    (recv_time.tv_usec - client->send_time->tv_usec) / 1000.0;

    struct icmphdr* icmp_check = (struct icmphdr*)icmp_buf;
    icmp_check->checksum       = 0;
    uint16_t recv_checksum     = icmp_checksum((unsigned char*)icmp_check, 8 + PAYLOAD_SIZE);

    if (client->packet[recv_seq].received == -1)
    {
        fprintf(stderr, "Late reply for icmp_seq %d (previously timed out)\n", recv_seq);
    }
    else if (client->packet[recv_seq].received == true)
    {
        fprintf(stderr, "Duplicate reply for icmp_seq %d\n", recv_seq);
    }

    if (icmp->type != ICMP_ECHOREPLY)
    {
        fprintf(stdout,
                "From %s icmp_seq=%d type=%d code=%d\n",
                client->ip,
                recv_seq,
                icmp->type,
                icmp->code);
    }

    if (recv_checksum != original_checksum || icmp->type != ICMP_ECHOREPLY ||
        recv_seq > client->seq || icmp->code != 0 || ip->saddr != client->target_addr)
    {
        if (icmp->type == ICMP_DEST_UNREACH)
        {
            fprintf(stderr, "From %s icmp_seq=%d Destination Unreachable\n", client->ip, recv_seq);
            client->counter.error++;
        }
        else if (icmp->type == ICMP_TIME_EXCEEDED)
        {
            fprintf(stderr, "From %s icmp_seq=%d Time to live exceeded\n", client->ip, recv_seq);
            client->counter.error++;
        }
    }

    client->packet[recv_seq].received = true;
    print_ping_line(ip, icmp, new_rtt, ttl, client->packet);
    update_client_time_stats(&client->rtt, new_rtt, client->counter.transmitted);

    return (SUCCESS);
}