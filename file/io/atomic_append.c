/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2025 Jianping Duan <static.integer@hotmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Author nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
#include "unibsd.h"
#include <sys/stat.h>
#include <fcntl.h>

int
main(int argc, char *argv[])
{
	int fd, flags;
	bool uselseek;
	long bytes, i;

	if (argc < 3) {
		fprintf(stderr, "Usage: %s <file> <bytes> [x]\n", argv[0]);
		fprintf(stderr, "\t'x' means use lseek() instend of "
			"O_APPEND.\n");
		exit(EXIT_FAILURE);
	}
	
	uselseek = argc > 3;
	flags = uselseek ? 0 : O_APPEND;
	fd = open(argv[1], O_RDWR | O_CREAT | flags, S_IRUSR | S_IWUSR);
	if (fd == -1)
		errmsg_exit1("open file %s error, %s.\n", argv[1], ERR_MSG);

	bytes = getlong(argv[2], GN_ANY_BASE);
	for (i = 0; i < bytes; i++) {
		if (uselseek) {
			if (lseek(fd, 0, SEEK_SET) == -1)
				errmsg_exit1("lseek error, %s.\n", ERR_MSG);
		}
		if (write(fd, "x", 1) != 1)
			errmsg_exit1("write error, %s.\n", ERR_MSG);
	}

	printf("%d done.\n", getpid());
	if (close(fd) == -1)
		errmsg_exit1("close file failure, %s\n", ERR_MSG);

	return 0;
}
