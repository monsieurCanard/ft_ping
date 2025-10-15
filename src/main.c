
#include "../includes/ping.h"

bool g_exit_program = false;
void set_exit_program(int sig)
{
    (void)sig;
    g_exit_program = true;
}

int main(int ac, char** av)
{
    t_ping_client client = {0};

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
    printf("PING %s (%s) %d(%d) bytes of data.\n",
           av[optind],
           client.ip,
           PAYLOAD_SIZE,
           PAYLOAD_SIZE + 28);

    main_loop_icmp(&client);

    exit_program(&client);
}