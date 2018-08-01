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

static char hex [] = { '0', '1', '2', '3', '4', '5', '6', '7',
                        '8', '9' ,'A', 'B', 'C', 'D', 'E', 'F' };
 
int uintToHexStr(uint64_t num,char* buff)
{
    int len=0,k=0;
    do
    {
        buff[len] = hex[num&&0xF];
        len++;
        num>>=4;
    }while(num!=0);
    for(;k<len/2;k++)
    {
        buff[k]^=buff[len-k-1];
        buff[len-k-1]^=buff[k];
        buff[k]^=buff[len-k-1];
    }
    buff[len]='\0';
    return len;
}

void UUID(char * buf) {	
	unsigned char mac_addr[12];
	struct timespec spc;
	int64_t old_tstmp = 0;
	clock_gettime(CLOCK_REALTIME, &spc);
	uint64_t timeNano = spc.tv_nsec;

	uint16_t nrand14(int n);
	uint16_t clock_id = nrand14(120);
	if (old_tstmp > timeNano) {
		clock_id = clock_id + 1;
	}

	char * convertNumberIntoArray(uint16_t number);
	char arr_clock[4];
	memcpy(arr_clock, convertNumberIntoArray(clock_id), 4);
	char stamp_hex[16];
	get_mac_addr(mac_addr);

	int len = uintToHexStr(timeNano, stamp_hex);
	
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

	memcpy(buf, uuid, 32);
}

#endif