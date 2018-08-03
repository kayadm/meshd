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

static int out;

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

char *get_mac_popen() {
	FILE *fp;

	static char mac[17];
	fp = popen("hcitool dev | grep -o '[[:xdigit:]:]\\{11,17\\}'", "r");
	if(fp == NULL) {
		printf("Failed to run command and get mac address.");
	}
	while(fgets(mac, 17, fp)) {
		printf("%s", mac);
	}
	char * ret = malloc(sizeof(mac));
	ret = mac;
//	fgets(mac, sizeof(mac_ad-1), fp);
	printf("%s", mac);
	pclose(fp);
	return ret;
}
 
/**void intToHex(long long n, char buf[]) {
	dprintf(out, "%s\n", "Started intToHex method.");
	char hex[100];
	int i = 0;
		while (n != 0) {
			long long temp = 0;
			temp = n % 16;
			if(n < 10) {
				hex[i] = n + '0';
				i++;
			} else {
				hex[i] = n + 55;
				i++;
			}
			n = n / 16;
	}
	dprintf(out, "%s\n", "Converted given time to hex.");
	memcpy(buf, hex, 100);
	dprintf(out, "%s\n","Copied hex string to given address");
}**/
long long get_clock() {
	struct timespec spc;
	clock_gettime(CLOCK_REALTIME, &spc);
	long long timeNano = spc.tv_sec;
	timeNano = timeNano * pow(10, 9);
	timeNano += spc.tv_nsec;
	return timeNano;
}

char* UUID() {
	char *mac_addr = get_mac_popen();
	
	char stamp_hex[100];
	unsigned long long int timeNano = get_clock();
	sprintf(stamp_hex, "%llx", timeNano);
	int64_t old_tstmp = 0;
	uint16_t nrand14(int n);
	uint16_t clock_id = nrand14(4);
	if (old_tstmp > timeNano) {
		clock_id++;
	}
	static char uuid[32];

	int idx1 = 0;
	int i = 7;
	dprintf(out, "%s\n", "All components gathered.");
	while (i < 15) { //first 8 chars
		uuid[idx1] = stamp_hex[i];
		idx1++;
		i++;
	}
	dprintf(out, "%s\n%s\n", "First loop finished.", uuid);
	i = 3;
	while(i < 7) { //4 chars
		uuid[idx1] = stamp_hex[i];
		idx1++;
		i++;
	}
	dprintf(out, "%s\n%s\n", "Second loop finished.", uuid);
	i = 0;
	uuid[12] = '1';
	idx1 = 13;
	while (i < 3) {//3 chars
		uuid[idx1] = stamp_hex[i];
		idx1++;
		i++;
	}
	dprintf(out, "%s\n%s\n", "Third loop finished.", uuid);
	i = 0;
	idx1 = 16;
	while(i < 4) {
		uuid[idx1] = (clock_id % 10) + '0';
		clock_id = clock_id / 10;
		idx1++;
		i++;
	}
	dprintf(out, "%s\n%s\n", "Fourth loop finished.", uuid);
	i = 0;
	idx1 = 20;

	FILE *fp;

	static char mac[17];
	fp = popen("hcitool dev | grep -o '[[:xdigit:]:]\\{11,17\\}'", "r");
	if(fp == NULL) {
		printf("Failed to run command and get mac address.");
	}
	/**while(fgets(mac, 17, fp)) {
		printf("%s", mac);
	}**/
	i = 0;
	while(i < 12) {
		char mac_a[] = "B8763FDFC85B";
		uuid[idx1] = mac_a[i];
		idx1++;
		i++;
	}
	pclose(fp);

	dprintf(out, "%s\n%s\n", "Last loop finished.", uuid);
	return uuid;
}

#endif