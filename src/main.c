#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <netinet/ip.h>


#include "../includes/client.h"


void handle_sigint() {
	//TODO: Ici on affichera les stats avant de quitter
	printf("Stop process");
	exit(0);
}

int main(int ac, char **av) {
	if (ac != 2) {
		printf("Wrong number of argument: Actual %d -> Need 1. Usage ./ft_ping [adresse]", ac - 1);
		return (1);
	}

	signal(SIGINT, handle_sigint);
	t_client client;

	// ft_ping 8.8.8.8
	// ft_ping google.com
	// ft_ping www.google.com

	client.infos = gethostbyname(av[1]);
	if(!client.infos) {
		printf("ft_ping handle IPv4 adress and nothing else !\n");
		return (1);
	}

	client.ip = inet_ntoa(*(struct in_addr*)client.infos->h_addr_list[0]);

	// printf("Adress ip du hostname : %s", client.ip);
	// printf("Arg %s\n", av[1]);
	
	
	client._fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if(client._fd < 0) {
		perror("Creation Socket");
		exit(1);
	}

	struct sockaddr_in sockaddr;
	memset(&sockaddr, 0, sizeof(sockaddr));

	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port   = 0; // ICMP does not use ports, set to 0


	if(inet_aton(client.ip, &sockaddr.sin_addr) < 0)
	{
		perror("Inet_aton ");
		return (1);
	}

	int seq = 1;
	unsigned char buff[8 + PAYLOAD_SIZE];
	printf("PING %s (%s) %d bytes of data.\n", av[1], client.ip, PAYLOAD_SIZE);
	
	while(1) {
		
		int payload_size = build_echo_request(buff, seq);

		sendto(client._fd, buff, payload_size, 0, (struct sockaddr*)&sockaddr, sizeof(sockaddr));
		
		socklen_t addrlen = sizeof(sockaddr);
		recvfrom(client._fd, buff, payload_size, 0, (struct sockaddr*)&sockaddr, &addrlen);

		// Buff contient tout le paquet IP recu
		// On doit sauter l'entete IP pour acceder a l'entete ICMP
		// La taille de l'entete IP est variable, on recupere sa taille
		// dans les 4 premiers bits de l'entete IP
		struct iphdr* ip = (struct iphdr*)buff;

		// Taille de l'entete IP en octets
		int ip_header_len = ip->ihl * 4;

		// On recupere l'entete ICMP
		struct icmphdr* icmp = (struct icmphdr*)(buff + ip_header_len);
		// int icmp_header_len = sizeof(struct icmphdr);
		
		//Verification du checksum
		// On copie l'entete ICMP dans un buffer temporaire pour recalculer le checksum
		// On met a 0 le champ checksum avant de le recalculer
		unsigned char icmp_buf[8 + PAYLOAD_SIZE];
		memcpy(icmp_buf, icmp, 8 + PAYLOAD_SIZE);
		struct icmphdr* icmp_check = (struct icmphdr*)icmp_buf;
		icmp_check->checksum = 0;
		uint16_t recv_checksum = icmp_checksum((unsigned char*)icmp_check, 8 + PAYLOAD_SIZE);


		if(recv_checksum != icmp->checksum) {
			printf("Received ICMP packet with invalid checksum: %d\n", icmp->checksum);
			continue;
		}

		if(icmp->type != ICMP_ECHOREPLY) {
			printf("Received ICMP packet with unexpected type: %d\n", icmp->type);
			continue;
		}

		// if (ntohs(icmp->un.echo.sequence) != seq) {
		// 	printf("Received ICMP packet with unexpected sequence number: %d\n", ntohs(icmp->un.echo.sequence));
		// 	continue;
		// }

		if (ntohs(icmp->un.echo.id) != (getpid() & 0xFFFF)) {
			printf("Received ICMP packet with unexpected identifier: %d\n", ntohs(icmp->un.echo.id));
			continue;
		}


		if(icmp->code != 0) {
			printf("Received ICMP packet with unexpected code: %d\n", icmp->code);
			continue;
		}

		if (ip->saddr != *(uint32_t*)client.infos->h_addr_list[0]) {
			printf("Received ICMP packet from unexpected source: %s\n", inet_ntoa(*(struct in_addr*)&ip->saddr));
			continue;
		}

		int ttl = ip->ttl;
		printf("%d bytes from %s: icmp_seq=%d ttl=%d\n", PAYLOAD_SIZE, client.ip, seq, ttl);

		sleep(1);
		
		seq++;
	}
	return 0;
}