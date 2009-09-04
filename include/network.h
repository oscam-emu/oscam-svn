#ifndef __NETWORK_H__
#  define __NETWORK_H__

int network_tcp_connection_open(char *, ushort);
void network_tcp_connection_close(int);

in_addr_t network_inet_order(in_addr_t);
char *network_inet_ntoa(in_addr_t);
in_addr_t network_inet_addr(char *);

#endif // __NETWORK_H__
