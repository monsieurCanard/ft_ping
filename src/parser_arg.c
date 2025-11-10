#include "../includes/ping.h"

int parse_args(int ac, char** av, t_ping_client* client)
{
    struct option long_options[] = {{"help", no_argument, 0, '?'},
                                    {"interval", required_argument, 0, 'i'},
                                    {"verbose", no_argument, 0, 'v'},
                                    {"linger", required_argument, 0, 'W'},
                                    {"timeout", required_argument, 0, 'w'},
                                    {"ttl", required_argument, 0, 't'},
                                    {"count", required_argument, 0, 'c'},
                                    {0, 0, 0, 0}};

    int opt;
    opterr = 0;
    while ((opt = getopt_long(ac, av, "?vl:t:i:c:W:w:", long_options, NULL)) != -1)
    {
        switch (opt)
        {
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

        case 'v':
            client->args.all_args |= OPT_VERBOSE;
            break;
        case 't':
            client->args.all_args |= OPT_TTL;
            client->args.ttl = atoi(optarg);
            break;
        case 'i':
            client->args.all_args |= OPT_INTERVAL;
            client->args.interval = atoi(optarg); // en secondes
            if (client->args.interval < 0)
            {
                fprintf(stderr, "ping: invalid interval %s\n", optarg);
                return (ERROR);
            }
            break;
        case 'c':
            client->args.all_args |= OPT_COUNT;
            client->args.count = atoi(optarg);
            if (client->args.count <= 0)
            {
                fprintf(stderr, "ping: bad number of packets to transmit.\n");
                return (ERROR);
            }
            break;
        case 'W':
            client->args.all_args |= OPT_LINGER;
            client->args.linger = atoi(optarg);
            if (client->args.linger < 0)
            {
                fprintf(stderr, "ping: invalid linger time %s\n", optarg);
                return (ERROR);
            }
            break;

        case 'w':
            client->args.all_args |= OPT_TIMEOUT;
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