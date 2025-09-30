typedef struct client {

	int _fd;
	char* ip;
	
	struct hostent* infos;

} t_client;


int build_echo_request(unsigned char* buff);