
#include <netinet/ip_icmp.h>
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

#define PAYLOAD_SIZE 56

int build_echo_request(unsigned char* buff) {

	struct icmphdr *icmph = (struct icmphdr*)buff;
	
	// Payload Size = 56
	// ICMP header = 8
	memset(buff, 0, 8 + PAYLOAD_SIZE);

	// Type du message ICMP 
	// 8 = Echo Request 
	// 0 = echo reply
	icmph->type = ICMP_ECHO;

	// Precision du type ICMP
	// Certains ICMP utilisent ce code pour preciser les evenements
	icmph->code = 0;

	// Sert a verifier l'integrite du paquet
	// On met juste 0 temporairement
	icmph->checksum = 0;

	// Identifiant du paquet pour le programme
	// On prend le pid du programme et on garde seulement les 16 bits de poids faible
	//Htons convertit en network byte order pour eviter les problemes
	icmph->un.echo.id = htons(getpid() & 0XFFFF);

	// Numero de sequence du paquet
	// Si je devais envoyer plusieurs ping je pourrai incrementer cette valeur
	icmph->un.echo.sequence = 1;


	// On remplit le payload avec un timestamp
	//TODO : Verifier si j'ai la taille dans mon payload pour le timestamp
	struct timeval tv;
	gettimeofday(&tv, NULL);
	memcpy(buff + 8, &tv, sizeof(tv));
	
	//On remplit le reste du payload avec des zeros
	for(int i = 8 + sizeof(tv); i < 8 + 56; ++i) {
		buff[i] = 0;
	}

	// On calcul la taille du paquets
	icmph->checksum = icmp_checksum(buff, 8 + PAYLOAD_SIZE);
	
	return 8 + PAYLOAD_SIZE;
}