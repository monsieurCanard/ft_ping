
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

    print_start_ping(&client);
    main_loop_icmp(&client);

    exit_program(&client);
}