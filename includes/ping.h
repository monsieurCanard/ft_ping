#include <arpa/inet.h>
#include <bits/types/struct_timeval.h>
#include <errno.h>
#include <getopt.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
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

#define PAYLOAD_SIZE   56
#define MAX_PING_SAVES 1024

#define RESEND 1

#define TIMEOUT_SEC  5
#define TIMEOUT_USEC 0

#define SECOND_PAUSE_BT_PINGS 1

#define ERROR   -1
#define SUCCESS 0

#define SEND    0
#define RECEIVE 1
#define TIMEOUT 2

typedef struct time_stats
{
    double min;
    double max;
    double average;

    double total;
    double delta;

} t_time_stats;

enum OPT_ARGS
{
    OPT_VERBOSE  = 1 << 0,
    OPT_TTL      = 1 << 1,
    OPT_INTERVAL = 1 << 2,
    OPT_COUNT    = 1 << 3,
    OPT_LINGER   = 1 << 4,
    OPT_TIMEOUT  = 1 << 5
};

typedef struct ping_counter
{
    int transmitted;
    int received;
    int dup;
    int lost;
} t_ping_counter;

typedef struct icmp_packet
{
    int receive;
} t_icmp_packet;

typedef struct data_icmp
{
    struct iphdr*   ip_header;
    struct icmphdr* data;
} t_data_icmp;

typedef struct args
{
    int           all_args;
    enum OPT_ARGS args;

    int ttl;
    int linger;
    int interval;
    int count;
    int timeout;
} t_args;

typedef struct ping_client
{
    t_args             args;
    struct sockaddr_in sockaddr;
    t_icmp_packet*     packets;
    fd_set             read_fds;

    char* name;
    int   fd;
    char* ip;
    int   seq;
    int   status;

    struct timespec delay_bt_pings;

    t_time_stats   time_stats;
    t_ping_counter counter;

} t_ping_client;

/// * PARSING AND SETUP FUNCTIONS
int parse_args(int ac, char** av, t_ping_client* client);
int create_client(t_ping_client* client, char* address);

/// * ICMP MESSAGE CREATION
int build_echo_request(t_ping_client* client, unsigned char* buff);
int icmp_checksum(unsigned char* buff, int len);

/// * MAIN LOOP AND HANDLERS
void  main_loop_icmp(t_ping_client* client);
float verify_response_and_print(t_ping_client* client,
                                unsigned char* buff,
                                struct timeval recv_time);

int time_checker(t_ping_client*  client,
                 struct timeval* start_time,
                 struct timeval* now,
                 struct timeval* send_time);

bool resend_packet(t_ping_client* client, struct timeval* now, struct timeval* time);
void handle_error_icmp(t_data_icmp icmp, t_ping_client* client);
void update_client_time_stats(t_time_stats* time_stats, double new_rtt, int count);

/// * PRINTING FUNCTIONS
void print_ping_final_stats(t_ping_client* client, double success_rate, double mdev, int total_msg);
void print_ping_line(t_data_icmp icmp, float rtt, int ttl, bool dup);
void print_start_ping(t_ping_client* client);

/// * TIMESTAMP FUNCTIONS
struct timeval add_timestamp(struct timeval* time1, struct timeval* time2);
struct timeval sub_timestamp(struct timeval* time1, struct timeval* time2);

/// * EXIT AND HELPERS
void exit_program(t_ping_client* client);
void print_helper();
