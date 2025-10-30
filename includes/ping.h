#include <arpa/inet.h>
#include <errno.h>
#include <getopt.h>
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

#define TIMEOUT_SEC  10
#define TIMEOUT_USEC 0

#define SECOND_PAUSE_BT_PINGS     1
#define NANOSECOND_PAUSE_BT_PINGS 0

#define ERROR   -1
#define SUCCESS 0

typedef struct time_stats
{
    double min;
    double max;
    double average;

    double total;
    double delta;

} t_time_stats;

typedef struct args
{
    bool verbose;
    int  debug_level;
    int  ttl;
    int  interval;
    int  count;
    int  timeout;

} t_args;

typedef struct ping_counter
{
    int transmitted;
    int received;
    int error;
    int lost;
} t_ping_counter;

typedef struct icmp_packet
{
    struct timeval send_time;
    int            received;
} t_icmp_packet;

typedef struct ping_client
{
    t_args             args;
    struct sockaddr_in sockaddr;
    t_icmp_packet*     packet;

    char*    name;
    int      fd;
    char*    ip;
    int      seq;
    int      status;
    uint32_t target_addr;

    struct timespec delay_bt_pings;

    t_time_stats   time_stats;
    t_ping_counter counter;

} t_ping_client;

int parse_args(int ac, char** av, t_ping_client* client);

int create_client(t_ping_client* client, char* address);

int build_echo_request(t_ping_client* client, unsigned char* buff);

int icmp_checksum(unsigned char* buff, int len);

void print_ping_infos(t_ping_client* client, double success_rate, double mdev);

void update_client_time_stats(t_time_stats* time_stats, double new_rtt, int count);

void print_ping_line(
    struct iphdr* ip, struct icmphdr* icmp, float rtt, int ttl, t_icmp_packet* packet);

void main_loop_icmp(t_ping_client* client);

float verify_response(t_ping_client* client, unsigned char* buff, struct timeval recv_time);

void handle_error_icmp(struct icmphdr* icmp, struct iphdr* ip, t_ping_client* client);

void exit_program(t_ping_client* client);

void print_helper();

void print_error(char* msg);