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
        //const char *port;
	//unsigned char *ipaddr;
	int tupleversion;
	int customflag;
};
struct uci_sid {
	struct ucimap_section_data map;
        struct list_head list;
        
        const char *id;

};

struct my_optmap {
	struct uci_optmap map;
	int test;
};
int
network_parse_ip(void *section, struct uci_optmap *om, union ucimap_data *data, const char *str);
int
network_format_ip(void *section, struct uci_optmap *om, union ucimap_data *data, char **str);
void
network_free_ip(void *section, struct uci_optmap *om, void *ptr);
int
monitoring_init_interface(struct uci_map *map, void *section, struct uci_section *s);
int
monitoring_add_interface(struct uci_map *map, void *section);
struct ucimap_section_data *
monitoring_allocate(struct uci_map *map, struct uci_sectionmap *sm, struct uci_section *s);

sensorid_init_interface(struct uci_map *map, void *section, struct uci_section *s);
int
sensorid_add_interface(struct uci_map *map, void *section);
struct ucimap_section_data *
sensorid_allocate(struct uci_map *map, struct uci_sectionmap *sm, struct uci_section *s);


