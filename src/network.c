#include "globals.h"
#include "network.h"

#include "oscam.h"
#include "log.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

static int network_inet_byteorder = 0;

static int network_connect_nonb(int sockfd, const struct sockaddr *saptr, socklen_t salen, int nsec)
{
	int flags, n, error;
	socklen_t len;
	fd_set rset, wset;
	struct timeval tval;

	flags = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

	error = 0;
	log_debug("conn_nb 1 (fd=%d)", sockfd);

	if ((n = connect(sockfd, saptr, salen)) < 0) {
		if (errno == EALREADY) {
			log_debug("conn_nb in progress, errno=%d", errno);
			return -1;
		} else if (errno == EISCONN) {
			log_debug("conn_nb already connected, errno=%d", errno);
			goto done;
		}
		log_debug("conn_nb 2 (fd=%d)", sockfd);
		if (errno != EINPROGRESS) {
			log_debug("conn_nb 3 (fd=%d)", sockfd);
			return -1;
		}
	}

	/* Do whatever we want while the connect is taking place. */
	if (n == 0)
		goto done;	/* connect completed immediately */

	FD_ZERO(&rset);
	FD_SET(sockfd, &rset);
	wset = rset;
	tval.tv_sec = nsec;
	tval.tv_usec = 0;

	if ((n = select(sockfd + 1, &rset, &wset, 0, nsec ? &tval : 0)) == 0) {
		//close(sockfd);          // timeout 
		log_debug("conn_nb 4 (fd=%d)", sockfd);
		errno = ETIMEDOUT;
		return -1;
	}

	log_debug("conn_nb 5 (fd=%d)", sockfd);

	if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset)) {
		log_debug("conn_nb 6 (fd=%d)", sockfd);
		len = sizeof (error);
		if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
			log_debug("conn_nb 7 (fd=%d)", sockfd);
			return (-1);	// Solaris pending error
		}
	} else {
		log_debug("conn_nb 8 (fd=%d)", sockfd);
		return -2;
	}

      done:
	log_debug("conn_nb 9 (fd=%d)", sockfd);
	fcntl(sockfd, F_SETFL, flags);	/* restore file status flags */

	if (error) {
		log_debug("conn_nb 10 (fd=%d)", sockfd);
		//close(sockfd);            /* just in case */
		errno = error;
		return -1;
	}

	return 0;
}

int network_tcp_connection_open(char *hostname, ushort port)
{
	int flags;

	if (network_connect_nonb(client[cs_idx].udp_fd, (struct sockaddr *) &client[cs_idx].udp_sa, sizeof (client[cs_idx].udp_sa), 5) < 0) {
		log_normal("network_connect_nonb(fd=%d) failed: (errno=%d)", client[cs_idx].udp_fd, errno);
		return -1;
	}
	flags = fcntl(client[cs_idx].udp_fd, F_GETFL, 0);
	flags &= ~O_NONBLOCK;
	fcntl(client[cs_idx].udp_fd, F_SETFL, flags);

	return client[cs_idx].udp_fd;
}

void network_tcp_connection_close(int fd)
{
	log_debug("network_tcp_connection_close(): fd=%d, is_server=%d", fd, is_server);
	close(fd);
	client[cs_idx].udp_fd = 0;

	if (!is_server) {
		int i;

		pfd = 0;
		reader[ridx].tcp_connected = 0;

		for (i = 0; i < CS_MAXPENDING; i++) {
			ecmtask[i].idx = 0;
			ecmtask[i].rc = 0;
		}

		if (reader[ridx].ph.c_init()) {
			log_debug("network_tcp_connection_close() exit(1);");
			oscam_exit(1);
		}

		oscam_resolve();
		reader[ridx].ncd_msgid = 0;
		reader[ridx].last_s = reader[ridx].last_g = 0;
//		log_normal("last_s=%d, last_g=%d", reader[ridx].last_s, reader[ridx].last_g);
	}
}

in_addr_t network_inet_order(in_addr_t n)
{
	if (!network_inet_byteorder)
		network_inet_byteorder = ((inet_addr("1.2.3.4") + 1) == inet_addr("1.2.3.5")) ? 1 : 2;
	switch (network_inet_byteorder) {
		case 1:
			break;
		case 2:
			n = ((n & 0xff000000) >> 24) | ((n & 0x00ff0000) >> 8) | ((n & 0x0000ff00) << 8) | ((n & 0x000000ff) << 24);
			break;
	}

	return n;
}

char *network_inet_ntoa(in_addr_t n)
{
	struct in_addr in;

	in.s_addr = network_inet_order(n);
	return ((char *) inet_ntoa(in));
}

in_addr_t network_inet_addr(char *txt)
{
	in_addr_t n = 0;

	if (!network_inet_byteorder)
		network_inet_byteorder = ((inet_addr("1.2.3.4") + 1) == inet_addr("1.2.3.5")) ? 1 : 2;
	switch (network_inet_byteorder) {
		case 1:
			n = inet_addr(txt);
			break;
		case 2:
			n = inet_network(txt);
			break;
	}

	return n;
}
