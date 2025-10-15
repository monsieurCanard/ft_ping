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
    client->counter.transmitted++;
}

void main_loop_icmp(t_ping_client* client)
{
    unsigned char  recv_buff[8 + 20 + PAYLOAD_SIZE];
    int            receive_packet, ret = 0;
    struct timeval recv_time;
    float          new_rtt;

    while (!g_exit_program)
    {
        receive_packet = 0;
        send_message(client, client->sockaddr);

        while (!receive_packet)
        {

            // ! Reception de la reponse ICMP
            socklen_t addrlen = sizeof(client->sockaddr);
            ret               = recvfrom(client->fd,
                           recv_buff,
                           sizeof(recv_buff),
                           MSG_DONTWAIT,
                           (struct sockaddr*)&client->sockaddr,
                           &addrlen);

            gettimeofday(&recv_time, NULL);
            if (ret == ERROR)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    continue;
                }

                perror("Recvfrom: ");
                client->status = EXIT_FAILURE;
                exit_program(client);
            }

            new_rtt = verify_response(client, recv_buff, recv_time);
            if (new_rtt != ERROR)
            {
                client->counter.received++;
            }
            receive_packet = 1;
        }
        usleep((client->delay_bt_pings.tv_sec * 1000000 + client->delay_bt_pings.tv_nsec / 1000) -
               (new_rtt * 1000));
    }
    exit_program(client);
}