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

static char *UUID() {	
	unsigned char mac_addr[12];
	struct timespec spc;
	int64_t old_tstmp = 0;
	clock_gettime(CLOCK_REALTIME, &spc);
	int64_t timeNano = spc.tv_nsec;

	unsigned int clock_id = nrand14(5);
	if (old_tstmp > timeNano) {
		clock_id = clock_id + 1;
	}
	char arr_clock[] = convertNumberIntoArray(clock_id);
	char stamp_hex[15];
	get_mac_addr(mac_addr);
	dec_to_hexadecimal(timeNano, stamp_hex);
	
	char uuid[32];
	int idx1 = 0;
	for (int i = 7; i < 15; ++i) {
		uuid[idx1] = stamp_hex[i];
		idx1++;
	}
	for (int i = 3; i < 7; ++i) {
		uuid[idx1] = stamp_hex[i];
		idx1++;
	}
	uuid[12] = 1;
	idx1 = 13;
	for (int i = 0; i < 3; ++i) {
		uuid[idx1] = stamp_hex[i];
		idx1++;
	}
	idx1 = 16;
	for (int i = 0; i < sizeof(arr_clock); ++i) {
		uuid[idx1] = arr_clock[i];
		idx1++;
	}
	idx1 = 20;
	for (int i = 0; i < sizeof(mac_addr); ++i) {
		uuid[idx1] = mac_addr[i];
		idx1++;
	}

	return uuid;
}

char * convertNumberIntoArray(unsigned int number) {
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

static uint16_t nrand14(int n) {
	uint16_t v = 0;
	static unsigned pos[16] = { 0, 1,  2,  3,  4,  5,  6,  7,
		8, 9, 10, 11, 12, 13 };
	shuffle(pos, 16);
	while (n--) v |= (1U << pos[n]);
	return v;
}

void get_mac_addr(unsigned char *idx[12]) {
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

void dec_to_hexadecimal(uint64_t n, char *hex[]) {
	long int decimalNumber, remainder, quotient;
	int i = 1, j, temp;
	char hexadecimal[100];
	decimalNumber = n;
	quotient = decimalNumber;
	while (quotient != 0) {
		temp = quotient % 16;
		//To convert integer into character
		if (temp < 10)
			temp = temp + 48; else
			temp = temp + 55;
		hexadecimal[i++] = temp;
		quotient = quotient / 16;
	}
	memcpy(hex, hexadecimal, sizeof(hexadecimal));
	return 0;
}


#endif
