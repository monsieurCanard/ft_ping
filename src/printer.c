#include "../includes/ping.h"

void print_start_ping(t_ping_client* client)
{
    // 28 = 20 (IP header min) + 8 (ICMP header) (peut etre variable mais pour l'affichage initial
    // on met 28)
    printf("PING %s (%s) %d(%d) data bytes",
           client->name,
           client->ip,
           PAYLOAD_SIZE,
           PAYLOAD_SIZE + 28);

    if (client->args.all_args & OPT_VERBOSE)
    {
        int pid = getpid() & 0xFFFF;

        printf(" id 0x%x = %d", pid, pid);
    }

    printf("\n");
}

void print_ping_final_stats(t_ping_client* client, double success_rate, double mdev, int total_msg)
{
    fprintf(stdout, "\n--- %s ping statistics ---\n", client->ip);

    if (client->counter.dup != 0)
    {
        printf("%d packets transmitted, %d packets received, +%d duplicates, %.1f%% packet loss\n",
               total_msg,
               client->counter.received,
               client->counter.dup,
               success_rate);
    }
    else
    {
        printf("%d packets transmitted, %d packets received, %.1f%% packet loss\n",
               total_msg,
               client->counter.received,
               success_rate);
    }

    if (client->counter.received > 0)
    {
        printf("round-trip min/avg/maxvvvv/mdev = %.3f/%.3f/%.3f/%.3f ms\n",
               client->time_stats.min,
               client->time_stats.average,
               client->time_stats.max,
               mdev);
    }
}

void print_ping_line(t_data_icmp icmp, float rtt, int ttl, bool dup)
{
    struct in_addr addr;
    addr.s_addr = icmp.ip_header->saddr;

    printf("%d bytes from %s: icmp_seq=%d ttl=%d rtt=%.2f ms",
           PAYLOAD_SIZE + 8,
           inet_ntoa(addr),
           ntohs(icmp.data->un.echo.sequence),
           ttl,
           rtt);

    if (dup)
        printf(" (DUP!)");

    printf("\n");
}

void print_helper()
{
    printf("Usage: ./ft_ping [options] <destination>\n");
    printf("Options:\n");
    printf("  -t, --ttl N        Set the packet time-to-live to N\n");
    printf("  -i, --interval N   Set the interval between sending each packet to N seconds\n");
    printf("  -v, --verbose      Enable verbose output\n");
    printf("  -w, --timeout N    Specify a timeout of N seconds before exiting\n");
    printf("  -c, --count N      Stop after sending N packets\n");
    printf("  -W, --linger N     Set the time to wait for a response to N seconds\n");
    printf("  -?, --help         Display this help message and exit\n");
    printf("\nExample:\n");
    printf("sudo ./ft_ping [options] [hostname/ip]\n");
    printf("\nNote: This program requires root privileges to run.\n");
}