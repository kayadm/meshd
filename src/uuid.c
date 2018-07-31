#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <net/if.h> 
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>

struct UUID() {
	char uuid_gen[16];
	unsigned char mac_addr[6];
	struct timespec spc;
	int64_t old_tstmp = 0;
	clock_gettime(CLOCK_REALTIME, &spc);
	int64_t timeNano = spc.tv_nsec;

	uint8_t clock_id = nrand14(12);
	if (old_tmstmp > timeNano) {
		clock_id++;
	}
	get_mac_addr(mac_addr);

}

static int randto(int n) {
	int r;
	int maxrand = (RAND_MAX / n) * n;
	do r = rand(); while (r >= maxrand);
	return r % n;
}

static void shuffle(unsigned *x, size_t n) {
	while (--n) {
		size_t j = randto(n + 1);
		unsigned tmp = x[n];
		x[n] = x[j];
		x[j] = tmp;
	}
}

uint16_t nrand14(int n) {
	uint16_t v = 0;
	static unsigned pos[16] = { 0, 1,  2,  3,  4,  5,  6,  7,
		8, 9, 10, 11, 12, 13 };
	shuffle(pos, 16);
	while (n--) v |= (1U << pos[n]);
	return v;
}

void get_mac_addr(unsigned char *idx[6]) {
	struct ifreq ifr;
	struct ifconf ifc;
	char buf[1024];
	int success = 0;

	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (sock == -1) { /* handle error*/ };

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
	if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) { /* handle error */ }

	struct ifreq* it = ifc.ifc_req;
	const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

	for (; it != end; ++it) {
		strcpy(ifr.ifr_name, it->ifr_name);
		if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
			if (!(ifr.ifr_flags & IFF_LOOPBACK)) { // don't count loopback
				if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
					success = 1;
					break;
				}
			}
		}
		else { /* handle error */ }
	}
	if (success) memcpy(idx, ifr.ifr_hwaddr.sa_data, 6);
}
