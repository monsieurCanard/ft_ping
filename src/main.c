#include "../includes/ping.h"

t_ping_client client;

void exit_program(int sig) {
	(void)sig;

	struct timeval end_time;
	gettimeofday(&end_time, NULL);

	double total_time = (end_time.tv_sec - client.start_time->tv_sec) * 1000.0 +
						(end_time.tv_usec - client.start_time->tv_usec) / 1000.0;

	double mdev = (client.counter.transmitted > 0) ? sqrt(client.rtt.delta / client.counter.transmitted) : 0.0;
	double success_rate = client.counter.transmitted == 0 ? 0.0 : ((client.counter.transmitted - client.counter.received) / (double)client.counter.transmitted) * 100.0;

	fprintf(stdout, "\n--- %s ping statistics ---\n", client.infos->h_name);
	fprintf(stdout, "%d packets transmitted, %d received, %.1f%% packet loss, time %.0f ms\n",
		client.counter.transmitted,
		client.counter.received,
		success_rate,
		total_time);

	if (client.counter.received > 0) {
		fprintf(stdout, "rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n",
			client.rtt.min,
			client.rtt.average,
			client.rtt.max,
			mdev);
	}

	if(client.start_time)
		free(client.start_time);

	if(client.recv_time)
		free(client.recv_time);
		
	if(client._fd >= 0)
		close(client._fd);

	exit(EXIT_SUCCESS);
}

int main(int ac, char **av) {
	
	if (ac != 2) {
		fprintf(stderr, "Wrong number of argument: Actual %d -> Need 1. Usage ./ft_ping [hostname/ip]", ac - 1);
		return (EXIT_FAILURE);
	}

	signal(SIGINT, exit_program);
	struct sockaddr_in sockaddr;
	
	int ret = create_client(&client, &sockaddr, av[1]);
	if (ret == ERROR) {
		return (ret);
	}

	printf("PING %s (%s) %d bytes of data.\n", av[1], client.ip, PAYLOAD_SIZE);
	
	unsigned char buff[8 + PAYLOAD_SIZE];

	while(1) {

		client.seq++;
		
		int payload_size = build_echo_request(buff, client.seq);
		if(payload_size == ERROR) {
			return (EXIT_FAILURE);
		}
		
		sendto(client._fd, buff, payload_size, 0, (struct sockaddr*)&sockaddr, sizeof(sockaddr));
		client.counter.transmitted++;
		
		socklen_t addrlen = sizeof(sockaddr);
		ret = recvfrom(client._fd, buff, payload_size, 0, (struct sockaddr*)&sockaddr, &addrlen);

		gettimeofday(client.recv_time, NULL);

		if(ret == ERROR) {
			
			if(errno == EAGAIN || errno == EWOULDBLOCK) {
				printf("Request timeout for icmp_seq %d\n", client.seq);
				client.counter.lost++;
				continue;
			} else {
				perror("Recvfrom ");
				return (1);
			}
		}

		// buff contient tout le paquet IP recu
		// On doit sauter l'entete IP pour acceder a l'entete ICMP
		// La taille de l'entete IP est variable, on recupere sa taille
		// dans les 4 premiers bits de l'entete IP
		struct iphdr* ip = (struct iphdr*)buff;
		int ttl = ip->ttl;

		
		// Taille de l'entete IP en octets
		// On recupere l'entete ICMP
		int ip_header_len = ip->ihl * 4;
		struct icmphdr* icmp = (struct icmphdr*)(buff + ip_header_len);

		unsigned char icmp_buf[8 + PAYLOAD_SIZE];
		memcpy(icmp_buf, icmp, 8 + PAYLOAD_SIZE);
		
		uint16_t original_checksum = icmp->checksum;
		uint16_t recv_seq = ntohs(icmp->un.echo.sequence);
		
		// if(client.send_time)
		// 	free(client.send_time);
		client.send_time = (struct timeval *)(icmp_buf + 8);
		float rtt = (client.recv_time->tv_sec - client.send_time->tv_sec) * 1000.0 +
								(client.recv_time->tv_usec - client.send_time->tv_usec) / 1000.0;


		
		struct icmphdr* icmp_check = (struct icmphdr*)icmp_buf;
		icmp_check->checksum = 0;
		uint16_t recv_checksum = icmp_checksum((unsigned char*)icmp_check, 8 + PAYLOAD_SIZE);

		if(recv_checksum != original_checksum || icmp->type != ICMP_ECHOREPLY || recv_seq > client.seq || ntohs(icmp->un.echo.id) != (getpid() & 0xFFFF) || icmp->code != 0 || ip->saddr != *(uint32_t*)client.infos->h_addr_list[0]) {
			printf("Received invalid ICMP packet\n");
			client.counter.lost++;
			continue;
		}
			
		client.counter.received++;

		struct in_addr addr;
		addr.s_addr = ip->saddr;
		struct hostent *host = gethostbyaddr(&addr, sizeof(addr), AF_INET);
		if (host)
				printf("%d bytes from %s (%s): icmp_seq=%d ttl=%d rtt=%.2f ms\n",
						PAYLOAD_SIZE + 8, host->h_name, inet_ntoa(addr),
						ntohs(icmp->un.echo.sequence), ttl, rtt);
		else
				printf("%d bytes from %s: icmp_seq=%d ttl=%d rtt=%.2f ms\n",
						PAYLOAD_SIZE + 8, inet_ntoa(addr),
						ntohs(icmp->un.echo.sequence), ttl, rtt);

			update_time_stats(&client.rtt, rtt, client.counter.transmitted);
			sleep(1);
			
	}
	return 0;
}