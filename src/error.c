#include "../includes/ping.h"

void handle_error_icmp(struct icmphdr* icmp, struct iphdr* ip, t_ping_client* client)
{
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
            if (client->args.all_args & OPT_VERBOSE)
                fprintf(stderr,
                        "From %s icmp_seq=%d Time to live exceeded\n",
                        inet_ntoa(*(struct in_addr*)&ip->saddr),
                        orig_seq);
        }
        else /* DEST_UNREACH */
        {
            if (client->args.all_args & OPT_VERBOSE)
                fprintf(stderr,
                        "From %s icmp_seq=%d Destination Unreachable\n",
                        inet_ntoa(*(struct in_addr*)&ip->saddr),
                        orig_seq);
        }
        client->counter.error++;
        return;
    }

    if (client->args.all_args & OPT_VERBOSE)
        fprintf(stderr,
                "From %s icmp_seq=%d type=%d code=%d\n",
                inet_ntoa(*(struct in_addr*)&ip->saddr),
                ntohs(icmp->un.echo.sequence),
                icmp->type,
                icmp->code);

    client->counter.error++;
}