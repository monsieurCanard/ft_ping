#include "../includes/ping.h"

extern bool g_exit_program;

void send_message(t_ping_client* client, struct sockaddr_in sockaddr)
{
    unsigned char send_buff[8 + PAYLOAD_SIZE];
    int           payload_size = 0;

    // ! Envoi immediat
    client->seq++;
    payload_size = build_echo_request(client, send_buff);
    if (payload_size == ERROR)
    {
        client->status = EXIT_FAILURE;
        exit_program(client);
    }

    // ! Envoi de la requete ICMP
    sendto(client->fd, send_buff, payload_size, 0, (struct sockaddr*)&sockaddr, sizeof(sockaddr));
    client->packet[client->seq].received = 0;
    client->counter.transmitted++;
}

void main_loop_icmp(t_ping_client* client)
{
    unsigned char recv_buff[8 + 20 + PAYLOAD_SIZE];
    // int            receive_packet, ret = 0;
    struct timeval start_time, recv_time, send_time, now;
    float          new_rtt;

    struct timeval timeout;
    timeout.tv_sec  = 0;
    timeout.tv_usec = 0;

    gettimeofday(&start_time, NULL);
    send_message(client, client->sockaddr);
    gettimeofday(&send_time, NULL);

    while (!g_exit_program)
    {
        FD_ZERO(&client->read_fds);
        FD_SET(client->fd, &client->read_fds);

        gettimeofday(&now, NULL);
        if (client->args.all_args & OPT_TIMEOUT)
        {
            struct timeval elapsed = sub_timestamp(&now, &start_time);
            if (elapsed.tv_sec >= client->args.timeout)
            {
                g_exit_program = true;
            }
        }

        if ((client->packet[client->seq].received == 0) &&
            (is_timeout(client, &now, &send_time) == true))
        {
            printf("Request timeout for icmp_seq %d\n", client->seq);
            client->packet[client->seq % MAX_PING_SAVES].received = -1;
            client->counter.lost++;
        }

        if (client->packet[client->seq % MAX_PING_SAVES].received != 0 &&
            resend_packet(client, &now, &send_time) == true)
        {
            send_message(client, client->sockaddr);
            gettimeofday(&send_time, NULL);
        }

        int ret = select(client->fd + 1, &client->read_fds, NULL, NULL, &timeout);
        if (ret < 0)
        {
            exit_program(client);
        }
        else if (ret == 1)
        {
            // ! Reception de la reponse ICMP
            socklen_t addrlen = sizeof(client->sockaddr);
            ret               = recvfrom(client->fd,
                           recv_buff,
                           sizeof(recv_buff),
                           0,
                           (struct sockaddr*)&client->sockaddr,
                           &addrlen);
            gettimeofday(&recv_time, NULL);
            new_rtt = verify_response(client, recv_buff, recv_time);
            if (new_rtt > 0)
            {
                client->packet[client->seq % MAX_PING_SAVES].received = 1;
                new_rtt *= -1;
                client->counter.received++;
            }

            if (client->args.all_args & OPT_COUNT && client->counter.received >= client->args.count)
            {
                g_exit_program = true;
            }
        }
    }
}