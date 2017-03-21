/*
 *  Copyright (c) 2009  Roy Keene
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include "tunnelit-common.h"
#include "tunnelit-helper.h"
#include "ethertun.h"
#include "net.h"

typedef enum {
	TUNNELIT_OPMODE_CONNECT = 0,
	TUNNELIT_OPMODE_LISTEN = 1,
	TUNNELIT_OPMODE_TCP = 0,
	TUNNELIT_OPMODE_UDP = 2
} opmode_t;

typedef ssize_t (*recvfunc_t)(int, void *, size_t, int);
typedef ssize_t (*sendfunc_t)(int, const void *, size_t, int);

static ssize_t recv_exact(recvfunc_t func, int fd, void *buf_p, size_t len) {
	ssize_t recv_ret;
	ssize_t retval = 0;
	char *buf = buf_p;

	do {
		recv_ret = func(fd, buf, len, 0);
		if (recv_ret <= 0) {
			return(recv_ret);
		}

		retval += recv_ret;
		buf += recv_ret;
		len -= recv_ret;
	} while(len);

	return(retval);
}

static ssize_t send_exact(sendfunc_t func, int fd, const void *buf, size_t len) {
	return(recv_exact((recvfunc_t) func, fd, (void *) buf, len));
}

static int write_message_to_socks(const char *buf, ssize_t buflen, const int socks[TUNNELIT_MAX_CHILDREN], int highestchildidx, int excludesock, fd_set *badfds) {
	ssize_t send_ret;
	int retval = 0;
	int sock;
	int idx;

	for (idx = 0; idx < highestchildidx; idx++) {
		sock = socks[idx];

		if (sock < 0 || sock == excludesock) {
			continue;
		}

		send_ret = send_exact((sendfunc_t) send, sock, buf, buflen);
		if (send_ret <= 0) {
			if (send_ret != buflen) {
				DEBUG_LOG("debug", "Got EOF from Socket (%i), closing it.", sock);
				FD_SET(sock, badfds);
				retval++;
				continue;
			}
		}
	}

	return(retval);
}

static fd_set *copy_sock_to_tun(int srcsock, int tun, const int socks[TUNNELIT_MAX_CHILDREN], int highestchildidx) {
	static fd_set retval;
	static char buf[16384];
	ssize_t recv_ret, send_ret;

	FD_ZERO(&retval);

	recv_ret = recv(srcsock, buf, sizeof(buf), 0);
	if (recv_ret < 0) {
		DEBUG_LOG("debug", "Got EOF from Socket (%i), closing it.", srcsock);
		FD_SET(srcsock, &retval);
		return(&retval);
	}

	if (recv_ret == 0) {
		DEBUG_LOG("debug", "Received keep-alive packet from Socket (%i).", srcsock);
		return(&retval);
	}

	DEBUG_LOG("debug", "Writing packet to TUN device (%i), len = %i", tun, (int) recv_ret);
	send_ret = send_exact((sendfunc_t) write, tun, buf, recv_ret);
	if (send_ret <= 0) {
		DEBUG_LOG("debug", "Got EOF from TUN device (%i), odd.", tun);
	}

	write_message_to_socks(buf, recv_ret, socks, highestchildidx, srcsock, &retval);

	return(&retval);
}

static fd_set *copy_tun_to_socks(int tun, const int socks[TUNNELIT_MAX_CHILDREN], int highestchildidx) {
	static fd_set retval;
	static char buf[16384];
	ssize_t read_ret;

	FD_ZERO(&retval);

	read_ret = read(tun, buf, sizeof(buf));
	if (read_ret <= 0) {
		DEBUG_LOG("debug", "Got EOF from TUN (%i) !?", tun);
		return(&retval);
	}

	write_message_to_socks(buf, read_ret, socks, highestchildidx, -1, &retval);

	return(&retval);
}

static int net_connect_tcp_persistent(const char *host, int port) {
	int newfd;

	newfd = -1;
	while (newfd < 0) {
		newfd = net_connect_tcp(host, port);
		if (newfd >= 0) {
			break;
		}

		PERROR("net_connect_tcp");
		sleep(5);
	}

	return(newfd);
}

static void print_help(FILE *chan) {
	fprintf(chan, "Usage: tunnelit-p2p [-VFhluo] [-p port] [-H host] [-d devname] [-f tunpath]\n");
	fprintf(chan, "                    [-t keepalive]\n");
	fprintf(chan, "   -V              Print version and exit\n");
	fprintf(chan, "   -F              Run in foreground (default is to background)\n");
	fprintf(chan, "   -h              Print this help and exit\n");
	fprintf(chan, "   -l              Listen (default is connect)\n");
	fprintf(chan, "   -u              UDP mode (implies \"-o\")\n");
	fprintf(chan, "   -o              Only allow one client to connect at a time.\n");
	fprintf(chan, "   -p <port>       Port to connect to or listen on\n");
	fprintf(chan, "   -H <host>       Connect to host, required if \"-l\" not specified\n");
	fprintf(chan, "   -d <devname>    TUN/TAP device name (i.e., \"tap0\")\n");
	fprintf(chan, "   -f <tunpath>    Path to kernel TUN interface (i.e., \"/dev/net/tun\")\n");
	fprintf(chan, "   -t <keepalive>  Specify how frequently to send keepalive packets, in\n");
	fprintf(chan, "                   seconds (default: 900)\n");
	return;
}

static void daemonize(void) {
	pid_t child;

	chdir("/");

	child = fork();
	if (child != 0) {
		/* Parent */
		waitpid(child, NULL, 0);
		exit(0);
	}

	/* Child */
	child = fork();
	if (child != 0) {
		/* Child */
		exit(0);
	}

	/* Grandchild */
	return;
}

int main(int argc, char **argv) {
	unsigned int pkt_count = 0;
	struct sockaddr_in remoteaddr;
	struct timeval select_timeout, select_timeout_tmp;
	socklen_t remoteaddrlen;
	opmode_t opmode = TUNNELIT_OPMODE_CONNECT | TUNNELIT_OPMODE_TCP;
	fd_set readfds, errfds, *copyfailfds;
	time_t last_connect_time, curr_time;
	char *host = NULL, *devname = NULL, *tunpath = NULL;
	char ch;
	int tunfd, masterfd, newfd, childfds[TUNNELIT_MAX_CHILDREN], maxfd;
	int port = 656;
	int opt_onlyoneclient = 0, opt_daemonize = 1;
	int found_place_for_fd;
	int select_ret;
	int idx, idxinner, highest_active_childidx;

	select_timeout.tv_sec = 900;
	select_timeout.tv_usec = 0;

	while ((ch = getopt(argc, argv, "VhluoFp:H:d:f:t:")) != -1) {
		switch (ch) {
			case 'V':
				printf("TunnelIt Point-to-Point version %s-%s\n", PACKAGE_VERSION,
#ifdef DEBUG
				       "debug"
#else
				       "rel"
#endif
				);
				return(0);
				break;
			case 'h':
				print_help(stdout);
				return(0);
				break;
			case 'l':
				opmode |= TUNNELIT_OPMODE_LISTEN;
				break;	
			case 'u':
				opmode |= TUNNELIT_OPMODE_UDP;
				opt_onlyoneclient = 1;
				break;
			case 'o':
				opt_onlyoneclient = 1;
				break;
			case 'F':
				opt_daemonize = 0;
				break;
			case 'p':
				port = atoi(optarg);
				break;
			case 'H':
				host = optarg;
				break;
			case 'd':
				devname = strdup(optarg);
				break;
			case 'f':
				tunpath = optarg;
				break;
			case 't':
				select_timeout.tv_sec = atoi(optarg);
				break;
			case '?':
			case ':':
				print_help(stderr);
				return(1);
				break;
		}
	}

	if ((opmode & TUNNELIT_OPMODE_LISTEN) == TUNNELIT_OPMODE_CONNECT) {
		if (!host) {
			ERROR_LOG("Must specify hostname (-H)");
			return(1);
		}
	}

	tunfd = tun_alloc(tunpath, devname);
	if (tunfd < 0) {
		PERROR("tun_alloc");
		ERROR_LOG("Unable to create tun/tap device");
		return(1);
	}

	if (opt_daemonize) {
		daemonize();
	}

	for (idx = 0; idx < (sizeof(childfds) / sizeof(childfds[0])); idx++) {
		childfds[idx] = -1;
	}

	masterfd = -1;
	switch ((int) opmode) {
		case TUNNELIT_OPMODE_CONNECT | TUNNELIT_OPMODE_TCP:
			newfd = net_connect_tcp_persistent(host, port);

			childfds[0] = newfd;

			break;
		case TUNNELIT_OPMODE_CONNECT | TUNNELIT_OPMODE_UDP:
			newfd = net_connect_udp(host, port);

			childfds[0] = newfd;

			break;
		case TUNNELIT_OPMODE_LISTEN | TUNNELIT_OPMODE_UDP:
			newfd = net_listen_udp(port);

			childfds[0] = newfd;

			break;
		case TUNNELIT_OPMODE_LISTEN | TUNNELIT_OPMODE_TCP:
			masterfd = -1;
			while (masterfd < 0) {
				masterfd = net_listen_tcp(port);
				if (masterfd >= 0) {
					break;
				}

				PERROR("net_listen_tcp");
				sleep(5);
			}

			break;
	}

	curr_time = time(NULL);
	last_connect_time = 0;
	while (1) {
		FD_ZERO(&readfds);
		FD_ZERO(&errfds);

		maxfd = 0;
		highest_active_childidx = -1;
		for (idx = 0; idx < (sizeof(childfds) / sizeof(childfds[0])); idx++) {
			if (childfds[idx] < 0) {
				continue;
			}

			highest_active_childidx = idx;

			FD_SET(childfds[idx], &readfds);
			FD_SET(childfds[idx], &errfds);
			if (childfds[idx] > maxfd) {
				maxfd = childfds[idx];
			}
		}
		highest_active_childidx++;

		if (tunfd > maxfd) {
			maxfd = tunfd;
		}
		FD_SET(tunfd, &readfds);
		FD_SET(tunfd, &errfds);

		if (masterfd >= 0) {
			if (masterfd > maxfd) {
				maxfd = masterfd;
			}
			FD_SET(masterfd, &readfds);
			FD_SET(masterfd, &errfds);
		}

		if (select_timeout.tv_sec == 0 && select_timeout.tv_usec == 0) {
			select_ret = select(maxfd + 1, &readfds, NULL, &errfds, NULL);
		} else {
			select_timeout_tmp.tv_sec = select_timeout.tv_sec;
			select_timeout_tmp.tv_usec = select_timeout.tv_usec;
			select_ret = select(maxfd + 1, &readfds, NULL, &errfds, &select_timeout_tmp);
		}
		if (select_ret < 0) {
			sleep(5);
			continue;
		}

		if (select_ret == 0) {
			/* Timeout, send a blank message to clients */
			select_ret = write_message_to_socks("", 0, childfds, highest_active_childidx, -1, &errfds);
		}

		pkt_count++;
		if ((pkt_count % 60) == 0) {
			curr_time = time(NULL);
		}

		if (select_ret) {
			if (FD_ISSET(tunfd, &errfds)) {
				select_ret--;
				ERROR_LOG("TUN device (%i) had an error, aborting.", tunfd);
				return(1);
			}

			if (FD_ISSET(tunfd, &readfds)) {
				select_ret--;
				DEBUG_LOG("debug", "Copying from TUN device (%i) to Sockets...", tunfd);
				copyfailfds = copy_tun_to_socks(tunfd, childfds, highest_active_childidx);
				for (idx = 0; idx < highest_active_childidx; idx++) {
					if (childfds[idx] < 0) {
						continue;
					}

					if (FD_ISSET(childfds[idx], copyfailfds)) {
						if (idx == 0 && (opmode & TUNNELIT_OPMODE_UDP) == TUNNELIT_OPMODE_UDP) {
							if (opmode == (TUNNELIT_OPMODE_UDP | TUNNELIT_OPMODE_LISTEN)) {
								close(childfds[idx]);
								childfds[idx] = net_listen_udp(port);
								last_connect_time = 0;
							}
						} else {
							DEBUG_LOG("debug", "Child Socket (%i) had an error, dropping it", childfds[idx]);
							close(childfds[idx]);
							childfds[idx] = -1;
						}

						if (idx == 0 && opmode == (TUNNELIT_OPMODE_CONNECT | TUNNELIT_OPMODE_TCP)) {
							sleep(5);
							DEBUG_LOG("debug", "Attempting to reconnect to %s:%i...", host, port);
							childfds[0] = net_connect_tcp_persistent(host, port);
						}
					}
				}
			}
		}

		if (select_ret && masterfd >= 0) {
			if (FD_ISSET(masterfd, &errfds)) {
				select_ret--;
				ERROR_LOG("Listening Socket (%i) had an error, aborting.", masterfd);
				return(1);
			}

			if (FD_ISSET(masterfd, &readfds)) {
				select_ret--;
				DEBUG_LOG("debug", "Accepting new connection (%i)", masterfd);
				newfd = -1;
				while (newfd < 0) {
					newfd = accept(masterfd, NULL, NULL);
					if (newfd >= 0) {
						break;
					}

					PERROR("accept");
					sleep(5);
				}

				if (opt_onlyoneclient) {
					for (idx = 0; idx < highest_active_childidx; idx++) {
						if (childfds[idx] >= 0) {
							close(newfd);
							newfd = -1;
						}
					}
				}

				if (newfd >= 0) {
					found_place_for_fd = 0;
					for (idx = 0; idx < (sizeof(childfds) / sizeof(childfds[0])); idx++) {
						if (childfds[idx] < 0) {
							found_place_for_fd = 1;
							childfds[idx] = newfd;
							break;
						}
					}

					if (!found_place_for_fd) {
						close(newfd);
					}
				}
			}
		}

		idx = 0;
		while (select_ret) {
			for (; idx < highest_active_childidx; idx++) {
				if (childfds[idx] < 0) {
					continue;
				}

				if (FD_ISSET(childfds[idx], &errfds)) {
					select_ret--;
					if (idx == 0 && (opmode & TUNNELIT_OPMODE_UDP) == TUNNELIT_OPMODE_UDP) {
						if (opmode == (TUNNELIT_OPMODE_UDP | TUNNELIT_OPMODE_LISTEN)) {
							close(childfds[idx]);
							childfds[idx] = net_listen_udp(port);
							last_connect_time = 0;
						}
					} else {
						DEBUG_LOG("debug", "Child Socket (%i) had an error, dropping it", childfds[idx]);
						close(childfds[idx]);
						childfds[idx] = -1;
					}

					if (idx == 0 && opmode == (TUNNELIT_OPMODE_CONNECT | TUNNELIT_OPMODE_TCP)) {
						sleep(5);
						DEBUG_LOG("debug", "Attempting to reconnect to %s:%i...", host, port);
						childfds[0] = net_connect_tcp_persistent(host, port);
					}
					break;
				}

				if (FD_ISSET(childfds[idx], &readfds)) {
					select_ret--;
					DEBUG_LOG("debug", "Copying from Socket (%i) to TUN device (%i)", childfds[idx], tunfd);

					if ((curr_time - last_connect_time) > 300 && opmode == (TUNNELIT_OPMODE_LISTEN | TUNNELIT_OPMODE_UDP)) {
						DEBUG_LOG("debug", "Issuing connect()");
						remoteaddrlen = sizeof(remoteaddr);

						recvfrom(childfds[idx], &"", 0, MSG_PEEK, (struct sockaddr *) &remoteaddr, &remoteaddrlen);
						connect(childfds[idx], (struct sockaddr *) &remoteaddr, remoteaddrlen);

						last_connect_time = curr_time;
					}

					copyfailfds = copy_sock_to_tun(childfds[idx], tunfd, childfds, highest_active_childidx);

					for (idxinner = 0; idxinner < highest_active_childidx; idxinner++) {
						if (childfds[idxinner] < 0) {
							continue;
						}

						if (FD_ISSET(childfds[idxinner], copyfailfds)) {
							if (idxinner == 0 && (opmode & TUNNELIT_OPMODE_UDP) == TUNNELIT_OPMODE_UDP) {
								if (opmode == (TUNNELIT_OPMODE_UDP | TUNNELIT_OPMODE_LISTEN)) {
									close(childfds[idx]);
									childfds[idx] = net_listen_udp(port);
									last_connect_time = 0;
								}
							} else {
								DEBUG_LOG("debug", "Child Socket (%i) had an error, dropping it", childfds[idxinner]);
								close(childfds[idxinner]);
								childfds[idxinner] = -1;
							}

							if (idxinner == 0 && opmode == (TUNNELIT_OPMODE_CONNECT | TUNNELIT_OPMODE_TCP)) {
								sleep(5);
								DEBUG_LOG("debug", "Attempting to reconnect to %s:%i...", host, port);
								childfds[0] = net_connect_tcp_persistent(host, port);
							}
						}
					}

					break;
				}
			}
		}
	}

	close(tunfd);

	return(0);
}
