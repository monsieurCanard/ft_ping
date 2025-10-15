#include "../includes/ping.h"

float verify_response(t_ping_client* client, unsigned char* recv_buff, struct timeval recv_time)
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

    if (icmp->type == ICMP_TIME_EXCEEDED || icmp->type == ICMP_DEST_UNREACH)
    {
        /* the original IP header starts at icmp + 8 */
        unsigned char*  inner     = (unsigned char*)icmp + 8;
        struct iphdr*   orig_ip   = (struct iphdr*)inner;
        int             orig_ihl  = orig_ip->ihl * 4;
        struct icmphdr* orig_icmp = (struct icmphdr*)(inner + orig_ihl);
        uint16_t        orig_seq  = ntohs(orig_icmp->un.echo.sequence);

        if (icmp->type == ICMP_TIME_EXCEEDED)
        {
            if (client->args.verbose)
                fprintf(stderr,
                        "From %s icmp_seq=%d Time to live exceeded\n",
                        inet_ntoa(*(struct in_addr*)&ip->saddr),
                        orig_seq);
        }
        else /* DEST_UNREACH */
        {
            if (client->args.verbose)
                fprintf(stderr,
                        "From %s icmp_seq=%d Destination Unreachable\n",
                        inet_ntoa(*(struct in_addr*)&ip->saddr),
                        orig_seq);
        }

        client->counter.error++;
        return (ERROR);
    }

    unsigned char icmp_buf[8 + PAYLOAD_SIZE];
    memcpy(icmp_buf, icmp, 8 + PAYLOAD_SIZE);

    uint16_t original_checksum = icmp->checksum;
    uint16_t recv_seq          = ntohs(icmp->un.echo.sequence);

    if (recv_seq > client->seq)
    {
        if (client->args.verbose)
            fprintf(stderr,
                    "Out of order packet: icmp_seq %d (expected <= %d)\n",
                    recv_seq,
                    client->seq);
        client->counter.error++;
        return (ERROR);
    }

    client->send_time = (struct timeval*)(icmp_buf + 8);
    float new_rtt     = (recv_time.tv_sec - client->send_time->tv_sec) * 1000.0 +
                    (recv_time.tv_usec - client->send_time->tv_usec) / 1000.0;

    struct icmphdr* icmp_check = (struct icmphdr*)icmp_buf;
    icmp_check->checksum       = 0;
    uint16_t recv_checksum     = icmp_checksum((unsigned char*)icmp_check, 8 + PAYLOAD_SIZE);

    if (client->packet[recv_seq].received == -1)
    {
        if (client->args.verbose)
            fprintf(stderr, "Late reply for icmp_seq %d (previously timed out)\n", recv_seq);
        client->counter.error++;
        return (ERROR);
    }
    else if (client->packet[recv_seq].received == true)
    {
        if (client->args.verbose)
            fprintf(stderr, "Duplicate reply for icmp_seq %d\n", recv_seq);
        client->counter.error++;
        return (ERROR);
    }

    if (recv_checksum != original_checksum || icmp->type != ICMP_ECHOREPLY ||
        recv_seq > client->seq || icmp->code != 0 || ip->saddr != client->target_addr)
    {
        if (client->args.verbose)
            fprintf(stderr,
                    "From %s icmp_seq=%d type=%d code=%d\n",
                    client->ip,
                    recv_seq,
                    icmp->type,
                    icmp->code);
        client->counter.error++;
        return (ERROR);
    }

    if (ntohs(icmp->un.echo.id) != (getpid() & 0xFFFF))
    {
        if (client->args.verbose)
            fprintf(stderr, "Received packet with unknown ID %d\n", ntohs(icmp->un.echo.id));
        return (ERROR);
    }

    client->packet[recv_seq].received = true;
    if (client->args.flood)
        write(1, " ", 1);
    else
        print_ping_line(ip, icmp, new_rtt, ttl, client->packet);

    update_client_time_stats(&client->rtt, new_rtt, client->counter.transmitted);

    return (new_rtt);
}