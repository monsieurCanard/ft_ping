#include "../includes/ping.h"

int parse_args(int ac, char** av, t_ping_client* client)
{
    struct option long_options[] = {
        {"verbose", no_argument, 0, 'v'},
        // Flood ping. Outputs packets as fast as they come back or one hundred times per second,
        // whichever is more. For every ECHO_REQUEST sent a period "." is printed, while for every
        // ECHO_REPLY received a backspace is printed. This provides a rapid display of how many
        // packets are being dropped. Only the super-user may use this option. This can be very hard
        // on a network and should be used with caution.
        {"flood", no_argument, 0, 'f'},
        {"help", no_argument, 0, '?'},

        {"ttl", required_argument, 0, 't'},      // Set N as the packet time-to-live.
        {"interval", required_argument, 0, 'i'}, // Définit l'intervalle entre les pings
        {"count", required_argument, 0, 'c'},    // Nombre de pings à envoyer
        {"linger", required_argument, 0, 'W'},
        // Number of seconds to wait for response.
        {0, 0, 0, 0} // Fin des options
    };

    int opt;
    while ((opt = getopt_long(ac, av, "::fh::t:i:c:W:v", long_options, NULL)) != -1)
    {
        switch (opt)
        {
        case 'v':
            client->args.verbose = true;
            break;
        case 'f':
            fprintf(stdout, "Flood mode: enabled\nTarget: %s\n", av[1]);
            if (client->args.interval != 0)
            {
                print_error("-f and -i incompatible options\n");
                return (EXIT_FAILURE);
            }
            client->args.flood = true;
            break;
        case '?':
            print_helper();
            return (EXIT_SUCCESS);
        case 't':
            // Définir le TTL
            client->args.ttl = atoi(optarg);
            break;
        case 'i':
            if (client->args.flood == true)
            {
                print_error("-f and -i incompatible options\n");
                return (EXIT_FAILURE);
            }
            client->args.interval = atoi(optarg); // en secondes
            if (client->args.interval < 0)
            {
                print_error("invalid interval\n");
                return (EXIT_FAILURE);
            }
            break;
        case 'c':
            client->args.count = atoi(optarg);
            if (client->args.count <= 0)
            {
                print_error("invalid count of packets to transmit\n");
                return (EXIT_FAILURE);
            }
            break;
        case 'W':
            client->args.timeout = atoi(optarg);
            if (client->args.timeout < 0)
            {
                print_error("invalid timeout\n");
                return (EXIT_FAILURE);
            }
            break;
        // case -1:
        //     // Option inconnue
        //     fprintf(stderr, "Unknown option: %s\n", av[optind - 1]);
        default:
            fprintf(stderr, "Unknown option: %c\n", opt);
            return (EXIT_FAILURE);
        }
    }
    return (SUCCESS);
}