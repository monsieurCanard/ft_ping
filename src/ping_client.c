#include "../includes/ping.h"

int create_client(t_ping_client *client, struct sockaddr_in *sockaddr, char *address) {


	client->infos = gethostbyname(address);
	if(!client->infos) {
		printf("ft_ping handle IPv4 adress and nothing else !\n");
		return (ERROR);
	}

	client->ip = inet_ntoa(*(struct in_addr*)client->infos->h_addr_list[0]);

	client->_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if(client->_fd < 0) {
		perror("Creation Socket");
		return(ERROR);
	}

	struct timeval timeout;
	timeout.tv_sec = TIMEOUT_SEC;  //On met un timeout de 1 seconde pour la reception des paquets // Nor
	timeout.tv_usec = TIMEOUT_USEC;
	setsockopt(client->_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

	memset(sockaddr, 0, sizeof(*sockaddr));
	sockaddr->sin_family = AF_INET;
	sockaddr->sin_port   = 0; // ICMP does not use ports, set to 0
	sockaddr->sin_addr.s_addr = inet_addr(client->ip);

	if(inet_aton(client->ip, &sockaddr->sin_addr) < 0)
	{
		perror("Inet_aton ");
		return (ERROR);
	}

	client->seq = 0;
	client->counter.transmitted = 0;
	client->counter.received = 0;
	client->counter.lost = 0;
	client->rtt.min = -1;
	client->rtt.max = -1;
	client->rtt.average = 0;
	client->rtt.mdev = 0;
	client->rtt.total = 0;
	client->rtt.total_sq = 0;

	return (0);
}

void update_time_stats(t_rtt *rtt, double new_rtt) {
	if (rtt->min == -1 || new_rtt < rtt->min) {
		rtt->min = new_rtt;
	}
	if (rtt->max == -1 || new_rtt > rtt->max) {
		rtt->max = new_rtt;
	}
	rtt->total += new_rtt;
	rtt->total_sq += new_rtt * new_rtt;
}