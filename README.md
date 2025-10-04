<img width="2000" height="626" alt="Image" src="https://github.com/user-attachments/assets/0dbafb7c-d9d8-4e97-bff8-40c80c1a43e5" />

# ft_ping

Recréation de la commande `ping` (inetutils-2.0)

---

## Description

Ce projet vise à recréer le fonctionnement de la commande `ping` : envoi de paquets ICMP Echo Request pour tester la connectivité réseau.

---

## Exemple d'utilisation

```bash
ping google.com
```

La commande envoie des paquets **ICMP Echo Request** à l'adresse IP spécifiée (ex : `8.8.8.8`). Le serveur répond avec des paquets **ICMP Echo Reply**, confirmant la connectivité.

---

## Affichage typique

```bash
ping 8.8.8.8
PING 8.8.8.8 (8.8.8.8) 56(84) bytes of data.
64 bytes from 8.8.8.8: icmp_seq=1 ttl=116 time=1.32 ms
64 bytes from 8.8.8.8: icmp_seq=2 ttl=116 time=1.67 ms
64 bytes from 8.8.8.8: icmp_seq=3 ttl=116 time=1.64 ms
64 bytes from 8.8.8.8: icmp_seq=4 ttl=116 time=1.39 ms
64 bytes from 8.8.8.8: icmp_seq=5 ttl=116 time=1.70 ms
64 bytes from 8.8.8.8: icmp_seq=6 ttl=116 time=1.41 ms
64 bytes from 8.8.8.8: icmp_seq=7 ttl=116 time=1.36 ms
^C
--- 8.8.8.8 ping statistics ---
7 packets transmitted, 7 received, 0% packet loss, time 6011ms
rtt min/avg/max/mdev = 1.316/1.497/1.696/0.150 ms
```

---

## Détail de l'affichage

### En-tête de lancement

```bash
PING 8.8.8.8 (8.8.8.8) 56(84) bytes of data.
```

- **PING 8.8.8.8** : ping de l’IP 8.8.8.8
- **(8.8.8.8)** : adresse IP résolue (si `ping google.com`, affiche l’IP)
- **56 bytes** : taille de la charge utile ICMP envoyée (par défaut)
- **(84) bytes** : taille totale du paquet IP = 56 (données) + 8 (ICMP) + 20 (IP header) = 84 octets

### Réponse de chaque paquet

```bash
64 bytes from 8.8.8.8: icmp_seq=1 ttl=116 time=1.32 ms
```

- **64 bytes** : taille du paquet reçu (données + en-tête ICMP)
- **from 8.8.8.8** : adresse source ayant répondu
- **icmp_seq=1** : numéro de séquence ICMP (incrémenté à chaque ping)
- **ttl=116** : Time To Live (décrémenté à chaque routeur traversé)
	- Valeur initiale dépend du système distant (souvent 64, 128, 255)
	- Ici, TTL initial probable : 128 → 12 sauts
- **time=1.32 ms** : round-trip time (RTT), délai aller-retour

### Statistiques finales

```bash
--- 8.8.8.8 ping statistics ---
7 packets transmitted, 7 received, 0% packet loss, time 6011ms
rtt min/avg/max/mdev = 1.316/1.497/1.696/0.150 ms
```

- **7 packets transmitted** : requêtes ICMP envoyées
- **7 received** : réponses reçues
- **0% packet loss** : aucun paquet perdu
- **time 6011ms** : durée totale du test
- **min = 1.316 ms** : plus petite latence
- **avg = 1.497 ms** : latence moyenne
- **max = 1.696 ms** : latence la plus élevée
- **mdev = 0.150 ms** : moyenne de l’écart absolu (stabilité du lien)

---

## Struct Hostent

La structure struct hostent est utilisée en C pour représenter une entrée de la base de données des hôtes (résolution DNS). Elle contient les informations sur un nom d’hôte ou une adresse IP.

Voici sa description :

- **char *h_name**: nom officiel de l’hôte.
- **char **h_aliases**: liste des alias (noms alternatifs).
- **int h_addrtype**: type d’adresse (généralement AF_INET pour IPv4).
- **int h_length**: longueur de l’adresse en octets.
- **char **h_addr_list**: tableau des adresses IP associées à l’hôte (sous forme binaire).

En général, pour récupérer l’adresse IP principale :
((struct in_addr *)h->h_addr_list[0])

## Construire une echo Request ICMP
```c
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
```

## Endianness (ordre des octets)

Un ordi peut stocker les entiers en little endian (Intel, AMD) ou big endian (réseaux, certaines architectures).

**Little endian** → l’octet de poids faible vient en premier en mémoire.

**Big endian** → l’octet de poids fort vient en premier.

En réseau, on utilise toujours le format big endian, qu’on appelle network byte order.
Mais sur ton PC (Intel/Linux), c’est du little endian.

Du coup, quand on écrit/lit un champ dans un paquet, on doit convertir.

## Fonctions de conversion

**htons(x)** → host to network short (16 bits → uint16_t)

**htonl(x)** → host to network long (32 bits → uint32_t)

**ntohs(x)** → network to host short (16 bits → uint16_t)

**ntohl(x)** → network to host long (32 bits → uint32_t)
