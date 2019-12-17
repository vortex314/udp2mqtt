#include <stdio.h>
#include <Log.h>
#include <Config.h>
#include <Udp.h>
#include <Udp2Mqtt.h>
#include <thread>
#include <unordered_map>
#include <utility>

#define DEFAULT_CONFIG "/home/lieven/workspace/udp2mqtt/udp2mqtt.json"

void overrideConfig(Config& config, int argc, char** argv);

Log logger(2048);
// Config config;
#define MAX_PORT 20

std::string logFile = "";
FILE* logFd = 0;

void myLogFunction(char* s, uint32_t length)
{
	fprintf(logFd, "%s\n", s);
	fflush(logFd);
	fprintf(stdout, "%s\r\n", s);
}

void SetThreadName(std::thread* thread, const char* threadName)
{
	auto handle = thread->native_handle();
	pthread_setname_np(handle, threadName);
}

int main(int argc, char** argv)
{

	std::thread threads[MAX_PORT];
	StaticJsonDocument<10240> jsonDoc;

	Sys::init();
	INFO("build : " __DATE__ " " __TIME__);
	if(argc > 1)
		{
			INFO(" loading config file : %s ", argv[1]);
			config.loadFile(argv[1]);
		}
	else
		{
			INFO(" load default config : %s", DEFAULT_CONFIG);
			config.loadFile(DEFAULT_CONFIG);
		}
	overrideConfig(config, argc, argv);
	if(logFile.length() > 0)
		{
			INFO(" logging to file %s ", logFile.c_str());
			logFd = fopen(logFile.c_str(), "w");
			if(logFd == NULL)
				{
					WARN(" open logfile %s failed : %d %s ", logFile.c_str(), errno, strerror(errno));
				}
			else
				{
					logger.setOutput(myLogFunction);
				}
		}

	config.setNameSpace("udp");
	uint32_t udpPort = config.root()["udp"]["port"] | 1883;
	std::unordered_map<in_addr_t, Udp2Mqtt*> receivers;

	Udp udp;
	udp.port(udpPort);
	udp.init();
	while(true)
		{
			UdpMsg udpMsg;
			udp.receive(udpMsg);
			in_addr_t fromAddress = udpMsg._src;
			uint16_t fromPort = udpMsg._srcPort;
			auto got = receivers.find(fromAddress);
			if(got == receivers.end())
				{
					auto receiver = new Udp2Mqtt(udp, fromAddress, fromPort);
					receiver->setConfig(config);
					receiver->init();
					char hostName[100];
					inet_ntop(AF_INET, &fromAddress, hostName, sizeof(hostName));
					receiver->queue(udpMsg);
					receivers.insert(std::make_pair(fromAddress, receiver));
					auto thread = new std::thread([=]()
					{
						INFO(" starting thread for %s ", hostName);
						receiver->run();
						receivers.erase(fromAddress);
						delete receiver;
						INFO(" stopping thread for %s ", hostName);
					});
					SetThreadName(thread, &hostName[4]);
				}
			else
				{
					got->second->queue(udpMsg);
				}
		}

	/*
	        for(uint32_t i=0; i<ports.size(); i++) {
	                threads[i] =
	                std::string port= ports[i];
	                SetThreadName(&threads[i],port.substr(8).c_str());
	        }
	*/
}

void overrideConfig(Config& config, int argc, char** argv)
{
	int opt;

	while((opt = getopt(argc, argv, "f:m:l:v:")) != -1)
		{
			switch(opt)
				{
				case 'm':
					config.setNameSpace("mqtt");
					config.set("host", optarg);
					break;
				case 'f':
					config.loadFile(optarg);
					break;
				case 'v':
				{
					char logLevel = optarg[0];
					logger.setLogLevel(logLevel);
					break;
				}
				case 'l':
					logFile = optarg;
					break;
				default: /* '?' */
					fprintf(stderr, "Usage: %s [-v(TDIWE)] [-f configFile] [-l logFile] [-m mqttHost]\n", argv[0]);
					exit(EXIT_FAILURE);
				}
		}
}
