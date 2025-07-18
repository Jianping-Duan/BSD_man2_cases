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
#include <sys/sem.h>
#include <time.h>	/* time(), localtime(), strftime() */

union semun {
	int		val;	/* value for SETVAL */
        struct semid_ds	*buf;	/* buffer for IPC_STAT & IPC_SET */
        unsigned short	*array; /* array for GETALL & SETALL */
};

static char * currtime(const char *);
static void usage_info(const char *);
static int parseops(const char *, struct sembuf *);

#define MAX_SEMOPS	512	/* Maximum operations that we permit for a
				single semop() */

int
main(int argc, char *argv[])
{
	struct sembuf sops[MAX_SEMOPS];
	int semid, i, nsops;

	if (argc < 2 || strcmp(argv[1], "--help") == 0)
		usage_info(argv[0]);

	semid = getint(argv[1]);

	for (i = 2; i < argc; i++) {
		nsops = parseops(argv[i], sops);

		printf("%5d, %s about to semop() [%s]\n", getpid(),
			currtime("%T"), argv[i]);
		if (semop(semid, sops, nsops) == -1)
			errmsg_exit1("semop (PID=%d) failed, %s\n", getpid(),
				ERR_MSG);
		printf("%5d, %s: semop() completed [%s]\n", getpid(),
			currtime("%T"), argv[i]);
	}

	exit(EXIT_SUCCESS);
}

static char *
currtime(const char *fmt)
{
#define BUFSIZE	512
	static char buf[BUFSIZE];
	time_t tim;
	size_t sz;
	struct tm *tmptr;

	tim = time(NULL);
	if ((tmptr = localtime(&tim)) == NULL) {
		fprintf(stderr, "localtime failed, %s\n", ERR_MSG);
		return NULL;
	}

	/* more formats see strftime(3) */
	sz = strftime(buf, BUFSIZE, (fmt != NULL) ? fmt : "%c", tmptr);	
	return ((sz == 0) ? NULL : buf);
}

static void
usage_info(const char *pname)
{
	fprintf(stderr, "Usage: %s semid op[,op...]\n\n", pname);
	fprintf(stderr, "'op' is either: <sem#>{+|-}<value>[u][n]\n");
	fprintf(stderr, "\t\tor: <sem#>=0[n]\n");
	fprintf(stderr, "\t\"n\" means include IPC_NOWAIT in 'op'\n");
	fprintf(stderr, "\t\"u\" means include SEM_UNDO in 'op'\n");
	fprintf(stderr, "The operations in each argument are performed in a "
		"single semop() call\n\n");
	fprintf(stderr, "e.g.:\t%s 12345 0+1,1-2un\n", pname);
	fprintf(stderr, "\t%s 12345 0=0n 1+1,2-1u 1=0\n", pname);

	exit(EXIT_FAILURE);
}

/*
 * The operation performed depends as follows on the value of sem_op:
 *
 * When sem_op is positive and the process has alter permission, the semaphore's
 * value is incremented by sem_op's value. If SEM_UNDO is specified, the
 * semaphore's adjust on exit value is decremented by sem_op's value. A positive
 * value for sem_op generally corresponds to a process releasing a resource
 * associated with the semaphore.
 *
 * The behavior when sem_op is negative and the process has alter permission,
 * depends on the current value of the semaphore:
 *
 *	If the current value of the semaphore is greater than or equal to the
 *	absolute value of sem_op, then the value is decremented by the absolute
 *	value of sem_op. If SEM_UNDO is specified, the semaphore's adjust on
 *	exit value is incremented by the absolute value of sem_op.
 *
 *	If the current value of the semaphore is less than the absolute value
 *	of sem_op, one of the following happens:
 *
 *		If IPC_NOWAIT was specified, then semop() returns immediately
 *		with a return value of EAGAIN.
 *
 *		Otherwise, the calling process is put to sleep until one of the
 *		following conditions is satisfied:
 *
 *			Some other process removes the semaphore with the
 *			IPC_RMID option of semctl(2). In this case, semop()
 *			returns immediately with a return value of EIDRM.
 *
 *			The process receives a signal that is to be caught. In
 *			this case, the process will resume execution as defined
 *			by sigaction(2).
 *
 *			The semaphore's value is greater than or equal to the
 *			absolute value of sem_op. When this condition becomes
 *			true, the semaphore's value is decremented by the
 *			absolute value of sem_op, the semaphore's adjust on exit
 *			value is incremented by the absolute value of sem_op.
 *
 *	A negative value for sem_op generally means that a process is waiting
 *	for a resource to become available.
 *
 * When sem_op is zero and the process has read permission, one of the following
 * will occur:
 *
 *	If the current value of the semaphore is equal to zero then semop() can
 *	return immediately.
 *
 *	If IPC_NOWAIT was specified, then semop() returns immediately with a
 *	return value of EAGAIN.
 *
 *	Otherwise, the calling process is put to sleep until one of the
 *	following conditions is satisfied:
 *
 *		Some other process removes the semaphore with the IPC_RMID
 *		option of semctl(2). In this case, semop() returns immediately
 *		with a return value of EIDRM.
 *
 *		The process receives a signal that is to be caught. In this
 *		case, the process will resume execution as defined by
 *		sigaction(2).
 *
 *		The semaphore's value becomes zero.
 *
 * For each semaphore a process has in use, the kernel maintains an “adjust on
 * exit” value, as alluded to earlier. When a process exits, either voluntarily
 * or involuntarily, the adjust on exit value for each semaphore is added to
 * the semaphore's value. This can be used to ensure that a resource is released
 * if a process terminates unexpectedly.
 */
static int
parseops(const char *arg, struct sembuf *sops)
{
	int nops = 0;
	const char *buf = arg, *comma;
	char *sign, *flags;

unlimloops:

	if (nops >= MAX_SEMOPS)
		errmsg_exit1("Too many operations (maxinum=%d) \"%s\"",
			MAX_SEMOPS, arg);

	if (*buf == '\0')
		errmsg_exit1("Trailing comma or empty argument: \"%s\"", arg);
	if (!isdigit(*buf))
		errmsg_exit1("Expected inital digit: \"%s\"", arg);

	sops[nops].sem_num = strtol(buf, &sign, 10);
		
	if (*sign == '\0' || strchr("+-=", *sign) == NULL)
		errmsg_exit1("Expected '+', '-', '=' in \"%s\"", arg);
	if (!isdigit(*(sign + 1)))
		errmsg_exit1("Expected digit after %c in \"%s\"", *sign, arg);

	sops[nops].sem_op = strtol(sign + 1, &flags, 10);

	switch (*sign) {
	case '-':
		sops[nops].sem_op = -(sops[nops].sem_op);
		break;
	case '=':
		if (sops[nops].sem_op != 0)
			errmsg_exit1("Expected \"=0\" in \"%s\"", arg);
		break;
	default:	/* '+' */
		break;
	}

	sops[nops].sem_flg = 0;

	while (1) {
		if (*flags == 'n')
			sops[nops].sem_flg |= IPC_NOWAIT;
		else if (*flags == 'u')
			sops[nops].sem_flg |= SEM_UNDO;
		else
			break;

		flags++;
	}

	if (*flags != ',' && *flags != '\0')
		errmsg_exit1("Bad trailing character (%c) in \"%s\"\n", *flags,
			arg);

	if ((comma = strchr(buf, ',')) == NULL)
		goto endparse;

	buf = comma + 1;

	nops++;

	if (1)
		goto unlimloops;

endparse:

	return nops + 1;
}
