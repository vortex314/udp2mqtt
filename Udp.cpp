
#include <Udp.h>
#include <Log.h>


int Udp::init()
{
	struct sockaddr_in servaddr;
	if ( (_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
		{
			perror("socket creation failed");
			return(errno);
		}

	int  optval = 1;
	setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR,(const void *)&optval, sizeof(int));
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(_myPort);
	if ( bind(_sockfd, (const struct sockaddr *)&servaddr,
	          sizeof(servaddr)) < 0 )
		{
			perror("bind failed");
			return(errno);
		}
	return 0;
}

int Udp::receive(UdpMsg& rxd)
{
	struct sockaddr_in  clientaddr;
	memset(&clientaddr, 0, sizeof(clientaddr));

	socklen_t len = sizeof(clientaddr); //len is value/resuslt
	clientaddr.sin_family = AF_INET;
	clientaddr.sin_port = htons(_myPort);
	clientaddr.sin_addr.s_addr = INADDR_ANY;

	int rc = recvfrom(_sockfd, (char *)buffer, sizeof(buffer),
	                  MSG_WAITALL, (struct sockaddr *) &clientaddr,
	                  &len);

	if ( rc >=0 )
		{
			rxd.message.clear();
			rxd.srcIp = clientaddr.sin_addr.s_addr;
			rxd._srcPort = ntohs(clientaddr.sin_port);
			char strIp[100];
			inet_ntop(AF_INET, &(rxd.srcIp), strIp, INET_ADDRSTRLEN);

			DEBUG(" received from %s:%d \n",strIp,rxd._srcPort);
			rxd.message.append(buffer,rc);
			return 0;
		}
	else
		{
			return errno;
		}
}


// Client side implementation of UDP client-server model


// Driver code
int Udp::send(UdpMsg& udpMsg)
{
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = udpMsg._dstPort;
	server.sin_addr.s_addr = udpMsg.dstIp;

	int rc = sendto(_sockfd, udpMsg.message.c_str(), udpMsg.message.length(),
	                0, (const struct sockaddr *) &server,
	                sizeof(server));
	if ( rc < 0 ) return errno;
	return 0;
}


/*
int main(int argc,char* argv[]) {
	Udp udp;
	udp.port(1883);
	UdpMsg udpMsg;
	udpMsg.dstIp("192.168.0.195");
	udpMsg.dstPort(4210);
	udpMsg.message=">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ";
	udp.init();
	udp.send(udpMsg);
	UdpMsg rcvMsg;
	udp.receive(rcvMsg);
	printf("received : '%s'\n",rcvMsg.message.c_str());
}
*/
