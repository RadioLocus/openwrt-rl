
typedef struct{
	char *ssid;
	char *encryption;
	char *key;
}wireless;

typedef struct{
	char *proto;
        char *ipaddr;
        char *netmask;
        char *gateway;
}network;

typedef struct{
	char *url;
	int tupleversion;
	char *moninf;
}sniffex;

typedef struct{
	char *url;
}alerts;

typedef struct{
	char *id;
}sensorid;

typedef struct{
	char *curlurl;
	char *moninf;
	char *inf;
}sensorrestart;
