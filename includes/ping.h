#include <arpa/inet.h>
#include <errno.h>
#include <math.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define PAYLOAD_SIZE 56
#define MAX_PINGS    1024

// Timeout for each ping reply
#define TIMEOUT_SEC  1
#define TIMEOUT_USEC 0

// Pause between pings
#define SECOND_PAUSE_BT_PINGS     1
#define NANOSECOND_PAUSE_BT_PINGS 0

#define ERROR   -1
#define SUCCESS 0

typedef struct rtt
{
    double min;
    double max;
    double average;
    double mdev;

    double total;
    double delta;
} t_rtt;

typedef struct counter
{
    int transmitted;
    int received;
    int error;
    int lost;
} t_counter;

typedef struct icmp_packet
{
    struct timeval send_time;
    int            received;
} t_icmp_packet;

typedef struct ping_client
{
    t_icmp_packet* packet;

    int      fd;
    char*    ip;
    uint32_t target_addr; // Adresse IP cible sauvegardée
    int      seq;
    int      status;

    struct timespec delay_bt_pings;
    struct timeval* start_time;
    struct timeval* send_time;
    struct hostent* infos;

    t_rtt     rtt;
    t_counter counter;

} t_ping_client;

int create_client(t_ping_client* client, struct sockaddr_in* sockaddr, char* address);

int build_echo_request(t_ping_client* client, unsigned char* buff);

int icmp_checksum(unsigned char* buff, int len);

void print_ping_infos(t_ping_client* client, double total_time, double success_rate, double mdev);

void update_time_stats(t_rtt* rtt, double new_rtt, int count);

void print_ping_line(
    struct iphdr* ip, struct icmphdr* icmp, float rtt, int ttl, t_icmp_packet* packet);

void main_loop_icmp(t_ping_client* client, struct sockaddr_in sockaddr);

void verify_packet(t_ping_client* client);

void exit_program(t_ping_client* client);