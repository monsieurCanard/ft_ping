#include "../includes/ping.h"

static t_ping_client client; 

void handle_sigint(int sig) {
	(void)sig;

	struct timeval end_time;
	gettimeofday(&end_time, NULL);
	double total_time = (end_time.tv_sec - client.start_time->tv_sec) * 1000.0 +
						(end_time.tv_usec - client.start_time->tv_usec) / 1000.0;


	client.rtt.average = client.rtt.total / client.counter.received;
	client.rtt.mdev = sqrt(client.rtt.total_sq / client.counter.received - client.rtt.average * client.rtt.average);

	// (void)total_time;
	printf("\n--- %s ping statistics ---\n", client.infos->h_name);
	printf("%d packets transmitted, %d received, %.1f%% packet loss, time %.0f ms\n",
		client.counter.transmitted,
		client.counter.received,
		client.counter.transmitted == 0 ? 0.0 : ((client.counter.transmitted - client.counter.received) / (double)client.counter.transmitted) * 100.0,
		total_time);


	if (client.counter.received > 0) {
		// client.rtt.mdev = client.rtt.total / client.counter.received;
		printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n",
			client.rtt.min,
			client.rtt.average,
			client.rtt.max,
			client.rtt.mdev);
	}
	exit(0);
}

int main(int ac, char **av) {
	
	if (ac != 2) {
		printf("Wrong number of argument: Actual %d -> Need 1. Usage ./ft_ping [hostname/ip]", ac - 1);
		return (1);
	}

	signal(SIGINT, handle_sigint);
	struct sockaddr_in sockaddr;
	
	// memset(client, 0, sizeof(t_ping_client));
	int ret = create_client(&client, &sockaddr, av[1]);
	if (ret == ERROR) {
		return (ret);
	}

	client.start_time = malloc(sizeof(struct timeval));
	gettimeofday(client.start_time, NULL);
	unsigned char buff[8 + PAYLOAD_SIZE];
	printf("PING %s (%s) %d bytes of data.\n", av[1], client.ip, PAYLOAD_SIZE);

	while(1) {
		client.seq++;
		int payload_size = build_echo_request(buff, client.seq);

		sendto(client._fd, buff, payload_size, 0, (struct sockaddr*)&sockaddr, sizeof(sockaddr));
		client.counter.transmitted++;
		
		socklen_t addrlen = sizeof(sockaddr);
		ret = recvfrom(client._fd, buff, payload_size, 0, (struct sockaddr*)&sockaddr, &addrlen);
		struct timeval recv_time;
		gettimeofday(&recv_time, NULL);
		

		
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

		// Buff contient tout le paquet IP recu
		// On doit sauter l'entete IP pour acceder a l'entete ICMP
		// La taille de l'entete IP est variable, on recupere sa taille
		// dans les 4 premiers bits de l'entete IP
		struct iphdr* ip = (struct iphdr*)buff;
		int ttl = ip->ttl;

		// Taille de l'entete IP en octets
		int ip_header_len = ip->ihl * 4;

		// On recupere l'entete ICMP
		struct icmphdr* icmp = (struct icmphdr*)(buff + ip_header_len);
		//Verification du checksum
		// On met a 0 le champ checksum avant de le recalculer

		unsigned char icmp_buf[8 + PAYLOAD_SIZE];
		memcpy(icmp_buf, icmp, 8 + PAYLOAD_SIZE);
		
		uint16_t original_checksum = icmp->checksum;
		uint16_t recv_seq = ntohs(icmp->un.echo.sequence);
		
		struct timeval *sent_time = (struct timeval *)(icmp_buf + 8);
		float rtt = (recv_time.tv_sec - sent_time->tv_sec) * 1000.0 +
								(recv_time.tv_usec - sent_time->tv_usec) / 1000.0;


		
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
						PAYLOAD_SIZE, host->h_name, inet_ntoa(addr),
						ntohs(icmp->un.echo.sequence), ttl, rtt);
		else
				printf("%d bytes from %s: icmp_seq=%d ttl=%d rtt=%.2f ms\n",
						PAYLOAD_SIZE, inet_ntoa(addr),
						ntohs(icmp->un.echo.sequence), ttl, rtt);

			update_time_stats(&client.rtt, rtt);
			sleep(1);
			
	}
	return 0;
}