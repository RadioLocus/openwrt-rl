#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ucimap.h>
#include "list.h"

struct uci_rl {
	struct ucimap_section_data map;
        struct list_head list;
        
        const char *id;
        const char *url;
        const char *moniturl;
        const char *moninf;
	int tupleversion;
	int curlingthreads;
	int maxtuplesinbatch;
	int seqinterval;
	int timenormalizer;
	int totptimeinterval;
	int tupleinterval;
	int badpacketslimit;
	int badpacketstimerlimit;
	int dbmsignallimit;

};

struct uci_sid {
	struct ucimap_section_data map;
        struct list_head list;
        
        const char *id;
        const char *sensortoken;

};

struct my_optmap {
	struct uci_optmap map;
	int test;
};

int network_parse_ip(void *section, struct uci_optmap *om, union ucimap_data *data, const char *str);
int network_format_ip(void *section, struct uci_optmap *om, union ucimap_data *data, char **str);
void network_free_ip(void *section, struct uci_optmap *om, void *ptr);
int blu_radiolocus_init_interface(struct uci_map *map, void *section, struct uci_section *s);
int blu_radiolocus_add_interface(struct uci_map *map, void *section);
struct ucimap_section_data *blu_radiolocus_allocate(struct uci_map *map, struct uci_sectionmap *sm, struct uci_section *s);
int sensorid_init_interface(struct uci_map *map, void *section, struct uci_section *s);
int sensorid_add_interface(struct uci_map *map, void *section);
struct ucimap_section_data *sensorid_allocate(struct uci_map *map, struct uci_sectionmap *sm, struct uci_section *s);
