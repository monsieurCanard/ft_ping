#include <getopt.h>

#include "../includes/ping.h"

bool g_exit_program = false;
void set_exit_program(int sig)
{
    (void)sig;
    g_exit_program = true;
}

int main(int ac, char** av)
{
    struct option long_options[] = {
        {"flood", no_argument, 0, 'f'}, // Envoi le plus rapidement possible
        {"help", no_argument, 0, '?'},
        {"ttl", required_argument, 0, 't'},      // Définit le TTL
        {"interval", required_argument, 0, 'i'}, // Définit l'intervalle entre les pings
        {0, 0, 0, 0}                             // Fin des options
    };
    int opt;
    while ((opt = getopt_long(ac, av, "::f::h::t:", long_options, NULL)) != -1)
    {
        switch (opt)
        {
        case 'f':
            fprintf(stdout, "Flood mode: enabled\nTarget: %s\n", av[1]);
            // Activer le mode flood
            break;
        case '?':
            print_helper();
            return (EXIT_SUCCESS);
        case 't':
            // Définir le TTL
            break;
        case -1:
            // Option inconnue
            fprintf(stderr, "Unknown option: %s\n", av[optind - 1]);
        default:
            fprintf(stderr, "Unknown option: %c\n", opt);
            return (EXIT_FAILURE);
        }
    };

    signal(SIGINT, set_exit_program);

    t_ping_client client = {0};

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