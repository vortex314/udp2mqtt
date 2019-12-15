// Server side implementation of UDP client-server model
#ifndef UDP_H
#define UDP_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT	 8080
#define MAXLINE 1500


class UdpMsg {
	public:
		in_addr_t _src;
		in_addr_t _dst;
		uint16_t _srcPort;
		uint16_t _dstPort;
		std::string message;
		void dstIp(const char* ip) {
			_dst = inet_addr(ip);
		}
		void dstPort(uint16_t port) {
			_dstPort=htons(port);
		}

} ;

class Udp {
		uint16_t _myPort=1883;
		int _sockfd;
		char buffer[MAXLINE];
	public:
		void dst(const char* );
		void port(uint16_t port) {
			_myPort= port;
		}
		int init();
		int receive(UdpMsg& );
		int send(UdpMsg& );
};
#endif
