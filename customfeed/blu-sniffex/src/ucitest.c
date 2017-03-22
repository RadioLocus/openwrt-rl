/*
 * ucimap-example - sample code for the ucimap library
 * Copyright (C) 2008-2009 Felix Fietkau <nbd@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ucimap.h>
#include "ucitest.h"

int
network_parse_ip(void *section, struct uci_optmap *om, union ucimap_data *data, const char *str)
{
	unsigned char *target;
	int tmp[4];
	int i;

	if (sscanf(str, "%d.%d.%d.%d", &tmp[0], &tmp[1], &tmp[2], &tmp[3]) != 4)
		return -1;

	target = malloc(4);
	if (!target)
		return -1;

	data->ptr = target;
	for (i = 0; i < 4; i++)
		target[i] = (char) tmp[i];

	return 0;
}

int
network_format_ip(void *section, struct uci_optmap *om, union ucimap_data *data, char **str)
{
	static char buf[16];
	unsigned char *ip = (unsigned char *) data->ptr;

	if (ip) {
		sprintf(buf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
		*str = buf;
	} else {
		*str = NULL;
	}

	return 0;
}

void
network_free_ip(void *section, struct uci_optmap *om, void *ptr)
{
	free(ptr);
}

int
bluetooth_init_interface(struct uci_map *map, void *section, struct uci_section *s)
{
	struct uci_rl *rl = section;

	INIT_LIST_HEAD(&rl->list);
	return 0;
}


struct ucimap_section_data *
bluetooth_allocate(struct uci_map *map, struct uci_sectionmap *sm, struct uci_section *s)
{
	struct uci_rl *p = malloc(sizeof(struct uci_rl));
	memset(p, 0, sizeof(struct uci_rl));
	return &p->map;
}

int
sensorid_init_interface(struct uci_map *map, void *section, struct uci_section *s)
{
	struct uci_sid *sid = section;

	INIT_LIST_HEAD(&sid->list);
	return 0;
}


struct ucimap_section_data *
sensorid_allocate(struct uci_map *map, struct uci_sectionmap *sm, struct uci_section *s)
{
	struct uci_sid *p = malloc(sizeof(struct uci_sid));
	memset(p, 0, sizeof(struct uci_sid));
	return &p->map;
}


