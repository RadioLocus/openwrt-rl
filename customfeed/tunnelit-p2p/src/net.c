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

/*
 *	Network related functions
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "net.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <stdarg.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

/* Do things required for network access. */
static int net_init(void) {
#ifdef HAVE_WSASTARTUP
	static int net_init_called = 0;
	WSADATA wsaData;

	if (net_init_called) {
		return(0);
	}

	if (WSAStartup(MAKEWORD(2, 0), &wsaData)!=0) {
		return(-1);                           
	}

	if (wsaData.wVersion != MAKEWORD(2, 0)) {
		/* Cleanup Winsock stuff */    
		WSACleanup();              
		return(-1);  
	}

	net_init_called = 1;
#endif
	return(0);
}


/*
 *	Create a listening port on tcp port PORT
 */
int net_listen_tcp(const int port) {
	struct sockaddr_in localname;
	int sockfd;
	int netinit_ret;

	netinit_ret = net_init();
	if (netinit_ret != 0) {
		return(-1);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	localname.sin_family = AF_INET;
	localname.sin_port = htons(port);
	localname.sin_addr.s_addr = INADDR_ANY;

	if (sockfd < 0) {
		return(-1);
	}

	if (bind(sockfd, (struct sockaddr *) &localname, sizeof(localname)) < 0) {
		close(sockfd);
		return(-1);
	}

	if (listen(sockfd, 1024) < 0) {
		close(sockfd);
		return(-1);
	}

	return(sockfd);
}

int net_listen_udp(const int port) {
	struct sockaddr_in localname;
	int sockfd;
	int netinit_ret;

	netinit_ret = net_init();
	if (netinit_ret != 0) {
		return(-1);
	}

	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	localname.sin_family = AF_INET;
	localname.sin_port = htons(port);
	localname.sin_addr.s_addr = INADDR_ANY;

	if (sockfd < 0) {
		return(-1);
	}

	if (bind(sockfd, (struct sockaddr *) &localname, sizeof(localname)) < 0) {
		close(sockfd);
		return(-1);
	}

	return(sockfd);
}

int net_connect_tcp(const char *host, const int port) {
	struct hostent *hostinfo;
	struct sockaddr_in sock;
	int sockid;
	int connect_ret, netinit_ret;
#if defined(HAVE_INET_ATON)
	int inetaton_ret;
#endif

	netinit_ret = net_init();
	if (netinit_ret != 0) {
		return(-1);
	}

#if defined(HAVE_INET_ATON)
	inetaton_ret = inet_aton(host,&sock.sin_addr);

	if (inetaton_ret == 0) {
#elif defined(HAVE_INET_ADDR)
	sock.sin_addr.s_addr = inet_addr(host);

	if (sock.sin_addr.s_addr == -1) {
#else
	{
#endif
		hostinfo = gethostbyname(host);
		if (!hostinfo) {
			return(-1);
		}
		memcpy(&sock.sin_addr.s_addr, hostinfo->h_addr_list[0], hostinfo->h_length);
	}

	sock.sin_family = AF_INET;
	sock.sin_port = htons(port);
	sockid = socket(AF_INET, SOCK_STREAM, 0);
	if (sockid < 0) {
		return(-1);
	}

	connect_ret = connect(sockid, (struct sockaddr *) &sock, sizeof(sock));
	if (connect_ret < 0) {
		close(sockid);
		return(-1);
	}

	return(sockid);
}

int net_connect_udp(const char *host, const int port) {
	struct hostent *hostinfo;
	struct sockaddr_in sock;
	int sockid;
	int connect_ret, netinit_ret;
#if defined(HAVE_INET_ATON)
	int inetaton_ret;
#endif

	netinit_ret = net_init();
	if (netinit_ret != 0) {
		return(-1);
	}

#if defined(HAVE_INET_ATON)
	inetaton_ret = inet_aton(host,&sock.sin_addr);

	if (inetaton_ret == 0) {
#elif defined(HAVE_INET_ADDR)
	sock.sin_addr.s_addr = inet_addr(host);

	if (sock.sin_addr.s_addr == -1) {
#else
	{
#endif
		hostinfo = gethostbyname(host);
		if (!hostinfo) {
			return(-1);
		}
		memcpy(&sock.sin_addr.s_addr, hostinfo->h_addr_list[0], hostinfo->h_length);
	}

	sock.sin_family = AF_INET;
	sock.sin_port = htons(port);
	sockid = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockid < 0) {
		return(-1);
	}

	connect_ret = connect(sockid, (struct sockaddr *) &sock, sizeof(sock));
	if (connect_ret < 0) {
		close(sockid);
		return(-1);
	}

	return(sockid);
}

int net_close(const int sockid) {
	return(close(sockid));
}
