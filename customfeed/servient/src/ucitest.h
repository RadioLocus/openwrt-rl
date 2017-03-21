#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ucimap.h>
#include "list.h"

struct uci_servient {
	struct ucimap_section_data map;
        struct list_head list;
        
        const char *channel_url;
        const char *keepalive_url;
        const char *exit_status_url;
        const char *monit_url;
	int channel_sleep;
	int sslselfsigned;
	int rpc_timeout;
	int curl_timeout;
	int max_curl_count;
	int keepalive_sleep;
	int exit_status_sleep;
	int aliveTimeInterval;
};
struct uci_sid {
	struct ucimap_section_data map;
        struct list_head list;
        
        const char *id;
        const char *sensortoken;
        const char *servertoken;

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
servient_init_interface(struct uci_map *map, void *section, struct uci_section *s);
int
servient_add_interface(struct uci_map *map, void *section);
struct ucimap_section_data *
servient_allocate(struct uci_map *map, struct uci_sectionmap *sm, struct uci_section *s);

int
sensorid_init_interface(struct uci_map *map, void *section, struct uci_section *s);
int
sensorid_add_interface(struct uci_map *map, void *section);
struct ucimap_section_data *
sensorid_allocate(struct uci_map *map, struct uci_sectionmap *sm, struct uci_section *s);


