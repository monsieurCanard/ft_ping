
#include "../includes/ping.h"

bool g_exit_program = false;
void set_exit_program(int sig)
{
    (void)sig;
    g_exit_program = true;
}

int main(int ac, char** av)
{
    if (getuid() != 0)
    {
        fprintf(stderr, "ft_ping: must be run as root\n");
        return (EXIT_FAILURE);
    }

    t_ping_client client;
    memset(&client, 0, sizeof(t_ping_client));

    if (parse_args(ac, av, &client) == ERROR)
        return (EXIT_FAILURE);

    signal(SIGINT, set_exit_program);

    int ret = create_client(&client, av[optind]);
    if (ret == ERROR)
    {
        client.status = EXIT_FAILURE;
        exit_program(&client);
    }

    // 28 = 20 (IP header min) + 8 (ICMP header) (peut etre variable mais pour l'affichage initial
    // on met 28)
    printf(
        "PING %s (%s) %d(%d) data bytes", client.name, client.ip, PAYLOAD_SIZE, PAYLOAD_SIZE + 28);
    if (client.args.all_args & OPT_VERBOSE)
    {
        int pid = getpid() & 0xFFFF;

        printf(" id 0x%x = %d", pid, pid);
    }
    printf("\n");
    main_loop_icmp(&client);

    exit_program(&client);
}