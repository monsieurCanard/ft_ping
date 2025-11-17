<div align="center">
<img width="1000" height="350" alt="Image" src="https://github.com/user-attachments/assets/fd3ee28b-e526-46ea-bc16-213eb72ab26e" />
</div>

# ft_ping

Recréation complète de la commande `ping` (compatible inetutils-2.X) en C avec gestion des sockets RAW et du protocole ICMP.

---

## 📋 Description

Ce projet implémente un outil de diagnostic réseau permettant d'envoyer des paquets **ICMP Echo Request** et de mesurer la latence et la disponibilité d'un hôte distant. Le programme gère la résolution DNS, le calcul de checksum ICMP, les timeouts, et les statistiques détaillées de latence.

### ⚠️ Permissions requises

Ce programme nécessite des **privilèges root** pour créer des sockets RAW :

```bash
sudo ./ft_ping [options] <destination>
```

Cependant, vous pouvez voir les options disponibles sans privilèges root :

```bash
sudo ./ft_ping -?
```
ou 
```bash
./ft_ping --help
```

---

## 🔧 Compilation

```bash
make        # Compile le projet
make clean  # Supprime les fichiers objets
make fclean # Supprime les fichiers objets et l'exécutable
make re     # Recompile complètement le projet
```

L'exécutable `ft_ping` sera créé à la racine du projet.

**Flags de compilation** : `-Wall -Wextra -Werror`

---

## 🚀 Options disponibles

| Option | Description |
|--------|-------------|
| `-v, --verbose` | Active le mode verbeux (affiche les doublons, erreurs, paquets tardifs, ID du processus) |
| `-t, --ttl N` | Définit le TTL (Time To Live) des paquets à N hops |
| `-i, --interval N` | Définit l'intervalle entre chaque ping à N secondes |
| `-c, --count N` | Arrête après avoir reçu N réponses |
| `-W, --linger N` | Temps d'attente pour une réponse (en secondes) |
| `-w, --timeout N` | Temps total d'exécution du programme (en secondes) |
| `-?, --help` | Affiche l'aide et quitte |

---

## 💡 Exemples d'utilisation

### Ping basique
```bash
sudo ./ft_ping google.com
```

### Limiter le nombre de paquets reçus
```bash
sudo ./ft_ping -c 5 8.8.8.8
```

### Définir un TTL personnalisé
```bash
sudo ./ft_ping -t 64 google.com
```

### Mode verbeux avec intervalle personnalisé
```bash
sudo ./ft_ping -v -i 2 1.1.1.1
```

### Timeout global de 10 secondes
```bash
sudo ./ft_ping -w 10 google.com
```

### Combiner plusieurs options
```bash
sudo ./ft_ping -v -t 128 -c 10 -i 0.5 cloudflare.com
```

---

## ✨ Fonctionnalités implémentées

✅ **Résolution DNS** : Support des noms d'hôte et adresses IP avec `getaddrinfo()`  
✅ **Sockets RAW** : Création et gestion de sockets `SOCK_RAW` avec protocole `IPPROTO_ICMP`  
✅ **Construction ICMP** : Génération complète de paquets ICMP Echo Request  
✅ **Checksum ICMP** : Calcul et vérification du checksum selon RFC 792  
✅ **Timeouts** : Gestion des timeouts avec `select()` et détection des paquets perdus  
✅ **Détection avancée** : Identification des doublons, paquets tardifs, et erreurs ICMP  
✅ **Statistiques** : Calcul de min/avg/max/stddev (écart-type)  
✅ **TTL personnalisé** : Configuration du Time To Live via `setsockopt()`
✅ **Options multiples** : Support complet des options via `getopt_long()`  
✅ **Gestion des signaux** : Interception propre de `SIGINT` (Ctrl+C)  
✅ **Mode verbeux** : Affichage détaillé pour le débogage  
✅ **Gestion d'erreurs** : Détection des erreurs ICMP (Destination Unreachable, etc.)

---

## 📊 Exemple de sortie

```bash
sudo ./ft_ping 8.8.8.8
PING 8.8.8.8 (8.8.8.8) 56(84) bytes of data.
64 bytes from 8.8.8.8: icmp_seq=1 ttl=116 rtt=1.32 ms
64 bytes from 8.8.8.8: icmp_seq=2 ttl=116 rtt=1.67 ms
64 bytes from 8.8.8.8: icmp_seq=3 ttl=116 rtt=1.64 ms
64 bytes from 8.8.8.8: icmp_seq=4 ttl=116 rtt=1.39 ms
64 bytes from 8.8.8.8: icmp_seq=5 ttl=116 rtt=1.70 ms
^C
--- 8.8.8.8 ping statistics ---
5 packets transmitted, 5 packets received, 0.0% packet loss
round-trip min/avg/max/stddev = 1.320/1.544/1.700/0.171 ms
```

### Mode verbeux activé
```bash
sudo ./ft_ping -v google.com
PING google.com (142.250.185.46) 56(84) data bytes id 0x1a2b = 6699
64 bytes from 142.250.185.46: icmp_seq=1 ttl=118 rtt=12.45 ms
64 bytes from 142.250.185.46: icmp_seq=2 ttl=118 rtt=11.23 ms
```

---

## 📖 Détail de l'affichage

### En-tête de lancement

```bash
PING 8.8.8.8 (8.8.8.8) 56(84) bytes of data.
```

- **PING 8.8.8.8** : Adresse cible (nom d'hôte ou IP)
- **(8.8.8.8)** : Adresse IP résolue (utile si un nom d'hôte est fourni)
- **56 bytes** : Taille de la charge utile ICMP (payload)
- **(84) bytes** : Taille totale = 56 (données) + 8 (en-tête ICMP) + 20 (en-tête IP) = 84 octets

### Réponse de chaque paquet

```bash
64 bytes from 8.8.8.8: icmp_seq=1 ttl=116 rtt=1.32 ms
```

- **64 bytes** : Taille du paquet reçu (données + en-tête ICMP)
- **from 8.8.8.8** : Adresse source de la réponse
- **icmp_seq=1** : Numéro de séquence ICMP (incrémenté à chaque envoi)
- **ttl=116** : Time To Live restant (décrémenté à chaque routeur)
  - Valeur initiale typique : 64, 128, ou 255
  - Exemple : TTL=116 → environ 12 sauts (128 - 116 = 12 routeurs traversés)
- **rtt=1.32 ms** : Round-Trip Time (temps aller-retour)

### Statistiques finales

```bash
--- 8.8.8.8 ping statistics ---
5 packets transmitted, 5 packets received, 0.0% packet loss
round-trip min/avg/max/stddev = 1.320/1.544/1.700/0.171 ms
```

- **packets transmitted** : Nombre de requêtes ICMP envoyées
- **packets received** : Nombre de réponses reçues
- **packet loss** : Pourcentage de paquets perdus
- **min** : Latence minimale observée
- **avg** : Latence moyenne (somme RTT / nombre de paquets)
- **max** : Latence maximale observée
- **stddev** : Écart-type (standard deviation - mesure la stabilité de la connexion)

---

## 🏗️ Architecture du code

### Structure des fichiers

```
ft_ping/
├── includes/
│   └── ping.h              # Définitions structures, prototypes, constantes
├── src/
│   ├── main.c              # Point d'entrée, vérification root, gestion signal
│   ├── parser_arg.c        # Parsing des options (getopt_long)
│   ├── ping_client.c       # Création du client, résolution DNS, socket
│   ├── builder.c           # Construction paquets ICMP, calcul checksum
│   ├── loop.c              # Boucle principale d'envoi/réception
│   ├── verifier.c          # Vérification réponses, gestion timeouts
│   ├── printer.c           # Affichage des résultats
│   ├── timestamp.c         # Gestion des timestamps et timeval
│   ├── exit.c              # Nettoyage et libération des ressources
│   └── error.c             # Gestion des erreurs ICMP
└── Makefile                # Compilation automatique
```

### Structures principales

#### `t_ping_client`
Structure centrale contenant toutes les informations du client :
- Arguments et options (`t_args`)
- Socket et adresse (`sockaddr_in`)
- Statistiques de temps (`t_time_stats`)
- Compteurs de paquets (`t_ping_counter`)
- Tableau de paquets sauvegardés (MAX_PING_SAVES = 1024)

#### `t_time_stats`
Statistiques de latence :
- `min`, `max`, `average` : Latences observées
- `total` : Somme totale des RTT
- `delta` : Utilisé pour le calcul de l'écart-type

#### `t_ping_counter`
Compteurs de paquets :
- `transmitted` : Paquets envoyés
- `received` : Paquets reçus avec succès
- `error` : Erreurs rencontrées
- `lost` : Paquets perdus (timeout)

---

## 🔬 Concepts techniques

### Struct Hostent

La structure `struct hostent` représente une entrée de la base de données des hôtes (résolution DNS) :

```c
struct hostent {
    char  *h_name;       // Nom officiel de l'hôte
    char **h_aliases;    // Liste des alias (noms alternatifs)
    int    h_addrtype;   // Type d'adresse (AF_INET pour IPv4)
    int    h_length;     // Longueur de l'adresse en octets
    char **h_addr_list;  // Tableau des adresses IP (format binaire)
};
```

Pour récupérer l'adresse IP principale :
```c
struct in_addr *addr = (struct in_addr *)h->h_addr_list[0];
```

**Note** : Ce projet utilise `getaddrinfo()` (API moderne) plutôt que `gethostbyname()` (obsolète).

---

### Endianness (ordre des octets)

Les ordinateurs stockent les entiers soit en **little endian** (Intel, AMD) soit en **big endian** (réseau, certaines architectures).

- **Little endian** : L'octet de poids faible est stocké en premier en mémoire
- **Big endian** : L'octet de poids fort est stocké en premier

Le réseau utilise toujours le format **big endian** (network byte order).  
Sur PC (Intel/Linux), c'est du **little endian** (host byte order).

**⚠️ Il faut donc convertir lors des échanges réseau !**

#### Fonctions de conversion

| Fonction | Description |
|----------|-------------|
| `htons(x)` | **H**ost **to** **N**etwork **S**hort (16 bits → uint16_t) |
| `htonl(x)` | **H**ost **to** **N**etwork **L**ong (32 bits → uint32_t) |
| `ntohs(x)` | **N**etwork **to** **H**ost **S**hort (16 bits → uint16_t) |
| `ntohl(x)` | **N**etwork **to** **H**ost **L**ong (32 bits → uint32_t) |

**Exemple** :
```c
icmph->un.echo.id = htons(getpid() & 0xFFFF);  // Conversion avant envoi
uint16_t recv_seq = ntohs(icmp->un.echo.sequence);  // Conversion après réception
```

---

### Construction d'un paquet ICMP Echo Request

#### Structure d'un paquet ICMP

```
+------------------+
| En-tête IP       |  20 octets (minimum)
+------------------+
| En-tête ICMP     |  8 octets
+------------------+
| Données (payload)|  56 octets (par défaut)
+------------------+
Total : 84 octets
```

#### En-tête ICMP (8 octets)

```c
struct icmphdr {
    uint8_t  type;       // Type du message (8 = Echo Request, 0 = Echo Reply)
    uint8_t  code;       // Code spécifique au type (0 pour Echo)
    uint16_t checksum;   // Checksum pour vérifier l'intégrité
    union {
        struct {
            uint16_t id;        // Identifiant du processus
            uint16_t sequence;  // Numéro de séquence
        } echo;
    } un;
};
```

#### Code de construction (extrait de `builder.c`)

```c
int build_echo_request(t_ping_client* client, unsigned char* buff) {
    struct icmphdr *icmph = (struct icmphdr*)buff;
    
    // Initialisation du buffer (8 octets header + 56 octets payload)
    memset(buff, 0, 8 + PAYLOAD_SIZE);
    
    // Configuration de l'en-tête ICMP
    icmph->type = ICMP_ECHO;           // 8 = Echo Request
    icmph->code = 0;                   // Toujours 0 pour Echo Request
    icmph->checksum = 0;               // Temporaire, calculé après
    
    // Identifiant : PID du processus (16 bits de poids faible)
    icmph->un.echo.id = htons(getpid() & 0xFFFF);
    
    // Numéro de séquence (incrémenté à chaque envoi)
    icmph->un.echo.sequence = htons(client->seq);
    
    // Timestamp dans le payload pour calculer le RTT
    struct timeval tv;
    gettimeofday(&tv, NULL);
    memcpy(buff + 8, &tv, sizeof(tv));
    
    // Remplissage du reste du payload avec des zéros
    for(int i = 8 + sizeof(tv); i < 8 + 56; ++i) {
        buff[i] = 0;
    }
    
    // Calcul du checksum final
    icmph->checksum = icmp_checksum(buff, 8 + PAYLOAD_SIZE);
    
    return PAYLOAD_SIZE + 8;  // Taille totale : 64 octets
}
```

---

### Calcul du checksum ICMP

Le checksum ICMP est défini dans la **RFC 792** comme le **complément à un** de la somme de tous les mots de 16 bits du paquet.

#### Algorithme (extrait de `builder.c`)

```c
int icmp_checksum(unsigned char* buff, int len) {
    const uint16_t* data = (uint16_t*)buff;
    int sum = 0;
    
    // Somme de tous les mots de 16 bits
    while (len > 1) {
        sum += ntohs(*data++);
        len -= 2;
    }
    
    // Si un octet reste (longueur impaire)
    if (len == 1) {
        uint16_t last = 0;
        *(uint8_t*)&last = *(const uint8_t*)data;
        sum += last;
    }
    
    // Fold des bits qui dépassent 16 bits
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    // Complément à un (inversion de tous les bits)
    return htons(~sum);
}
```

**Étapes** :
1. Additionner tous les mots de 16 bits
2. Gérer l'octet restant si la longueur est impaire
3. "Folder" les bits au-delà de 16 bits dans la somme
4. Inverser tous les bits (complément à un)
5. Convertir en network byte order

---

### Vérification des réponses

Le programme vérifie plusieurs aspects des réponses ICMP :

#### Extraction de l'en-tête IP

```c
struct iphdr* ip = (struct iphdr*)recv_buff;
int ip_header_len = ip->ihl * 4;  // ihl = longueur en mots de 32 bits
struct icmphdr* icmp = (struct icmphdr*)(recv_buff + ip_header_len);
```

#### Vérifications effectuées (`verifier.c`)

1. **Identifiant du processus** : Le paquet est-il pour nous ?
   ```c
   if (ntohs(icmp->un.echo.id) != (getpid() & 0xFFFF))
       return ERROR;
   ```

2. **Type de réponse** : Echo Reply (0) attendu
   ```c
   if (icmp->type != ICMP_ECHOREPLY)
       handle_error_icmp(icmp, ip, client);
   ```

3. **Checksum** : Vérification de l'intégrité
   ```c
   uint16_t recv_checksum = icmp_checksum(icmp_buf, 8 + PAYLOAD_SIZE);
   if (recv_checksum != original_checksum)
       // Paquet corrompu
   ```

4. **Détection des doublons** : Le paquet a-t-il déjà été reçu ?
   ```c
   if (client->packet[recv_seq % MAX_PING_SAVES].status == true)
       fprintf(stderr, "Duplicate reply for icmp_seq %d\n", recv_seq);
   ```

5. **Détection des paquets tardifs** : Timeout déjà affiché ?
   ```c
   if (client->packet[recv_seq % MAX_PING_SAVES].status == -1)
       fprintf(stderr, "Late reply for icmp_seq %d\n", recv_seq);
   ```

---

### Calcul du RTT (Round-Trip Time)

Le temps aller-retour est calculé en embarquant un timestamp dans le payload :

```c
// Lors de l'envoi (builder.c)
struct timeval tv;
gettimeofday(&tv, NULL);
memcpy(buff + 8, &tv, sizeof(tv));  // Timestamp dans le payload

// Lors de la réception (verifier.c)
struct timeval* send_time = (struct timeval*)(icmp_buf + 8);
struct timeval recv_time;
gettimeofday(&recv_time, NULL);

float rtt = (recv_time.tv_sec - send_time->tv_sec) * 1000.0 +
            (recv_time.tv_usec - send_time->tv_usec) / 1000.0;
```

Le RTT est exprimé en **millisecondes** (ms).

---

### Calcul des statistiques (stddev)

Le **stddev** (standard deviation / écart-type) mesure la dispersion des temps de réponse autour de la moyenne, indiquant la **stabilité** et la **variabilité** de la connexion.

#### Algorithme de Welford (implémentation en ligne)

```c
void update_client_time_stats(t_time_stats* time_stats, double new_rtt, int count) {
    // Mise à jour min/max
    if (time_stats->min == -1 || new_rtt < time_stats->min)
        time_stats->min = new_rtt;
    if (time_stats->max == -1 || new_rtt > time_stats->max)
        time_stats->max = new_rtt;
    
    // Calcul de la moyenne et de la variance (algorithme de Welford)
    time_stats->total += new_rtt;
    double tmp_delta = new_rtt - time_stats->average;
    time_stats->average += tmp_delta / count;
    time_stats->delta += tmp_delta * (new_rtt - time_stats->average);
}
```

Le **stddev** final est calculé avec la formule de l'écart-type :
```c
// Dans exit.c
int msg_transmitted = client->counter.received + client->counter.lost + client->counter.error;
double stddev = (msg_transmitted > 0) 
    ? sqrt(client->time_stats.delta / msg_transmitted) 
    : 0.0;
```

**Formule mathématique** : $\sigma = \sqrt{\frac{\sum (x_i - \bar{x})^2}{n}}$

Où :
- $\sigma$ = écart-type (stddev)
- $x_i$ = chaque RTT mesuré
- $\bar{x}$ = moyenne des RTT
- $n$ = nombre total de messages (transmis)

---

## 🐛 Gestion des erreurs

### Messages ICMP d'erreur

Le programme détecte et affiche les messages ICMP d'erreur courants :

| Type | Code | Description |
|------|------|-------------|
| 3 | 0 | Destination Network Unreachable |
| 3 | 1 | Destination Host Unreachable |
| 3 | 2 | Destination Protocol Unreachable |
| 3 | 3 | Destination Port Unreachable |
| 11 | 0 | Time Exceeded (TTL = 0) |

Ces erreurs sont gérées dans `error.c` via la fonction `handle_error_icmp()`.

### Mode verbeux

Le mode `-v` affiche des informations supplémentaires :
- Paquets dupliqués
- Paquets tardifs (après timeout)
- Erreurs de checksum
- ID du processus
- Messages ICMP détaillés

---

## 🐳 Environnement de test Docker

Pour tester le programme dans des conditions réseau dégradées (pertes de paquets, corruption, duplication), un environnement Docker est fourni avec simulation d'erreurs réseau via **NetEm** (Network Emulator).

### Architecture Docker

L'environnement comprend 3 nœuds interconnectés :

```
    node1 ──── net1 ──── node2 ──── net2 ──── node3
```

- **node1** : Nœud émetteur (où `ft_ping` est exécuté)
- **node2** : Nœud intermédiaire/cible (peut router vers node3)
- **node3** : Nœud isolé (accessible via node2)

### Prérequis

- Docker et Docker Compose installés
- Le binaire `ft_ping` compilé à la racine du projet

### Démarrage de l'environnement

```bash
# Compiler le projet
make

# Lancer les conteneurs Docker
docker-compose up --build
```

Les conteneurs restent actifs en arrière-plan (`sleep infinity`).

### Simulation d'erreurs réseau

Le script `apply_netem.sh` utilise **tc** (Traffic Control) et **netem** pour simuler des conditions réseau réalistes.

#### Syntaxe

```bash
docker-compose exec node1 apply_netem.sh <loss%> <corrupt%> <duplicate%>
```

#### Paramètres

| Paramètre | Description | Valeur |
|-----------|-------------|---------|
| `loss%` | Pourcentage de paquets perdus | 0-100 |
| `corrupt%` | Pourcentage de paquets corrompus | 0-100 |
| `duplicate%` | Pourcentage de paquets dupliqués | 0-100 |

#### Exemples de configuration

**Perte de paquets (10%)**
```bash
docker-compose exec node1 apply_netem.sh 10 0 0
```

**Corruption de paquets (50%)**
```bash
docker-compose exec node1 apply_netem.sh 0 50 0
```

**Duplication de paquets (40%)**
```bash
docker-compose exec node1 apply_netem.sh 0 0 40
```

**Conditions réseau dégradées (combinaison)**
```bash
docker-compose exec node1 apply_netem.sh 10 50 40
# 10% perte + 50% corruption + 40% duplication
```

**Réseau parfait (reset)**
```bash
docker-compose exec node1 apply_netem.sh 0 0 0
```

### Exécution de ft_ping dans Docker

Une fois les erreurs réseau configurées, testez `ft_ping` depuis node1 vers node2 :

```bash
docker-compose exec node1 ft_ping [options] node2
```

#### Exemples de tests

**Test basique avec pertes**
```bash
# Configurer 20% de perte
docker-compose exec node1 apply_netem.sh 20 0 0

# Ping node2 avec mode verbeux
docker-compose exec node1 ft_ping -v -c 10 node2
```

**Test avec corruption de checksum**
```bash
# Configurer 30% de corruption
docker-compose exec node1 apply_netem.sh 0 30 0

# Observer les erreurs de checksum
docker-compose exec node1 ft_ping -v -c 20 node2
```

**Test de duplication**
```bash
# Configurer 50% de duplication
docker-compose exec node1 apply_netem.sh 0 0 50

# Observer les "Duplicate reply"
docker-compose exec node1 ft_ping -v -c 15 node2
```

**Test complet avec statistiques**
```bash
# Conditions réseau réalistes
docker-compose exec node1 apply_netem.sh 15 25 30

# Test prolongé pour statistiques
docker-compose exec node1 ft_ping -v -c 50 -i 0.5 node2
```

### Nettoyage

```bash
# Arrêter les conteneurs
docker-compose down

# Supprimer les volumes et images
docker-compose down -v --rmi all
```

### Fichiers Docker

#### `Dockerfile`
Construit une image Debian avec :
- `iproute2` : Pour **tc** et **netem**
- `inetutils-ping` : Ping standard pour comparaison
- `tcpdump` : Capture de paquets pour débogage
- Le binaire `ft_ping` compilé
- Le script `apply_netem.sh`

#### `docker-compose.yml`
Définit 3 services avec :
- **Capabilities** : `NET_ADMIN` et `NET_RAW` (requis pour sockets RAW et tc)
- **Réseaux** : 2 réseaux bridge (`net1`, `net2`)

#### `apply_netem.sh`
Script shell configurant les règles netem sur l'interface `eth0`.

### Cas d'usage

✅ **Tests unitaires** : Vérifier la gestion des timeouts  
✅ **Tests de robustesse** : Corruption, duplication, pertes  
✅ **Tests de statistiques** : Calcul correct de stddev avec pertes  
✅ **Validation du mode verbeux** : Détection des doublons et erreurs  
✅ **Comparaison** : Comparer avec `ping` standard sur les mêmes conditions

---

