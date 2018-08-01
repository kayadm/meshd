#ifndef _UUID_H
#define _UUID_H

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h> 
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <math.h>


char * convertNumberIntoArray(uint16_t number) {
	unsigned int length = (int)(log10((float)number)) + 1;
	char * arr = (char *)malloc(length * sizeof(char)), *curr = arr;
	do {
		*curr++ = number % 10;
		number /= 10;
	} while (number != 0);
	return arr;
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

void get_mac_addr(unsigned char idx[12]) {
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
 
/**void intToHex(long long n, char *buf[15]) {
	char hex[15];
	int i = 0;
		while (n != 0) {
			long long temp = 0;
			temp = n % 16;
			if(n < 10) {
				hex[i] = temp + 48;
				i++;
			} else {
				hex[i] = temp + 55;
				i++;
			}
			n = n / 16;
	}
	memcpy(buf, hex, 100);
}**/
long long get_clock() {
	struct timespec spc;
	clock_gettime(CLOCK_REALTIME, &spc);
	long long timeNano = spc.tv_sec;
	timeNano = timeNano * pow(10, 9);
	timeNano += spc.tv_nsec;
	return timeNano;
}

/**void UUID(char *buf) {	
	unsigned char mac_addr[12];
	uint64_t timeNano = get_clock();
	int64_t old_tstmp = 0;
	uint16_t nrand14(int n);
	uint16_t clock_id = nrand14(120);
	if (old_tstmp > timeNano) {
		clock_id = clock_id + 1;
	}

	char * convertNumberIntoArray(uint16_t number);
	char arr_clock[4];
	memcpy(arr_clock, convertNumberIntoArray(clock_id), 4); 
	char stamp_hex[15];
	intToHex(timeNano, stamp_hex);
	get_mac_addr(mac_addr);
	char str[12];
	
	char uuid[32];

	int idx1 = 0;
	int i = 7;
	while (i < 15) { //first 8 chars
		uuid[idx1] = stamp_hex[i];
		idx1++;
		i++;
	}
	i = 3;
	while(i < 7) { //4 chars
		uuid[idx1] = stamp_hex[i];
		idx1++;
		i++;
	}
	i = 0;
	uuid[12] = 1;
	idx1 = 13;
	while (i < 3) {//3 chars
		uuid[idx1] = stamp_hex[i];
		idx1++;
		i++;
	}
	i = 0;
	idx1 = 15;
	while(i < 4) {
		uuid[idx1] = arr_clock[i];
		idx1++;
		i++;
	}
	i = 0;
	idx1 = 19;
	while(i < 12) {
		uuid[idx1] = mac_addr[i];
		idx1++;
		i++;
	}
	memcpy(buf, uuid, 32);
}**/

#endif