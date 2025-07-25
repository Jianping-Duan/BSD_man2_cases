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
 * 3. Neither the name of the University nor the names of its contributors
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
#include <sys/ipc.h>
#include <sys/stat.h>

int
main(int argc, char *argv[])
{
	key_t key;
	struct stat fst;

	if (argc != 3 || strcmp(argv[1], "--help") == 0)
		errmsg_exit1("Usage: %s filename keychar\n", argv[0]);

	printf("The size of key_t = %lu bytes\n", sizeof(key_t));

	if (stat(argv[1], &fst) == -1)
		errmsg_exit1("stat failed, %s\n", ERR_MSG);

	/*
	 * The ftok() function attempts to create a unique key suitable for use
	 * with the msgget(2), semget(2) and shmget(2) functions given the path
	 * of an existing file and a user-selectable id.
	 *
	 * The specified path must specify an existing file that is accessible
	 * to the calling process or the call will fail. Also, note that links
	 * to files will return the same key, given the same id.
	 */
	if ((key = ftok(argv[1], argv[2][0])) == -1)
		errmsg_exit1("ftok '%s' failed\n");

	printf("Key = %lx, i-node = %lx, st_dev = %lx, proj = %x\n", key,
		fst.st_ino, fst.st_dev, argv[2][0]);

	exit(EXIT_SUCCESS);
}
