#include "../includes/ping.h"

int icmp_checksum(unsigned char* buff, int len)
{
    const uint16_t* data = (uint16_t*)buff;
    int             sum  = 0;

    // Le checksum est la somme de tout les mots de 16 bits
    // On additionne tout les bits du paquet mais 2 par 2 pour correspondre a la norme ICMP
    while (len > 1)
    {
        sum += ntohs(*data++);
        len -= 2;
    }

    // Si il reste un octet a la fin
    if (len == 1)
    {
        uint16_t last    = 0;
        *(uint8_t*)&last = *(const uint8_t*)data;
        sum += last;
    }

    // Si la somme depasse 16 bits, on fold
    // sum & 0XFFFF on garde les 16 bits de poids faible
    // sum >> 16 : et on ajouter les bits qui depasses 16 bits
    while (sum >> 16)
    {
        sum = (sum & 0XFFFF) + (sum >> 16);
    }
    // Le checksum ICMP est defini comme le complement a un de la sommes
    // Le complement consiste a inverser tous les bits
    return htons(~sum);
}

int build_echo_request(unsigned char* buff, int seq)
{

    struct icmphdr* icmph = (struct icmphdr*)buff;

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
    // Htons convertit en network byte order pour eviter les problemes
    icmph->un.echo.id = htons(getpid() & 0XFFFF);

    // Numero de sequence du paquet
    // Si je devais envoyer plusieurs ping je pourrai incrementer cette valeur
    icmph->un.echo.sequence = htons(seq);

    // On remplit le payload avec un timestamp
    if (PAYLOAD_SIZE < sizeof(struct timeval))
    {
        fprintf(stderr, "Payload size too small for timestamp\n");
        return (ERROR);
    }

    struct timeval tv;
    gettimeofday(&tv, NULL);
    memcpy(buff + 8, &tv, sizeof(tv));

    // On remplit le reste du payload avec des zeros
    for (int i = 8 + sizeof(tv); i < 8 + 56; ++i)
    {
        buff[i] = 0;
    }

    // On calcul la taille du paquets
    icmph->checksum = icmp_checksum(buff, 8 + PAYLOAD_SIZE);

    return PAYLOAD_SIZE + 8;
}