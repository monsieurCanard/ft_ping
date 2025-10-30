#include "../includes/ping.h"

int parse_args(int ac, char** av, t_ping_client* client)
{
    struct option long_options[] = {
        {"verbose", no_argument, 0, 'v'},
        {"type", required_argument, 0, 't'},
        {"help", no_argument, 0, '?'},
        {"ttl", required_argument, 0, 'ttl'},    // Set N as the packet time-to-live.
        {"interval", required_argument, 0, 'i'}, // Définit l'intervalle entre les pings
        {"count", required_argument, 0, 'c'},    // Nombre de pings à envoyer
        {"linger", required_argument, 0, 'W'},
        // Number of seconds to wait for response.
        {0, 0, 0, 0} // Fin des options
    };

    int opt;
    opterr = 0;
    while ((opt = getopt_long(ac, av, "::f?h::t:i:c:W:v", long_options, NULL)) != -1)
    {
        switch (opt)
        {
        case 'v':
            client->args.verbose = true;
            break;
        case 't':
            // TODO : Change type request
            client->args.request_type = atoi(optarg);
            break;
        case '?':
            if (optopt != 0)
            {
                if (optopt == 't' || optopt == 'i' || optopt == 'c' || optopt == 'W')
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                else
                    fprintf(stderr, "Unknown option '-%c'.\n", optopt);
                return (ERROR);
            }
            print_helper();
            return (ERROR);
        case 'ttl':
            // Définir le TTL
            client->args.ttl = atoi(optarg);
            break;
        case 'i':
            client->args.interval = atoi(optarg); // en secondes
            if (client->args.interval < 0)
            {
                fprintf(stderr, "ping: invalid interval %s\n", optarg);
                return (ERROR);
            }
            break;
        case 'c':
            client->args.count = atoi(optarg);
            if (client->args.count <= 0)
            {
                fprintf(stderr, "ping: bad number of packets to transmit.\n");
                return (ERROR);
            }
            break;
        case 'W':
            client->args.timeout = atoi(optarg);
            if (client->args.timeout < 0)
            {
                fprintf(stderr, "ping: invalid timeout %s\n", optarg);
                return (ERROR);
            }
            break;
        default:
            fprintf(stderr, "Unknown option: %c\n", opt);
            return (ERROR);
        }
    }
    return (SUCCESS);
}