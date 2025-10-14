#include "../includes/ping.h"

void print_ping_infos(t_ping_client* client, double total_time, double success_rate, double mdev)
{
    fprintf(stdout, "\n--- %s ping statistics ---\n", client->ip);
    if (client->counter.error != 0)
    {
        fprintf(
            stdout,
            "%d packets transmitted, %d received, +%d errors, %.1f%% packet loss, time %.0f ms\n",
            client->counter.transmitted,
            client->counter.received,
            client->counter.error,
            success_rate,
            total_time);
    }
    else
    {
        fprintf(stdout,
                "%d packets transmitted, %d received, %.1f%% packet loss, time %.0f ms\n",
                client->counter.transmitted,
                client->counter.received,
                success_rate,
                total_time);
    }
    if (client->counter.received > 0)
    {
        fprintf(stdout,
                "rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n",
                client->rtt.min,
                client->rtt.average,
                client->rtt.max,
                mdev);
    }
}

void print_ping_line(
    struct iphdr* ip, struct icmphdr* icmp, float rtt, int ttl, t_icmp_packet* packet)
{
    (void)packet;
    struct in_addr addr;
    addr.s_addr          = ip->saddr;
    struct hostent* host = gethostbyaddr(&addr, sizeof(addr), AF_INET);
    printf("%d bytes from %s (%s): icmp_seq=%d ttl=%d rtt=%.2f ms\n",
           PAYLOAD_SIZE + 8,
           (host) ? host->h_name : inet_ntoa(addr),
           inet_ntoa(addr),
           ntohs(icmp->un.echo.sequence),
           ttl,
           rtt);
}

void print_helper()
{
    printf("Usage: ./ft_ping [options] <destination>\n");
    printf("Options:\n");
    printf("  -f, --flood       Flood mode: send packets as fast as possible\n");
    printf("  -h, --help        Display this help message and exit\n");
    printf("\nExample:\n");
    printf("  sudo ./ft_ping --flood [hostname/ip]\n");
    printf("\nNote: This program requires root privileges to run.\n");
}