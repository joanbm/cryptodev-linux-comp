/*  cryptodev_test - simple benchmark tool for cryptodev
 *
 *    Copyright (C) 2010 by Phil Sutter <phil.sutter@viprinet.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include "../cryptodev.h"

static double udifftimeval(struct timeval start, struct timeval end)
{
	return (double)(end.tv_usec - start.tv_usec) +
	       (double)(end.tv_sec - start.tv_sec) * 1000 * 1000;
}

static int must_finish = 0;

static void alarm_handler(int signo)
{
        must_finish = 1;
}

static void value2human(double bytes, double time, double* data, double* speed,char* metric)
{
        if (bytes > 1000 && bytes < 1000*1000) {
                *data = ((double)bytes)/1000;
                *speed = *data/time;
                strcpy(metric, "Kb");
                return;
        } else if (bytes >= 1000*1000 && bytes < 1000*1000*1000) {
                *data = ((double)bytes)/(1000*1000);
                *speed = *data/time;
                strcpy(metric, "Mb");
                return;
        } else if (bytes >= 1000*1000*1000) {
                *data = ((double)bytes)/(1000*1000*1000);
                *speed = *data/time;
                strcpy(metric, "Gb");
                return;
        } else {
                *data = (double)bytes;
                *speed = *data/time;
                strcpy(metric, "bytes");
                return;
        }
}


int encrypt_data(struct session_op *sess, int fdc, int chunksize, int flags)
{
	struct crypt_op cop;
	char *buffer, iv[32];
	static int val = 23;
	struct timeval start, end;
	double total = 0;
	double secs, ddata, dspeed;
	char metric[16];

	buffer = malloc(chunksize);
	memset(iv, 0x23, 32);

	printf("\tEncrypting in chunks of %d bytes: ", chunksize);
	fflush(stdout);

	memset(buffer, val++, chunksize);

	must_finish = 0;
	alarm(5);

	gettimeofday(&start, NULL);
	do {
		memset(&cop, 0, sizeof(cop));
		cop.ses = sess->ses;
		cop.len = chunksize;
		cop.iv = (unsigned char *)iv;
		cop.op = COP_ENCRYPT;
		cop.flags = flags;
		cop.src = cop.dst = (unsigned char *)buffer;

		if (ioctl(fdc, CIOCCRYPT, &cop)) {
			perror("ioctl(CIOCCRYPT)");
			return 1;
		}
		total+=chunksize;
	} while(must_finish==0);
	gettimeofday(&end, NULL);

	secs = udifftimeval(start, end)/ 1000000.0;
	
	value2human(total, secs, &ddata, &dspeed, metric);
	printf ("done. %.2f %s in %.2f secs: ", ddata, metric, secs);
	printf ("%.2f %s/sec\n", dspeed, metric);

	return 0;
}

int main(void)
{
	int fd, i, fdc = -1;
	struct session_op sess;
	char keybuf[32];

	signal(SIGALRM, alarm_handler);

	if ((fd = open("/dev/crypto", O_RDWR, 0)) < 0) {
		perror("open()");
		return 1;
	}
	if (ioctl(fd, CRIOGET, &fdc)) {
		perror("ioctl(CRIOGET)");
		return 1;
	}
	
	fprintf(stderr, "Testing NULL cipher: \n");
	memset(&sess, 0, sizeof(sess));
	sess.cipher = CRYPTO_NULL;
	sess.keylen = 0;
	sess.key = (unsigned char *)keybuf;
	if (ioctl(fdc, CIOCGSESSION, &sess)) {
		perror("ioctl(CIOCGSESSION)");
		return 1;
	}

	for (i = 256; i <= (64 * 1024); i *= 2) {
		if (encrypt_data(&sess, fdc, i, 0))
			break;
	}

	fprintf(stderr, "\nTesting AES-128-CBC cipher: \n");
	memset(&sess, 0, sizeof(sess));
	sess.cipher = CRYPTO_AES_CBC;
	sess.keylen = 16;
	memset(keybuf, 0x42, 16);
	sess.key = (unsigned char *)keybuf;
	if (ioctl(fdc, CIOCGSESSION, &sess)) {
		perror("ioctl(CIOCGSESSION)");
		return 1;
	}

	for (i = 256; i <= (64 * 1024); i *= 2) {
		if (encrypt_data(&sess, fdc, i, 0))
			break;
	}

	close(fdc);
	close(fd);
	return 0;
}
