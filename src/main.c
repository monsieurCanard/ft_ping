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


#include "../includes/client.h"


void handle_sigint() {
	//TODO: Ici on affichera les stats avant de quitter
	printf("Stop process");
	return;
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

	write(1, "coucou", 6);
	client.ip = inet_ntoa(*(struct in_addr*)client.infos->h_addr_list[0]);

	printf("Adress ip du hostname : %s", client.ip);
	printf("Arg %s\n", av[1]);
	
	
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

	if (connect(client._fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0)
	{
		perror("Connect Socket");
		return 1;
	}

	unsigned char buff[8 + PAYLOAD_SIZE];
	int payload_size = build_echo_request(buff);

	sendto(client._fd, buff,payload_size, 0, (struct sockaddr*)&sockaddr, sizeof(sockaddr));
	
	while(1) {


	}
	return 0;
}