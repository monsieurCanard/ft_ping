#include <netinet/in.h>
#include <netinet/ip.h>

#include "../includes/ping.h"

extern bool g_exit_program;

static void send_message(t_ping_client* client, struct sockaddr_in sockaddr)
{
    unsigned char send_buff[sizeof(struct iphdr) + PAYLOAD_SIZE];
    int           payload_size = 0;

    client->seq++;
    client->counter.transmitted++;

    payload_size = build_echo_request(client, send_buff);
    if (payload_size == ERROR)
    {
        client->status = EXIT_FAILURE;
        exit_program(client);
    }

    if (sendto(client->fd,
               send_buff,
               payload_size,
               0,
               (struct sockaddr*)&sockaddr,
               sizeof(sockaddr)) == -1)
    {
        perror("Sendto error: ");
    }

    client->packets[client->seq % MAX_PING_SAVES].receive = false;
}

void main_loop_icmp(t_ping_client* client)
{
    unsigned char  recv_buff[sizeof(struct iphdr) + sizeof(struct icmphdr) + PAYLOAD_SIZE];
    struct timeval start_time, recv_time, send_time, now;
    float          new_rtt;

    struct timeval timeout;
    timeout.tv_sec  = 1;
    timeout.tv_usec = 0;

    gettimeofday(&start_time, NULL);
    send_message(client, client->sockaddr);
    gettimeofday(&send_time, NULL);

    while (!g_exit_program)
    {
        FD_ZERO(&client->read_fds);
        FD_SET(client->fd, &client->read_fds);

        gettimeofday(&now, NULL);

        if (time_checker(client, &start_time, &now, &send_time) == RESEND)
        {
            send_message(client, client->sockaddr);
            gettimeofday(&send_time, NULL);
        }

        int ret = select(client->fd + 1, &client->read_fds, NULL, NULL, &timeout);
        if (ret < 0)
        {
            if (errno != EINTR)
                client->status = EXIT_FAILURE;

            exit_program(client);
        }

        if (ret == 1)
        {
            if (FD_ISSET(client->fd, &client->read_fds) == 0)
                continue;

            struct sockaddr_in src_addr;
            socklen_t          addrlen = sizeof(src_addr);
            ret                        = recvfrom(
                client->fd, recv_buff, sizeof(recv_buff), 0, (struct sockaddr*)&src_addr, &addrlen);
            if (ret < 0)
            {
                perror("Recvfrom error: ");
                continue;
            }

            gettimeofday(&recv_time, NULL);
            new_rtt = verify_response_and_print(client, recv_buff, recv_time);
            if (new_rtt != ERROR)
            {
                update_client_time_stats(&client->time_stats, new_rtt, client->counter.received);
                if (client->args.all_args & OPT_COUNT &&
                    client->counter.received >= client->args.count)
                {
                    g_exit_program = true;
                }
            }
        }
    }
}