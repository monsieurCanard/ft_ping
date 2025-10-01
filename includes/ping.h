#include <stdio.h>
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
#include <math.h>

#define PAYLOAD_SIZE 56
#define TIMEOUT_SEC 1
#define TIMEOUT_USEC 0

#define ERROR -1
#define SUCCESS 0


typedef struct rtt {
	double min;
	double max;
	double average;
	double mdev;

	double total;
	double delta;
} t_rtt;

typedef struct counter {
	int transmitted;
	int received;
	int lost;
} t_counter;

typedef struct ping_client {

	int 		_fd;
	char* 	ip;
	int 		seq;
	
	struct timeval *start_time;
	struct timeval *send_time;
	struct timeval *recv_time;
	struct hostent* infos;
	
	t_rtt rtt;
	t_counter counter;

} t_ping_client;


int create_client(t_ping_client *client, struct sockaddr_in *sockaddr, char *address);

int build_echo_request(unsigned char* buff, int seq);
int icmp_checksum(unsigned char* buff, int len);

void update_time_stats(t_rtt *rtt, double new_rtt, int count);