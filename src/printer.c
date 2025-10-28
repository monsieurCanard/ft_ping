#include "../includes/ping.h"

void print_error(char* msg)
{
    fprintf(stderr, "ping: %s\n", msg);
}

void print_ping_infos(t_ping_client* client, double total_time, double success_rate, double mdev)
{
    // Voir avec JB pour l'affichage des statistiques
    (void)total_time;

    fprintf(stdout, "\n--- %s ping statistics ---\n", client->ip);
    if (client->counter.error != 0)
    {
        fprintf(stdout,
                "%d packets transmitted, %d packets received, +%d errors, %.1f%% packet loss\n",
                client->counter.transmitted,
                client->counter.received,
                client->counter.error,
                success_rate);
    }
    else
    {
        fprintf(stdout,
                "%d packets transmitted, %d packets received, %.1f%% packet loss\n",
                client->counter.transmitted,
                client->counter.received,
                success_rate);
    }
    if (client->counter.received > 0)
    {
        fprintf(stdout,
                "round-trip min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n",
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