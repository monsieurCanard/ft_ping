#include "../includes/ping.h"

t_ping_client client;

void exit_program(int sig)
{
    (void)sig;

    if (client.fd > 0)
    {
        struct timeval end_time;
        gettimeofday(&end_time, NULL);

        double total_time = (end_time.tv_sec - client.start_time->tv_sec) * 1000.0 +
                            (end_time.tv_usec - client.start_time->tv_usec) / 1000.0;

        double mdev         = (client.counter.transmitted > 0)
                                  ? sqrt(client.rtt.delta / client.counter.transmitted)
                                  : 0.0;
        double success_rate = client.counter.transmitted == 0
                                  ? 0.0
                                  : ((client.counter.transmitted - client.counter.received) /
                                     (double)client.counter.transmitted) *
                                        100.0;

        print_ping_infos(total_time, success_rate, mdev);
    }

    if (client.start_time)
        free(client.start_time);

    if (client.recv_time)
        free(client.recv_time);

    if (client.fd >= 0)
        close(client.fd);

    exit(client.status);
}

int main(int ac, char** av)
{
    if (ac != 2)
    {
        fprintf(stderr,
                "Wrong number of argument: Actual %d -> Need 1. Usage ./ft_ping [hostname/ip]",
                ac - 1);
        return (EXIT_FAILURE);
    }

    signal(SIGINT, exit_program);
    struct sockaddr_in sockaddr;

    int ret = create_client(&client, &sockaddr, av[1]);
    if (ret == ERROR)
    {
        client.status = EXIT_FAILURE;
        exit_program(0);
    }

    // 28 = 20 (IP header min) + 8 (ICMP header) (peut etre variable mais pour l'affichage initial
    // on met 28)
    printf(
        "PING %s (%s) %d(%d) bytes of data.\n", av[1], client.ip, PAYLOAD_SIZE, PAYLOAD_SIZE + 28);

    main_loop_icmp(sockaddr);

    exit_program(0);
}