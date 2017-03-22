#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/wireless.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pcap.h>
#include <ctype.h>
#include <errno.h>
#include "radiotap-parser.h"
#include "sniffer.h"
#include "queue.h"
#include "commonFunctions.h"

/* default snap length (maximum bytes per packet to capture) */
#define SNAP_LEN 1518

/* ethernet headers are always exactly 14 bytes [1] */
#define SIZE_ETHERNET 14

/* Ethernet addresses are 6 bytes */
#define ETHER_ADDR_LEN  6

void got_pkg(u_char *, const struct pcap_pkthdr *, const u_char *);

//typedef struct _configuration Configuration;
typedef struct{
  struct Queue * queue;
  int sensor_tupleversion, tuplecounter, custom_flag, bad_packets, bad_packets_time, dbmsignal_limit, bad_packets_timer, bad_packets_count, seqinterval, timenormalizer, totptimeinterval;
  char *sensor_id;
  char *sensor_token;
} Configuration ;
