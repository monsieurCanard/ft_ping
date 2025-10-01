#include "../includes/ping.h"

int verify_response(t_ping_client *client, unsigned char *buff) {

		struct iphdr* ip = (struct iphdr*)buff;

		// Taille de l'entete IP en octets
		int ip_header_len = ip->ihl * 4;

		// On recupere l'entete ICMP
		struct icmphdr* icmp = (struct icmphdr*)(buff + ip_header_len);
	//Verification du checksum
	// On met a 0 le champ checksum avant de le recalculer

	unsigned char icmp_buf[8 + PAYLOAD_SIZE];
	memcpy(icmp_buf, icmp, 8 + PAYLOAD_SIZE);
	
	uint16_t original_checksum = icmp->checksum;
	
	struct icmphdr* icmp_check = (struct icmphdr*)icmp_buf;
	icmp_check->checksum = 0;
	
	uint16_t recv_checksum = icmp_checksum((unsigned char*)icmp_check, 8 + PAYLOAD_SIZE);

	if(recv_checksum != original_checksum) {
		printf("Received ICMP packet with invalid checksum: %d (expected: %d)\n", recv_checksum, original_checksum);
		return (ERROR);
	}

	if(icmp->type != ICMP_ECHOREPLY) {
		printf("Received ICMP packet with unexpected type: %d\n", icmp->type);
		return (ERROR);
	}

	// TODO: Voir pour gerer les doublons et affiche (DUP!) dans ce cas la
	uint16_t recv_seq = ntohs(icmp->un.echo.sequence);
	if (recv_seq > client->seq) {
		printf("Received ICMP packet with unexpected sequence number: %d, seq=%d\n", recv_seq, client->seq);
		return (ERROR);
	}

	if (ntohs(icmp->un.echo.id) != (getpid() & 0xFFFF)) {
		printf("Received ICMP packet with unexpected identifier: %d\n", ntohs(icmp->un.echo.id));
		return (ERROR);
	}

	if(icmp->code != 0) {
		printf("Received ICMP packet with unexpected code: %d\n", icmp->code);
		return (ERROR);
	}

	if (ip->saddr != *(uint32_t*)client->infos->h_addr_list[0]) {
		printf("Received ICMP packet from unexpected source: %s\n", inet_ntoa(*(struct in_addr*)&ip->saddr));
		return (ERROR);
	}

	return (SUCCESS);
}