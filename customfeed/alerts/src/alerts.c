/**
 *  * example C code using libcurl and json-c
 *   * to post and return a payload using
 *    * http://jsonplaceholder.typicode.com
 *     *
 *      * Requirements:
 *       *
 *        * json-c - https://github.com/json-c/json-c
 *         * libcurl - http://curl.haxx.se/libcurl/c
 *          *
 *           * Build:
 *            *
 *             * cc curltest.c -lcurl -ljson-c -o curltest
 *              *
 *               * Run:
 *                *
 *                 * ./curltest
 *                  * 
 *                   */

/* standard includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

/* json-c (https://github.com/json-c/json-c) */
#include <json-c/json.h>

/* libcurl (http://curl.haxx.se/libcurl/c) */
#include <curl/curl.h>
#include "ucitest.h"
#define SKIP_PEER_VERIFICATION 0
#define SKIP_HOSTNAME_VERIFICATION 0
#define MAX_ATTEMPTS 3

/* holder for curl fetch */
struct curl_fetch_st {
    char *payload;
    size_t size;
};

/* callback for curl fetch */
size_t curl_callback (void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;                             /* calculate buffer size */
    struct curl_fetch_st *p = (struct curl_fetch_st *) userp;   /* cast pointer to fetch struct */

    /* expand buffer */
    p->payload = (char *) realloc(p->payload, p->size + realsize + 1);

    /* check buffer */
    if (p->payload == NULL) {
      /* this isn't good */
      fprintf(stderr, "ERROR: Failed to expand buffer in curl_callback");
      /* free buffer */
      free(p->payload);
      /* return */
      return -1;
    }

    /* copy contents to buffer */
    memcpy(&(p->payload[p->size]), contents, realsize);

    /* set new buffer size */
    p->size += realsize;

    /* ensure null termination */
    p->payload[p->size] = 0;

    /* return size */
    return realsize;
}

/* fetch and return url body via curl */
CURLcode curl_fetch_url(CURL *ch, const char *url, struct curl_fetch_st *fetch) {
    CURLcode rcode;                   /* curl result code */
    int i;

    /* init payload */
    fetch->payload = (char *) calloc(1, sizeof(fetch->payload));

    /* check payload */
    if (fetch->payload == NULL) {
        /* log error */
        fprintf(stderr, "ERROR: Failed to allocate payload in curl_fetch_url");
        /* return error */
        return CURLE_FAILED_INIT;
    }

    /* init size */
    fetch->size = 0;

    /* set url to fetch */
    curl_easy_setopt(ch, CURLOPT_URL, url);

    /* set calback function */
    curl_easy_setopt(ch, CURLOPT_WRITEFUNCTION, curl_callback);

    /* pass fetch struct pointer */
    curl_easy_setopt(ch, CURLOPT_WRITEDATA, (void *) fetch);

    /* set default user agent */
    curl_easy_setopt(ch, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    /* set timeout */
    curl_easy_setopt(ch, CURLOPT_TIMEOUT, 5);

    /* enable location redirects */
    curl_easy_setopt(ch, CURLOPT_FOLLOWLOCATION, 1);

    /* set maximum allowed redirects */
    curl_easy_setopt(ch, CURLOPT_MAXREDIRS, 1);

    /* fetch the url */
    for (i = 0; i < MAX_ATTEMPTS; i++) {
        printf("try %d", i);
        if ((rcode = curl_easy_perform(ch)) == CURLE_OK && fetch->size > 1){
            printf("rcode %d", rcode);
            break;
            }
        printf("rcode %d", rcode);
    }
    /* return */
    return rcode;
}
char *runMonitoringCommand(char *param)
{
  FILE *fp;
  static char output[1035];
  char cmd[1035];

  /* Open the command for reading. */
  sprintf(cmd, "/bin/monitoring %s", param);
  /*printf("cmd %s\n", cmd);*/
  fp = popen(cmd, "r");
  if (fp == NULL) {
    printf("Failed to run command\n" );
    exit(1);
  }

  /* Read the output a line at a time - output it. */
  while (fgets(output, sizeof(output)-1, fp) != NULL) {
    /*printf("%s", output);*/
  }

  /* close */
  pclose(fp);

  return output;

}

/************for configs***********************/

struct list_head rls;
struct list_head sidls;
struct uci_sectionmap monitoring;
struct uci_sectionmap sensorid;

int
monitoring_add_interface(struct uci_map *map, void *section)
{
        struct uci_rl *rl = section;

        list_add_tail(&rl->list, &rls);

        return 0;
}

int
sensorid_add_interface(struct uci_map *map, void *section)
{
        struct uci_sid *sid = section;

        list_add_tail(&sid->list, &sidls);

        return 0;
}


int
monitoring_init_interface(struct uci_map *map, void *section, struct uci_section *s)
{
        struct uci_rl *rl = section;

        INIT_LIST_HEAD(&rl->list);
        return 0;
}


struct ucimap_section_data *
monitoring_allocate(struct uci_map *map, struct uci_sectionmap *sm, struct uci_section *s)
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


struct my_optmap monitoring_options[] = {
	{
		.map = {
			UCIMAP_OPTION(struct uci_rl, url),
			.type = UCIMAP_STRING,
			.name = "url",
			.data.s.maxlen = 200,
		}
	}
};

struct my_optmap sensorid_options[] = {
	{
		.map = {
			UCIMAP_OPTION(struct uci_sid, id),
			.type = UCIMAP_STRING,
			.name = "id",
			.data.s.maxlen = 170,
		}
	}
};
struct uci_sectionmap monitoring = {
	UCIMAP_SECTION(struct uci_rl, map),
	.type = "alerts",
	.alloc = monitoring_allocate,
	.init = monitoring_init_interface,
	.add = monitoring_add_interface,
	.options = &monitoring_options[0].map,
	.n_options = ARRAY_SIZE(monitoring_options),
	.options_size = sizeof(struct my_optmap)
};

struct uci_sectionmap sensorid = {
	UCIMAP_SECTION(struct uci_sid, map),
	.type = "sensor",
	.alloc = sensorid_allocate,
	.init = sensorid_init_interface,
	.add = sensorid_add_interface,
	.options = &sensorid_options[0].map,
	.n_options = ARRAY_SIZE(sensorid_options),
	.options_size = sizeof(struct my_optmap)
};

struct uci_sectionmap *monitoring_smap[] = {
	&monitoring,
};

struct uci_map monitoring_map = {
	.sections = monitoring_smap,
	.n_sections = ARRAY_SIZE(monitoring_smap),
};


struct uci_sectionmap *sensorid_smap[] = {
	&sensorid,
};

struct uci_map sensorid_map = {
	.sections = sensorid_smap,
	.n_sections = ARRAY_SIZE(sensorid_smap),
};

int main(int argc, char *argv[]) {
    CURL *ch;                                               /* curl handle */
    CURLcode rcode;                                         /* curl result code */
    char jsonObj[3000], timestamp[35], free_mem[20], free_disk[20], uptime[20], cpu_usage[20], sensor_id[200] ;

    json_object *json;                                      /* json post body */
    enum json_tokener_error jerr = json_tokener_success;    /* json parse error */

    struct curl_fetch_st curl_fetch;                        /* curl fetch struct */
    struct curl_fetch_st *cf = &curl_fetch;                 /* pointer to fetch struct */
    struct curl_slist *headers = NULL;                      /* http headers to send with request */

    
    /* url to test site */
    //char *url = "http://postcatcher.in/catchers/554338ed0e99fb0300007f93";
    //char *url = "https://192.168.99.27:4443";
    //char *url = "http://packet.radiolocus.com:8080/monitoring-web/v1/g/m/u/test-neodb-vm";
	/*********ucitest.c********************/
	struct uci_context *ctx;
	struct uci_context *ctx1;
	struct uci_package *pkg1;
	struct uci_package *pkg2;
	struct list_head *q;
	struct list_head *r;
	struct uci_rl *rl;
	struct uci_sid *sid;
	char url1[200];
	char url[200];
	bool set = false;

	INIT_LIST_HEAD(&rls);
	INIT_LIST_HEAD(&sidls);
	ctx = uci_alloc_context();
	ctx1 = uci_alloc_context();
	ucimap_init(&monitoring_map);
	ucimap_init(&sensorid_map);

	if ((argc >= 2) && !strcmp(argv[1], "-s")) {
		uci_set_savedir(ctx, "./test/save");
		set = true;
	}

	uci_set_confdir(ctx, "/etc/config");
	uci_load(ctx, "alerts", &pkg1);
	uci_load(ctx1, "sensorid", &pkg2);
	ucimap_parse(&monitoring_map, pkg1);
	ucimap_parse(&sensorid_map, pkg2);
	list_for_each(q, &rls) {
		const unsigned char *ipaddr;

		rl = list_entry(q, struct uci_rl, list);
	//	ipaddr = rl->ipaddr;
	//	if (!ipaddr)
	//		ipaddr = (const unsigned char *) "\x00\x00\x00\x00";
	//	
		sprintf(url1,"%s",rl->url);
	}

	list_for_each(r, &sidls) {
		sid = list_entry(r, struct uci_sid, list);
		sprintf(sensor_id,"%s",sid->id);
		
	}
	ucimap_cleanup(&monitoring_map);
	ucimap_cleanup(&sensorid_map);
	uci_free_context(ctx);
	uci_free_context(ctx1);
	sprintf(url, "%s/%s", url1, sensor_id);
    /* init curl handle */
    if ((ch = curl_easy_init()) == NULL) {
        /* log error */
        fprintf(stderr, "ERROR: Failed to create curl handle in fetch_session");
        /* return error */
        return 1;
    }

    /* set content type */
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");

    /*****************************/
      strcpy(free_mem, runMonitoringCommand("free_memory"));
  //printf("%s", free_mem);
      strcpy(free_disk, runMonitoringCommand("free_disk"));
  //    //printf("%s", free_disk);
      strcpy(uptime, runMonitoringCommand("uptime"));
  //        //printf("%s", uptime);
      strcpy(cpu_usage, runMonitoringCommand("cpu_usage"));
      strcpy(timestamp, runMonitoringCommand("timestamp"));
    /*******************************/

    /* create json object for post */
    json = json_object_new_object();

    /* build post data */
    //json_object_object_add(json, "title", json_object_new_string("testies"));
    //json_object_object_add(json, "body", json_object_new_string("testies ... testies ... 1,2,3"));
    //json_object_object_add(json, "userId", json_object_new_int(133));
    json_object * jobj1 = json_object_new_object();
    json_object * jobj2 = json_object_new_object();
    json_object * jobj3 = json_object_new_object();
    json_object * jobj4 = json_object_new_object();
    json_object * jobj_ts = json_object_new_object();

    json_object *jstr_lbl_cpu_usage = json_object_new_string("CPU_USAGE");
    json_object *jstr_lbl_free_memory = json_object_new_string("FREE_MEMORY");
    json_object *jstr_lbl_free_disk = json_object_new_string("FREE_DISK");
    json_object *jstr_lbl_uptime = json_object_new_string("UPTIME");

    json_object *jstr_free_mem = json_object_new_double(atof(free_mem));
    json_object *jstr_free_disk = json_object_new_double(atof(free_disk));
    json_object *jstr_uptime = json_object_new_double(atof(uptime));
    json_object *jstr_cpu_usage = json_object_new_double(atof(cpu_usage));
    json_object *jstr_timestamp = json_object_new_double(atof(timestamp));

    json_object_object_add(jobj_ts,"millis", jstr_timestamp);

    json_object_object_add(jobj1,"key", jstr_lbl_cpu_usage);
    json_object_object_add(jobj1,"value", jstr_cpu_usage);
    json_object_object_add(jobj1,"sensorTs", jobj_ts);

    json_object_object_add(jobj2,"key", jstr_lbl_free_memory);
    json_object_object_add(jobj2,"value", jstr_free_mem);
    json_object_object_add(jobj2,"sensorTs", jobj_ts);

    json_object_object_add(jobj3,"key", jstr_lbl_free_disk);
    json_object_object_add(jobj3,"value", jstr_free_disk);
    json_object_object_add(jobj3,"sensorTs", jobj_ts);

    json_object_object_add(jobj4,"key", jstr_lbl_uptime);
    json_object_object_add(jobj4,"value", jstr_uptime);
    json_object_object_add(jobj4,"sensorTs", jobj_ts);

    json_object *metricArray = json_object_new_array();
    json_object_array_add(metricArray,jobj1);
    json_object_array_add(metricArray,jobj2);
    json_object_array_add(metricArray,jobj3);
    json_object_array_add(metricArray,jobj4);
    json_object_object_add(json,"monitoringMetricList", metricArray);

    //printf("%s\n",json_object_to_json_string(json));
    /* set curl options */
     #ifdef SKIP_PEER_VERIFICATION
        /*
     *      * If you want to connect to a site who isn't using a certificate that is
     *           * signed by one of the certs in the CA bundle you have, you can skip the
     *                * verification of the server's certificate. This makes the connection
     *                     * A LOT LESS SECURE.
     *                          *
     *                               * If you have a CA cert for the server stored someplace else than in the
     *                                    * default bundle, then the CURLOPT_CAPATH option might come handy for
     *                                         * you.
     *                                              */ 
        curl_easy_setopt(ch, CURLOPT_SSL_VERIFYPEER, 0L);
    #endif
     
    #ifdef SKIP_HOSTNAME_VERIFICATION
        /*
     *      * If the site you're connecting to uses a different host name that what
     *           * they have mentioned in their server certificate's commonName (or
     *                * subjectAltName) fields, libcurl will refuse to connect. You can skip
     *                     * this check, but this will make the connection less secure.
     *                          */ 
        curl_easy_setopt(ch, CURLOPT_SSL_VERIFYHOST, 0L);
    #endif

    curl_easy_setopt(ch, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(ch, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(ch, CURLOPT_POSTFIELDS, json_object_to_json_string(json));
    /* fetch page and capture return code */
    rcode = curl_fetch_url(ch, url, cf);

    /* cleanup curl handle */
    curl_easy_cleanup(ch);

    /* free headers */
    curl_slist_free_all(headers);

    /* free json object */
    json_object_put(json);

    /* check return code */
    if (rcode != CURLE_OK || cf->size < 1) {
        /* log error */
        fprintf(stderr, "ERROR: Failed to fetch url (%s) - curl said: %s",
            url, curl_easy_strerror(rcode));
        /* return error */
        return 2;
    }

    /* check payload */
    if (cf->payload != NULL) {
        /* print result */
        printf("CURL Returned: \n%s\n", cf->payload);
        /* parse return */
        json = json_tokener_parse_verbose(cf->payload, &jerr);
        /* free payload */
        free(cf->payload);
    } else {
        /* error */
        fprintf(stderr, "ERROR: Failed to populate payload");
        /* free payload */
        free(cf->payload);
        /* return */
        return 3;
    }

    /* check error */
    //if (jerr != json_tokener_success) {
    //    /* error */
    //    fprintf(stderr, "ERROR: Failed to parse json string");
    //    /* free json object */
    //    json_object_put(json);
    //    /* return */
    //    return 4;
    //}

    /* debugging */
    //printf("Parsed JSON: %s\n", json_object_to_json_string(json));

    /* exit */
    return 0;
}

