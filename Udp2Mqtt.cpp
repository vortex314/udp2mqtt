
#include "Udp2Mqtt.h"

// MQTT_C_CLIENT_TRACE to ON or a filename to trace

const char* signalString[] = {"PIPE_ERROR",
                              "SELECT_ERROR",
                              "UDP_DISCONNECT",
                              "UDP_RXD",
                              "MQTT_CONNECT_SUCCESS",
                              "MQTT_CONNECT_FAIL",
                              "MQTT_SUBSCRIBE_SUCCESS",
                              "MQTT_SUBSCRIBE_FAIL",
                              "MQTT_PUBLISH_SUCCESS",
                              "MQTT_PUBLISH_FAIL",
                              "MQTT_DISCONNECTED",
                              "MQTT_MESSAGE_RECEIVED",
                              "MQTT_ERROR",
                              "TIMEOUT"
                             };

Udp2Mqtt::Udp2Mqtt(Udp& udp, in_addr_t ip, uint16_t port)
	: _jsonDocument()
	, _udp(udp) {
	_mqttConnectionState = MS_DISCONNECTED;
	_ip = ip;
	_port = port;
	char hostName[100];
	inet_ntop(AF_INET, &_ip, hostName, sizeof(hostName));
	_ipAsString = hostName;
}

Udp2Mqtt::~Udp2Mqtt() {
}

void Udp2Mqtt::setConfig(Config config) {
	_config = config;
}

void Udp2Mqtt::setLogFd(FILE* logFd) {
	_logFd = logFd;
}

void Udp2Mqtt::init() {
	_startTime = Sys::millis();

	_config.setNameSpace("udp");
	std::string protocol;
	_config.get("protocol", protocol, "jsonArray");
	_protocol = JSON_ARRAY;
	if(protocol == "jsonObject") _protocol = JSON_OBJECT;
	if(protocol == "jsonArray") _protocol = JSON_ARRAY;
	if(protocol == "protobuf") _protocol = PROTOBUF;

	std::string crc;
	_config.get("protocol", crc, "off");
	_crc = CRC_OFF;
	if(crc == "on") _crc = CRC_ON;
	if(crc == "off") _crc = CRC_OFF;

	_config.setNameSpace("mqtt");
	_config.get("port", _mqttPort, 1883);
	_config.get("host", _mqttHost, "test.mosquitto.org");
	_config.get("keepAliveInterval", _mqttKeepAliveInterval, 5);
	_config.get("willMessage", _mqttWillMessage, "false");
	_mqttWillQos = 0;
	_mqttWillRetained = false;
	_mqttDevice = _ipAsString;
	_mqttDevice += "@";
	_mqttDevice += Sys::hostname();

	_mqttSubscribedTo = "dst/" + _mqttDevice + "/#";
	_mqttClientId = _mqttDevice;
	INFO(" Mqtt client id : %s ", _mqttClientId.c_str());
	std::string willTopicDefault = "src/" + _mqttDevice + "/udp2mqtt/alive";
	_config.get("willTopic", _mqttWillTopic, willTopicDefault.c_str());
	INFO(" willTopic:'%s' default:'%s' device:'%s'",_mqttWillTopic.c_str(),willTopicDefault.c_str(),_mqttDevice.c_str());
	if(pipe(_signalFd) < 0) {
		INFO("Failed to create pipe: %s (%d)", strerror(errno), errno);
	}

	if(fcntl(_signalFd[0], F_SETFL, O_NONBLOCK) < 0) {
		INFO("Failed to set pipe non-blocking mode: %s (%d)", strerror(errno), errno);
	}
	std::string connection = "tcp://" + _mqttHost + ":";
	connection += std::to_string(_mqttPort);
	int rc = MQTTAsync_create(&_client, connection.c_str(), _mqttClientId.c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL);
	if ( rc != MQTTASYNC_SUCCESS ) {
		WARN(" MQTTAsync_create() failed.");
	}
}

// void Udp2Mqtt::threadFunction(void* pv) { run(); }

void Udp2Mqtt::run() {
	Timer mqttConnectTimer;
	Timer udpConnectTimer;
	Timer mqttPublishTimer;
// every interval try to reconnect if connection was lost
	mqttConnectTimer.atInterval(3000).doThis([this]() {
		if(_mqttConnectionState != MS_CONNECTING && _mqttConnectionState != MS_CONNECTED) {
			mqttConnect();
		}
	});
	// check every 10 sec if no data received in the last 20 sec , then signla disconnect and stop thread
	udpConnectTimer.atInterval(10000).doThis([this]() {
		if ( (Sys::millis()-_lastReceived)>20000) signal(UDP_DISCONNECT);
	});
	// publish own properties every sec
	mqttPublishTimer.atInterval(1000).doThis([this]() {
		std::string sUpTime = std::to_string((Sys::millis() - _startTime) / 1000);
		mqttPublish("src/" + _mqttDevice + "/udp2mqtt/alive", "true", 0, 0);
		mqttPublish("src/" + _mqttDevice + "/udp2mqtt/upTime", sUpTime, 0, 0);
		mqttPublish("src/" + _mqttDevice + "/udp2mqtt/device", _mqttDevice, 0, 0);
	});
// start with a connect attempt
	if(_mqttConnectionState != MS_CONNECTING) mqttConnect();
	// run loop
	while(true) {
		Signal s = waitSignal(1000);

		DEBUG("signal = %s", signalString[s]);
		mqttConnectTimer.check();
		mqttPublishTimer.check();
		udpConnectTimer.check();
		switch(s) {
			case TIMEOUT: {
					break;
				}
			case UDP_DISCONNECT : {
					DEBUG("UDP_DISCONNECT");
					MQTTAsync_destroy(&_client);
					return; // EXIT thread
				}
			case UDP_RXD: {
					DEBUG("UDP_RXD");
					if(_outgoing.size()) {
						auto udpMsg = _outgoing.front();
						_outgoing.pop_front();
						udpHandleMessage(udpMsg);
					}
					break;
				}
			case MQTT_CONNECT_SUCCESS: {
					INFO("MQTT_CONNECT_SUCCESS %s ", _ipAsString.c_str());
					mqttConnectionState(MS_CONNECTED);
					mqttSubscribe(_mqttSubscribedTo);
					break;
				}
			case MQTT_CONNECT_FAIL: {
					WARN("MQTT_CONNECT_FAIL %s ", _ipAsString.c_str());
					mqttConnectionState(MS_DISCONNECTED);
					break;
				}
			case MQTT_DISCONNECTED: {
					WARN("MQTT_DISCONNECTED %s ", _ipAsString.c_str());
					mqttConnectionState(MS_DISCONNECTED);
					break;
				}
			case MQTT_SUBSCRIBE_SUCCESS: {
					INFO("MQTT_SUBSCRIBE_SUCCESS %s ", _ipAsString.c_str());
					break;
				}
			case MQTT_SUBSCRIBE_FAIL: {
					WARN("MQTT_SUBSCRIBE_FAIL %s ", _ipAsString.c_str());
					mqttDisconnect();
					break;
				}
			case MQTT_ERROR: {
					WARN("MQTT_ERROR %s ", _ipAsString.c_str());
					break;
				}
			case PIPE_ERROR: {
					WARN("PIPE_ERROR %s ", _ipAsString.c_str());
					break;
				}
			case MQTT_PUBLISH_SUCCESS: {
					break;
				}
			case MQTT_MESSAGE_RECEIVED: {
					break;
				}
			default: {
					WARN("received signal [%d] %s for %s ", s, signalString[s], _ipAsString.c_str());
				}
		}
	}
	WARN(" exited run loop !!");
}

void Udp2Mqtt::signal(uint8_t m) {
	if(write(_signalFd[1], (void*)&m, 1) < 1) {
		INFO("Failed to write pipe: %s (%d)", strerror(errno), errno);
	}
	//	INFO(" signal '%c' ",m);
}

Udp2Mqtt::Signal Udp2Mqtt::waitSignal(uint32_t timeout) {
	Signal returnSignal = TIMEOUT;
	uint8_t buffer;
	fd_set rfds;
	fd_set wfds;
	fd_set efds;
	struct timeval tv;
	int retval;
	//    uint64_t delta=1000;

	// Wait up to 1000 msec.
	uint64_t delta = timeout;

	tv.tv_sec = 1;
	tv.tv_usec = (delta * 1000) % 1000000;

	// Watch serialFd and tcpFd  to see when it has input.
	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_ZERO(&efds);

	if(_signalFd[0]) {
		FD_SET(_signalFd[0], &rfds);
		FD_SET(_signalFd[0], &efds);
	}
	int maxFd = _signalFd[0] + 1;

	retval = select(maxFd, &rfds, NULL, &efds, &tv);

	if(retval < 0) {
		WARN(" select() : error : %s (%d)", strerror(errno), errno);
		returnSignal = SELECT_ERROR;
	} else if(retval > 0) { // one of the fd was set

		if(FD_ISSET(_signalFd[0], &rfds)) {
			::read(_signalFd[0], &buffer, 1); // read 1 event
			returnSignal = (Signal)buffer;
		}

		if(FD_ISSET(_signalFd[0], &efds)) {
			WARN("pipe  error : %s (%d)", strerror(errno), errno);
			returnSignal = PIPE_ERROR;
		}
	} else {

		TRACE(" timeout %llu", Sys::millis());
		returnSignal = TIMEOUT;
	}

	return (Signal)returnSignal;
}

unsigned short crc16(const unsigned char* data_p, unsigned char length) {
	unsigned char x;
	unsigned short crc = 0xFFFF;

	while(length--) {
		x = crc >> 8 ^ *data_p++;
		x ^= x >> 4;
		crc = (crc << 8) ^ ((unsigned short)(x << 12)) ^ ((unsigned short)(x << 5)) ^ ((unsigned short)x);
	}
	return crc;
}
/*
        * extract CRC and verify in syntax => ["ABCD",2,..]
 */
bool Udp2Mqtt::checkCrc(std::string& line) {
	if(line.length() < 9) return false;
	std::string crcStr = line.substr(2, 4);
	uint32_t crcFound;
	sscanf(crcStr.c_str(), "%4X", &crcFound);
	std::string line2 = line.substr(0, 2) + "0000" + line.substr(6, string::npos);
	uint32_t crcCalc = crc16((uint8_t*)line2.data(), line2.length());
	return crcCalc == crcFound;
}

void Udp2Mqtt::genCrc(std::string& line) {
	uint32_t crcCalc = crc16((uint8_t*)line.data(), line.length());
	char hexCrc[5];
	sprintf(hexCrc, "%4.4X", crcCalc);
	std::string lineCopy = "[\"";
	lineCopy += hexCrc + line.substr(6, string::npos);
	line = lineCopy.c_str();
}

/*
 * JSON protocol : [CRC,CMD,TOPIC,MESSAGE,QOS,RETAIN]
 * CMD : 0:PING,1:PUBLISH,2:PUBACK,3:SUBSCRIBE,4:SUBACK,...
 * ping : ["0000",0,"someText"]
 * publish : ["ABCD",1,"dst/topic1","message1",0,0]
 * subscribe : ["ABCD",3,"dst/myTopic"]
 *
 */

std::vector<string> split(const string& text, char sep) {
	std::vector<string> tokens;
	std::size_t start = 0, end = 0;
	while((end = text.find(sep, start)) != string::npos) {
		tokens.push_back(text.substr(start, end - start));
		start = end + 1;
	}
	tokens.push_back(text.substr(start));
	return tokens;
}

typedef enum { SUBSCRIBE = 0, PUBLISH } CMD;

void Udp2Mqtt::queue(UdpMsg& udpMsg) {
	_outgoing.push_back(udpMsg);
	_lastReceived = Sys::millis();
	if(_outgoing.size() > 2) INFO("[%X] messages messages queued : %d", this, _outgoing.size());
	signal(UDP_RXD);
}

void Udp2Mqtt::udpHandleMessage(UdpMsg& udpMsg) {
	std::string line = udpMsg.message;
	std::vector<string> token;
	_jsonDocument.clear();
	if(_protocol == JSON_ARRAY && line.length() > 2 && line[0] == '[' && line[line.length() - 1] == ']') {
		deserializeJson(_jsonDocument, line);
		if(_jsonDocument.is<JsonArray>()) {
			JsonArray array = _jsonDocument.as<JsonArray>();
			int cmd = array[0];
			if(cmd == PUBLISH) {
				std::string topic = array[1];
				std::string message = array[2];
				uint32_t qos = 0;
				if(array.size() > 3) qos = array[3];
				bool retained = false;
				if(array.size() > 4) retained = array[4];
				mqttPublish(topic, message, qos, retained);
				return;
			} else if(cmd == SUBSCRIBE) {
				std::string topic = array[1];
				mqttSubscribe(topic);
				return;
			}
		}
	} else if(_protocol == JSON_OBJECT && line.length() > 2 && line[0] == '{' && line[line.length() - 1] == '}') {
		deserializeJson(_jsonDocument, line);
		if(_jsonDocument.is<JsonObject>()) {
			JsonObject json = _jsonDocument.as<JsonObject>();
			if(json.containsKey("cmd")) {
				string cmd = json["cmd"];
				if(cmd.compare("MQTT-PUB") == 0 && json.containsKey("topic") && json.containsKey("message")) {
					int qos = 0;
					bool retained = false;
					string topic = json["topic"];
					token = split(topic, '/');
					if(token[1].compare(_mqttDevice) != 0) {
						WARN(" subscribed topic differ %s <> %s ", token[1].c_str(), _mqttDevice.c_str());
						_mqttDevice = token[1];
						_mqttSubscribedTo = "dst/" + _mqttDevice + "/#";
						mqttSubscribe(_mqttSubscribedTo);
					}
					string message = json["message"];
					/*                    Bytes msg(1024);
					                    msg.append((uint8_t*)message.c_str(),message.length());*/
					mqttPublish(topic, message, qos, retained);
					return;
				} else if(cmd.compare("MQTT-SUB") == 0 && json.containsKey("topic")) {
					string topic = json["topic"];
					mqttSubscribe(topic);
					return;
				} else {
					WARN(" invalid command from device : %s", line.c_str());
				}
			}
		}
	}
	fprintf(stdout, "%s\n", line.c_str());
	if(_logFd != NULL) {
		fprintf(_logFd, "%s\n", line.c_str());
		fflush(_logFd);
	}

	mqttPublish("src/" + _mqttDevice + "/Udp2Mqtt/log", line, 0, false);
}

void Udp2Mqtt::udpPublish(string topic, Bytes message, int qos, bool retained) {
	std::string line;

	if(_protocol == JSON_OBJECT) {
		string msg;
		msg.assign((const char*)message.data(), 0, message.length());
		_jsonDocument.clear();
		JsonObject out = _jsonDocument.to<JsonObject>();
		out["cmd"] = "MQTT-PUB";
		out["topic"] = topic;
		out["message"] = msg.c_str();
		if(qos) out["qos"] = qos;
		if(retained) out["retained"] = retained;
		serializeJson(out, line);
	} else if(_protocol == JSON_ARRAY) {
		string msg;
		msg.assign((const char*)message.data(), 0, message.length());
		DynamicJsonDocument doc(2038);
		doc.add(1);
		doc.add(topic);
		doc.add(msg);
		if(qos) doc.add(qos);
		if(retained) doc.add(1);
		if(_crc == CRC_ON) doc.add("0000");
		serializeJson(doc, line);
		if(_crc == CRC_ON) genCrc(line);
	} else {
		WARN(" invalid protocol found.");
	}
	udpSend(line);
	INFO(" TXD %s : %s ", _ipAsString.c_str(), line.c_str());
}

void Udp2Mqtt::udpSend(const string line) {
	UdpMsg udpMsg;
	udpMsg.dstPort(_port);
	udpMsg.dstIp = _ip;
	udpMsg.message = line;
	int erc = _udp.send(udpMsg);
	if(erc < 0) {
		INFO("write() failed '%s' errno : %d : %s ", _mqttDevice.c_str(), errno, strerror(errno));
	}
}

/*
 *
                                                @     @  @@@@@  @@@@@@@ @@@@@@@
                                                @@   @@ @     @    @       @
                                                @ @ @ @ @     @    @       @
                                                @  @  @ @     @    @       @
                                                @     @ @   @ @    @       @
                                                @     @ @    @     @       @
                                                @     @  @@@@ @    @       @

 *
 */
const char* mqttConnectionStates[] = {"MS_CONNECTED", "MS_DISCONNECTED", "MS_CONNECTING", "MS_DISCONNECTING"};
void Udp2Mqtt::mqttConnectionState(MqttConnectionState st) {
	INFO(" MQTT connection state %s => %s ", mqttConnectionStates[_mqttConnectionState], mqttConnectionStates[st]);
	_mqttConnectionState = st;
}

Erc Udp2Mqtt::mqttConnect() {
	int rc;
	if(_mqttConnectionState == MS_CONNECTING || _mqttConnectionState == MS_CONNECTED) return E_OK;

	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	MQTTAsync_willOptions will_opts = MQTTAsync_willOptions_initializer;


	INFO(" MQTT connecting tcp://%s:%d ... for %s ", _mqttHost.c_str(),_mqttPort, _mqttClientId.c_str());
	mqttConnectionState(MS_CONNECTING);

	MQTTAsync_setCallbacks(_client, this, onConnectionLost, onMessage, onDeliveryComplete); // TODO add ondelivery

	conn_opts.keepAliveInterval = _mqttKeepAliveInterval;
	conn_opts.cleansession = 1;
	conn_opts.onSuccess = onConnectSuccess;
	conn_opts.onFailure = onConnectFailure;
	conn_opts.context = this;
	will_opts.message = _mqttWillMessage.c_str();
	will_opts.topicName = _mqttWillTopic.c_str();
	will_opts.qos = _mqttWillQos;
	will_opts.retained = _mqttWillRetained;
	conn_opts.will = &will_opts;
	INFO("LW topic:%s message:%s qos:%d retain:%d",will_opts.topicName,will_opts.message,will_opts.qos,will_opts.retained);
	if((rc = MQTTAsync_connect(_client, &conn_opts)) != MQTTASYNC_SUCCESS) {
		WARN("Failed to start connect, return code %d", rc);
		mqttConnectionState(MS_DISCONNECTED);
		return E_NOT_FOUND;
	}
	return E_OK;
}

void Udp2Mqtt::mqttDisconnect() {
	mqttConnectionState(MS_DISCONNECTING);
	MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
	disc_opts.onSuccess = onDisconnect;
	disc_opts.context = this;
	int rc = 0;
	if((rc = MQTTAsync_disconnect(_client, &disc_opts)) != MQTTASYNC_SUCCESS) {
		WARN("Failed to disconnect, return code %d", rc);
		return;
	}
	MQTTAsync_destroy(&_client);
	mqttConnectionState(MS_DISCONNECTED);
}

void Udp2Mqtt::mqttSubscribe(string topic) {
	int qos = 0;
	if(_mqttConnectionState != MS_CONNECTED) return;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	INFO("Subscribing to topic %s for client %s using QoS%d", topic.c_str(), _ipAsString.c_str(), qos);
	opts.onSuccess = onSubscribeSuccess;
	opts.onFailure = onSubscribeFailure;
	opts.context = this;
	int rc = E_OK;

	if((rc = MQTTAsync_subscribe(_client, topic.c_str(), qos, &opts)) != MQTTASYNC_SUCCESS) {
		ERROR("Failed to start subscribe, return code %d", rc);
		signal(MQTT_SUBSCRIBE_FAIL);
	} else {
		INFO(" subscribe send : %s ", topic.c_str());
	}
}

void Udp2Mqtt::onConnectionLost(void* context, char* cause) {
	Udp2Mqtt* me = (Udp2Mqtt*)context;
	me->signal(MQTT_DISCONNECTED);
}

int Udp2Mqtt::onMessage(void* context, char* topicName, int topicLen, MQTTAsync_message* message) {
	INFO(" MQTT RXD %s ", topicName);
	Udp2Mqtt* me = (Udp2Mqtt*)context;
	Bytes msg((uint8_t*)message->payload, message->payloadlen);
	string topic(topicName, topicLen);

	me->udpPublish(topic, msg, message->qos, message->retained);

	MQTTAsync_freeMessage(&message);
	MQTTAsync_free(topicName);
	me->signal(MQTT_MESSAGE_RECEIVED);
	return 1;
}

void Udp2Mqtt::onDeliveryComplete(void* context, MQTTAsync_token response) {
	//    Udp2Mqtt* me = (Udp2Mqtt*)context;
}

void Udp2Mqtt::onDisconnect(void* context, MQTTAsync_successData* response) {
	Udp2Mqtt* me = (Udp2Mqtt*)context;
	me->signal(MQTT_DISCONNECTED);
}

void Udp2Mqtt::onConnectFailure(void* context, MQTTAsync_failureData* response) {
	Udp2Mqtt* me = (Udp2Mqtt*)context;
	WARN(" onConnectFailure : response.code : %d : %s ", response->code, response->message);
	me->signal(MQTT_CONNECT_FAIL);
	me->mqttConnectionState(MS_DISCONNECTED);
}

void Udp2Mqtt::onConnectSuccess(void* context, MQTTAsync_successData* response) {
	Udp2Mqtt* me = (Udp2Mqtt*)context;
	me->signal(MQTT_CONNECT_SUCCESS);
	me->mqttConnectionState(MS_CONNECTED);
}

void Udp2Mqtt::onSubscribeSuccess(void* context, MQTTAsync_successData* response) {
	Udp2Mqtt* me = (Udp2Mqtt*)context;
	me->signal(MQTT_SUBSCRIBE_SUCCESS);
}

void Udp2Mqtt::onSubscribeFailure(void* context, MQTTAsync_failureData* response) {
	Udp2Mqtt* me = (Udp2Mqtt*)context;
	me->signal(MQTT_SUBSCRIBE_FAIL);
}

void Udp2Mqtt::mqttPublish(string topic, string message, int qos, bool retained) {
	Bytes msg(1024);
	//	INFO(" MQTT PUB : %s = %s ", topic.c_str(), message.c_str());
	msg = message.c_str();
	mqttPublish(topic, msg, qos, retained);
}

void Udp2Mqtt::mqttPublish(string topic, Bytes message, int qos, bool retained) {
	if(!(_mqttConnectionState == MS_CONNECTED)) {
		return;
	}
	qos = 1;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	MQTTAsync_message pubmsg = MQTTAsync_message_initializer;

	//   INFO("mqttPublish %s",topic.c_str());

	int rc = E_OK;
	opts.onSuccess = onPublishSuccess;
	opts.onFailure = onPublishFailure;
	opts.context = this;

	pubmsg.payload = message.data();
	pubmsg.payloadlen = message.length();
	pubmsg.qos = qos;
	pubmsg.retained = retained;

	if((rc = MQTTAsync_sendMessage(_client, topic.c_str(), &pubmsg, &opts)) != MQTTASYNC_SUCCESS) {
		signal(MQTT_DISCONNECTED);
		ERROR("MQTTAsync_sendMessage failed.");
	}
}
void Udp2Mqtt::onPublishSuccess(void* context, MQTTAsync_successData* response) {
	Udp2Mqtt* me = (Udp2Mqtt*)context;
	me->signal(MQTT_PUBLISH_SUCCESS);
}
void Udp2Mqtt::onPublishFailure(void* context, MQTTAsync_failureData* response) {
	Udp2Mqtt* me = (Udp2Mqtt*)context;
	me->signal(MQTT_PUBLISH_FAIL);
}
