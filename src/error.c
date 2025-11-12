#include <sys/socket.h>

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

        if (ntohs(orig_icmp->un.echo.id) != (getpid() & 0xFFFF))
            return;

        if (client->args.all_args & OPT_VERBOSE)
            fprintf(stderr,
                    "%d bytes from %s : %s\n",
                    ntohs(ip->tot_len),
                    inet_ntoa(*(struct in_addr*)&ip->saddr),
                    (icmp->type == ICMP_TIME_EXCEEDED) ? "Time to live exceeded"
                                                       : "Destination Unreachable");
        fprintf(stderr, "IP Hdr Dump:\n");
        for (int i = 0; i < orig_ihl; i++)
        {
            fprintf(stderr, "%02x", *((unsigned char*)orig_ip + i));
            if ((i + 1) % 2 == 0)
            {
                fprintf(stderr, " ");
            }
        }

        char src_buf[INET_ADDRSTRLEN], dst_buf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &orig_ip->saddr, src_buf, sizeof src_buf);
        inet_ntop(AF_INET, &orig_ip->daddr, dst_buf, sizeof dst_buf);

        printf("\nVr HL TOS  Len  ID  Flg  Off TTL Pro  Cks      Src            Dst\n");
        printf("%1d  %1d  %02x  %04d %04d %1d  %04d  %02d  %03d %04x  %s    %s\n",
               (orig_ip->version & 0x0F),
               (orig_ip->ihl & 0x0F),
               orig_ip->tos,
               ntohs(orig_ip->tot_len),
               ntohs(orig_ip->id),
               (ntohs(orig_ip->frag_off) >> 13) & 0x07,
               (ntohs(orig_ip->frag_off) & 0x1FFF),
               orig_ip->ttl,
               orig_ip->protocol,
               ntohs(orig_ip->check),
               src_buf,
               dst_buf);

        printf("ICMP: Type %d, code %d, size %d, id 0x%04x, seq 0x%04x\n",
               orig_icmp->type,
               orig_icmp->code,
               ntohs(orig_ip->tot_len) - orig_ihl,
               ntohs(orig_icmp->un.echo.id),
               ntohs(orig_icmp->un.echo.sequence));

        client->counter.error++;
        client->packets[orig_seq % MAX_PING_SAVES].status = -1;
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