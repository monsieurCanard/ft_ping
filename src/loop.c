#include "../includes/ping.h"

extern t_ping_client client;

void main_loop_icmp(struct sockaddr_in sockaddr)
{
    unsigned char buff[8 + PAYLOAD_SIZE];
    int           ret;

    // Boucle principale d'envoi et de reception des paquets ICMP
    // Tant que le client est actif (fd > 0)
    while (client.fd > 0)
    {
        int packet_received = 0;
        client.seq++;

        int payload_size = build_echo_request(buff, client.seq);
        if (payload_size == ERROR)
        {
            client.status = EXIT_FAILURE;
            exit_program(0);
        }

        // ! Envoi de la requete ICMP
        sendto(client.fd, buff, payload_size, 0, (struct sockaddr*)&sockaddr, sizeof(sockaddr));
        client.counter.transmitted++;

        while (!packet_received)
        {
            // ! Reception de la reponse ICMP
            socklen_t addrlen = sizeof(sockaddr);
            ret = recvfrom(client.fd, buff, payload_size, 0, (struct sockaddr*)&sockaddr, &addrlen);

            gettimeofday(client.recv_time, NULL);

            if (ret == ERROR)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    fprintf(stderr, "Request timeout for icmp_seq %d\n", client.seq);
                    client.counter.lost++;
                    break;
                }
                else
                {
                    perror("Recvfrom: ");
                    client.status = EXIT_FAILURE;
                    exit_program(0);
                }
            }
            //! Verification de la reponse ICMP

            // buff contient tout le paquet IP recu
            // On doit sauter l'entete IP pour acceder a l'entete ICMP
            // La taille de l'entete IP est variable, on recupere sa taille
            // dans les 4 premiers bits de l'entete IP
            struct iphdr* ip  = (struct iphdr*)buff;
            int           ttl = ip->ttl;

            // Taille de l'entete IP en octets
            // On recupere l'entete ICMP
            int             ip_header_len = ip->ihl * 4;
            struct icmphdr* icmp          = (struct icmphdr*)(buff + ip_header_len);

            unsigned char icmp_buf[8 + PAYLOAD_SIZE];
            memcpy(icmp_buf, icmp, 8 + PAYLOAD_SIZE);

            uint16_t original_checksum = icmp->checksum;
            uint16_t recv_seq          = ntohs(icmp->un.echo.sequence);

            client.send_time = (struct timeval*)(icmp_buf + 8);
            float rtt        = (client.recv_time->tv_sec - client.send_time->tv_sec) * 1000.0 +
                        (client.recv_time->tv_usec - client.send_time->tv_usec) / 1000.0;

            struct icmphdr* icmp_check = (struct icmphdr*)icmp_buf;
            icmp_check->checksum       = 0;
            uint16_t recv_checksum = icmp_checksum((unsigned char*)icmp_check, 8 + PAYLOAD_SIZE);

            if (recv_checksum != original_checksum || icmp->type != ICMP_ECHOREPLY ||
                recv_seq > client.seq || ntohs(icmp->un.echo.id) != (getpid() & 0xFFFF) ||
                icmp->code != 0 || ip->saddr != *(uint32_t*)client.infos->h_addr_list[0])
            {
                if (icmp->type == ICMP_DEST_UNREACH)
                {
                    fprintf(stderr,
                            "From %s icmp_seq=%d Destination Unreachable\n",
                            client.ip,
                            recv_seq);

                    client.counter.error++;
                    packet_received = 1;
                }
                else if (icmp->type == ICMP_TIME_EXCEEDED)
                {
                    fprintf(
                        stderr, "From %s icmp_seq=%d Time to live exceeded\n", client.ip, recv_seq);

                    client.counter.error++;
                    packet_received = 1;
                }
                else
                {
                    continue;
                }
            }
            else
            {

                // ! Reception valide de la reponse ICMP
                client.counter.received++;
                print_ping_line(ip, icmp, rtt, ttl);
                update_time_stats(&client.rtt, rtt, client.counter.transmitted);
                packet_received = 1;
            }
        }
        nanosleep(&client.delay_bt_pings, NULL);
    }
}