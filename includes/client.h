typedef struct client {

	int _fd;
	char* ip;
	
	struct hostent* infos;

} t_client;

#define PAYLOAD_SIZE 56

int build_echo_request(unsigned char* buff, int seq);
int icmp_checksum(unsigned char* buff, int len);