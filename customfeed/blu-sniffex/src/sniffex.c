#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <time.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include "metrics.h"
#include "prober.h"
#include "curler.h"
#define CUSTOMFLAG      1
#ifdef PC
/*do nothing*/
#else
#include "ucitest.h"
#endif

struct thread_args {
	struct Queue* queue;
	int *dropcounter;
	int *max_packets;
	int *max_tuples_in_batch;
	int *seq_interval;
	int *time_normalizer;
	int *totp_time_interval;
	int *tupleinterval;
	int *sensor_tupleversion;
	int *custom_flag;
	int *tuplecounter;
	int *dbmsignal_limit;
	int *bad_packets;
	int *bad_packets_time;
	char *interface_name;
	char *sensor_id;
	char *sensor_token;
	char *moniturl;
	char *tupleurl;
};

void *dropcounter_method(void *arg)
{
   struct Metric* metrics[1];
   char timestamp[20];
   char *metricsStr;
   int *dropCounter;
   struct thread_args *args = (struct thread_args *)arg;
   #ifdef PC
       int j = 2;
       while(j>0){
           sleep(2);
   #else
       while(1){
           sleep(60);
   #endif
   dropCounter = (int *)args->dropcounter;
   sprintf(timestamp, "%u\n", (unsigned)time(NULL)); 
   metrics[0] = createMetric("DROP_COUNT", *dropCounter, atof(timestamp));
   json_object* metricJson = createMetricJson(metrics,1);
   metricsStr = json_object_to_json_string(metricJson);
   #ifdef PC
   printf("^^ metricstr %s\n",metricsStr);
   #endif
   curl_post_jsonstr(args->moniturl, metricsStr);
   #ifdef PC
   printf("^^ metricstr %s\n",metricsStr);
   #endif
   //json_object_put(metricJson);
   free(metrics[0]);
   free(metricsStr);
   #ifdef PC
   j--;}
   #else
   printf("json sent\n");
   }
   #endif
}


void *tuplesender_method(void *arg)
{
    struct thread_args *args = (struct thread_args *)arg;
    struct Queue* queue = (struct Queue*)args->queue;
    int *dropCounter = (int *)args->dropcounter;
    int *max_tuples_in_batch = (int *)args->max_tuples_in_batch;
    int *tupleinterval = (int *)args->tupleinterval;
    char *tuple;
    char *url = (char *)args->tupleurl;
    int while_ctr=0;
    #ifdef PC
    int i = 5;
    while(i>0){
    #else
    while(1){
    #endif
        //sleep(3);
        if (while_ctr > 100){
            sleep(3);
        }
        //tuple = consumer_dequeue(queue);
        tuple = consumer_dequeue_all(queue, *max_tuples_in_batch);
        //printf("tuplesender %s\n", tuple);
        if (tuple != NULL)
        {
            if (CURLE_OK != curl_post_str(url, tuple)){
                *dropCounter = *dropCounter + 1;
                enqueue(queue, tuple, 1);
            }
            while_ctr=0;
        }
        while_ctr++;
        free(tuple);
        #ifdef PC
        i--;
        #endif
    }
}

void begin_inquiry(int dd) {
	int err;

	inquiry_cp icp;
	memset (&icp, 0, sizeof(icp));
	icp.lap[2] = 0x9e;
	icp.lap[1] = 0x8b;
	icp.lap[0] = 0x33;
	icp.num_rsp = 0;
	icp.length = 0x30;

	err = hci_send_cmd(dd, OGF_LINK_CTL, OCF_INQUIRY, INQUIRY_CP_SIZE, &icp);
	if (err < 0) {
		printf(stderr,"Error #%d beginning inquiry\n",err);
		exit(1);
	}
}

struct hci_request ble_hci_request(uint16_t ocf, int clen, void * status, void * cparam)
{
	struct hci_request rq;
	memset(&rq, 0, sizeof(rq));
	rq.ogf = OGF_LE_CTL;
	rq.ocf = ocf;
	rq.cparam = cparam;
	rq.clen = clen;
	rq.rparam = status;
	rq.rlen = 1;
	return rq;
}

void *hcithread_method(void *args) {
	struct pollfd fd;
	unsigned char buf[HCI_MAX_EVENT_SIZE], *ptr;
	hci_event_hdr *hdr;
	struct hci_filter flt;
	int i, totp, len, ret, status;
	struct timeval tv;
	char tuple[1000], totp_key_str[200];
	evt_le_meta_event * meta_event;
	le_advertising_info * info;
	uint8_t num, reports_count;

	Configuration* conf = (Configuration * ) args;
	int dbmsignal_limit = conf->dbmsignal_limit;
	int seqinterval = conf->seqinterval;
	int timenormalizer = conf->timenormalizer;
	int totptimeinterval = conf->totptimeinterval;
	int sensor_tupleversion = conf->sensor_tupleversion;
	int tuplecounter = conf->tuplecounter;
	int sensor_customflag = conf->custom_flag;
	struct Queue *queue = conf->queue;
	char *sensor_id = conf->sensor_id;
	char *sensor_token = conf->sensor_token;

	int dev_id = hci_get_route(NULL);
	int dd = hci_open_dev(dev_id);
	if (dd < 0) {
		printf("Can't open HCI device\n");
		exit(1);
	} else {
		printf("Opened socket to hci%d\n", dd);
	}

	hci_filter_clear(&flt);
	hci_filter_set_ptype(HCI_EVENT_PKT, &flt);
	hci_filter_set_event(EVT_LE_META_EVENT, &flt);
	hci_filter_set_event(EVT_INQUIRY_RESULT_WITH_RSSI, &flt);
	hci_filter_set_event(EVT_INQUIRY_COMPLETE, &flt);
	if (setsockopt(dd, SOL_HCI, HCI_FILTER, &flt, sizeof(flt)) < 0) {
		printf("Can't set HCI filter\n");
		exit(1);
	}
	
	/*send command inquiry mode with rssi result*/
	write_inquiry_mode_cp wicp;
	wicp.mode = 0x01;
	ret = hci_send_cmd(dd, OGF_HOST_CTL, OCF_WRITE_INQUIRY_MODE, WRITE_INQUIRY_MODE_RP_SIZE, &wicp);
        if (ret < 0) {
                hci_close_dev(dd);
                printf("Failed to set inquiry mode on device\n");
                exit(1);
        } else {
		printf("Inquiry mode enabled\n");
	}

	/*set BLE scan parameters*/
	le_set_scan_parameters_cp scan_params_cp;
	memset(&scan_params_cp, 0, sizeof(scan_params_cp));
	scan_params_cp.type = 0x00; 
	scan_params_cp.interval	= htobs(0x0010);
	scan_params_cp.window = htobs(0x0010);
	scan_params_cp.own_bdaddr_type = 0x00; // Public Device Address (default).
	scan_params_cp.filter = 0x00; // Accept all.
	struct hci_request scan_params_rq = ble_hci_request(OCF_LE_SET_SCAN_PARAMETERS, LE_SET_SCAN_PARAMETERS_CP_SIZE, &status, &scan_params_cp);
	ret = hci_send_req(dd, &scan_params_rq, 1000);
	if (ret < 0) {
		hci_close_dev(dd);
		printf("Failed to set BLE scan parameters data\n");
		exit(1);
	} else {
		printf("BLE scan parameters set\n");
	}

	/*set BLE events report mask*/
	le_set_event_mask_cp event_mask_cp;
	memset(&event_mask_cp, 0, sizeof(le_set_event_mask_cp));
	for (i=0; i<8; i++)
		event_mask_cp.mask[i] = 0xFF;
	struct hci_request set_mask_rq = ble_hci_request(OCF_LE_SET_EVENT_MASK, LE_SET_EVENT_MASK_CP_SIZE, &status, &event_mask_cp);
	ret = hci_send_req(dd, &set_mask_rq, 1000);
	if (ret < 0) {
		hci_close_dev(dd);
		printf("Failed to set LE event mask\n");
		exit(1);
	} else {
		printf("BLE events mask set\n");
	}

	/*enable BLE scanning*/
	le_set_scan_enable_cp scan_cp;
	memset(&scan_cp, 0, sizeof(scan_cp));
	scan_cp.enable = 0x01;	//enable flag
	scan_cp.filter_dup = 0x00; //filtering disabled
	struct hci_request enable_adv_rq = ble_hci_request(OCF_LE_SET_SCAN_ENABLE, LE_SET_SCAN_ENABLE_CP_SIZE, &status, &scan_cp);
	ret = hci_send_req(dd, &enable_adv_rq, 1000);
	if (ret < 0) {
		hci_close_dev(dd);
		printf("failed to enable BLE scan\n");
		exit(1);
	} else {
		printf("BLE scanning enabled\n");
	}

	fd.fd = dd;
	fd.events = POLLIN;
	fd.revents = 0;

	while(1) {
		if(poll(&fd, 1, -1) > 0) {
			len = read(dd, buf, sizeof(buf));
			if (len < 0)
				continue;
			else if (len == 0)
				break;
			hdr = (void *) (buf + 1);
			ptr = buf + (1 + HCI_EVENT_HDR_SIZE);
			len -= (1 + HCI_EVENT_HDR_SIZE);
			switch (hdr->evt) {
			case EVT_LE_META_EVENT:
				printf("Received le_meta_event\n");
				meta_event = (evt_le_meta_event*) ptr;
				if (meta_event->subevent == EVT_LE_ADVERTISING_REPORT) {
					printf("Received le advertising report\n");
					reports_count = meta_event->data[0];
					void * offset = meta_event->data + 1;
					while (reports_count--) {
						info = (le_advertising_info*)offset;
						char addr[18];
						ba2str(&(info->bdaddr), addr);
						int rssi = (int)info->data[info->length];
						printf("%s - %d\n", addr, (char)info->data[info->length]);
						if (rssi > dbmsignal_limit) {
							gettimeofday(&tv, NULL);
							if (sensor_tupleversion == 1) {
								conf->tuplecounter++;
								char float_str[25];
								sprintf(float_str,"%ld.%.6ld", tv.tv_sec, tv.tv_usec);
								double d_timestamp = string_to_double(float_str);
								d_timestamp = d_timestamp * 1000;
								d_timestamp = floor(d_timestamp)/ 1000;
								long l_timestamp = (long) d_timestamp;
								int timestamp = l_timestamp; /// 1000;
								int normalized_ts = get_normalized_time(timestamp, timenormalizer);
								// generating the key
								sprintf(totp_key_str,"%s%s%d", sensor_token, sensor_id, tuplecounter / seqinterval );
								totp = generateTOTPUsingTimestamp(totp_key_str, 8, normalized_ts);
								printf("%s - %d\n", addr, (char)info->data[info->length]);
								sprintf(tuple, "%d,%s,%d,%d,%ld.%.6ld,%s,%d,%d,%d\n", sensor_tupleversion, sensor_id, tuplecounter, queue->size, tv.tv_sec, tv.tv_usec, addr, rssi, sensor_customflag, totp);
							} else {
								printf("Unsupported tupleversion");
								exit(1);
							}
							enqueue(queue, tuple, 0);
						}
						offset = info->data + info->length + 2;
					}
				}
				break;
			case EVT_INQUIRY_RESULT_WITH_RSSI:
				printf("Received inquiry info with rssi\n");
				num = buf[0];
				for (i = 0; i < num; i++) {
					inquiry_info_with_rssi *info = (void *) buf + (sizeof(*info) * i) + 1;
					char addr[18];
					ba2str(&(info->bdaddr), addr);
					int rssi = info->rssi;
					printf("%s - %d", addr, rssi);
					if (rssi > dbmsignal_limit) {
						gettimeofday(&tv, NULL);
						if (sensor_tupleversion == 1) {
							conf->tuplecounter++;
							char float_str[25];
							sprintf(float_str,"%ld.%.6ld", tv.tv_sec, tv.tv_usec);
							double d_timestamp = string_to_double(float_str);
							d_timestamp = d_timestamp * 1000;
							d_timestamp = floor(d_timestamp)/ 1000;
							long l_timestamp = (long) d_timestamp;
							int timestamp = l_timestamp; /// 1000;
							int normalized_ts = get_normalized_time(timestamp, timenormalizer);
							// generating the key
							sprintf(totp_key_str,"%s%s%d", sensor_token, sensor_id, tuplecounter / seqinterval );
							totp = generateTOTPUsingTimestamp(totp_key_str, 8, normalized_ts);
							sprintf(tuple, "%d,%s,%d,%d,%ld.%.6ld,%s,%d,%d,%d\n", sensor_tupleversion, sensor_id, tuplecounter, queue->size, tv.tv_sec, tv.tv_usec, addr, rssi, sensor_customflag, totp);
						} else {
							printf("Unsupported tupleversion");
							exit(1);
						}
						enqueue(queue, tuple, 0);
					}
				}
				break;
			case EVT_INQUIRY_COMPLETE:
				printf("received inquiry complete\n");
				// maybe add some sleep here
				begin_inquiry(dd);
				break;
			}
		}
	}
	
}

#ifdef PC
/*do nothing*/
#else
/************for configs***********************/

struct list_head rls;
struct list_head sidls;
struct uci_sectionmap bluetooth;
struct uci_sectionmap sensorid;

int
bluetooth_add_interface(struct uci_map *map, void *section)
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

struct my_optmap sensorid_options[] = {
        {
                .map = {
                        UCIMAP_OPTION(struct uci_sid, id),
                        .type = UCIMAP_STRING,
                        .name = "id",
                        .data.s.maxlen = 170,
                }
        },
        {
                .map = {
                        UCIMAP_OPTION(struct uci_sid, sensortoken),
                        .type = UCIMAP_STRING,
                        .name = "sensortoken",
                        .data.s.maxlen = 170,
                }
        }
};
struct my_optmap bluetooth_options[] = {
        {
                .map = {
                        UCIMAP_OPTION(struct uci_rl, tupleversion),
                        .type = UCIMAP_INT,
                        .name = "tupleversion",
                }
        },
        {
                .map = {
                        UCIMAP_OPTION(struct uci_rl, tupleinterval),
                        .type = UCIMAP_INT,
                        .name = "tupleinterval",
                }
        },
        {
                .map = {
                        UCIMAP_OPTION(struct uci_rl, curlingthreads),
                        .type = UCIMAP_INT,
                        .name = "curlingthreads",
                }
        },
        {
                .map = {
                        UCIMAP_OPTION(struct uci_rl, seqinterval),
                        .type = UCIMAP_INT,
                        .name = "seqinterval",
                }
        },
        {
                .map = {
                        UCIMAP_OPTION(struct uci_rl, timenormalizer),
                        .type = UCIMAP_INT,
                        .name = "timenormalizer",
                }
        },
        {
                .map = {
                        UCIMAP_OPTION(struct uci_rl, totptimeinterval),
                        .type = UCIMAP_INT,
                        .name = "totptimeinterval",
                }
        },
        {
                .map = {
                        UCIMAP_OPTION(struct uci_rl, maxtuplesinbatch),
                        .type = UCIMAP_INT,
                        .name = "maxtuplesinbatch",
                }
        },
        {
                .map = {
                        UCIMAP_OPTION(struct uci_rl, dbmsignallimit),
                        .type = UCIMAP_INT,
                        .name = "dbmsignallimit",
                }
        },
        {
                .map = {
                        UCIMAP_OPTION(struct uci_rl, badpacketslimit),
                        .type = UCIMAP_INT,
                        .name = "badpacketslimit",
                }
        },
        {
                .map = {
                        UCIMAP_OPTION(struct uci_rl, badpacketstimerlimit),
                        .type = UCIMAP_INT,
                        .name = "badpacketstimerlimit",
                }
        },
        {
                .map = {
                        UCIMAP_OPTION(struct uci_rl, id),
                        .type = UCIMAP_STRING,
                        .name = "id",
                        .data.s.maxlen = 170,
                }
        },
        {
                .map = {
                        UCIMAP_OPTION(struct uci_rl, url),
                        .type = UCIMAP_STRING,
                        .name = "url",
                        .data.s.maxlen = 170,
                }
        },
        {
                .map = {
                        UCIMAP_OPTION(struct uci_rl, moniturl),
                        .type = UCIMAP_STRING,
                        .name = "moniturl",
                        .data.s.maxlen = 170,
                }
        },
        {
                .map = {
                        UCIMAP_OPTION(struct uci_rl, moninf),
                        .type = UCIMAP_STRING,
                        .name = "moninf",
                        .data.s.maxlen = 70,
                }
        }
};

struct uci_sectionmap bluetooth = {
        UCIMAP_SECTION(struct uci_rl, map),
        .type = "bluetooth",
        .alloc = bluetooth_allocate,
        .init = bluetooth_init_interface,
        .add = bluetooth_add_interface,
        .options = &bluetooth_options[0].map,
        .n_options = ARRAY_SIZE(bluetooth_options),
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

struct uci_sectionmap *bluetooth_smap[] = {
        &bluetooth,
};

struct uci_sectionmap *sensorid_smap[] = {
        &sensorid,
};


struct uci_map bluetooth_map = {
        .sections = bluetooth_smap,
        .n_sections = ARRAY_SIZE(bluetooth_smap),
};

struct uci_map sensorid_map = {
        .sections = sensorid_smap,
        .n_sections = ARRAY_SIZE(sensorid_smap),
};

#endif

int main(int argc, char **argv)
{
	struct Queue* queue;
	struct thread_args arguments;
	int dropCounter = 0;
	pthread_t tHciThread;
	pthread_t *tTupleSender;
	pthread_t tDropCounter;
	int i;
	void* ret;
	int maxQueueSize = 10000;
	int maxTuplesInBatch = 2000;
	int noOfCurlingThreads = 5;
	int max_packets = -1;
	int sensor_tupleversion = 3;
	int tuplecounter = 0;
	int custom_flag = 1;
	int dbmsignal_limit = -200;
	int bad_packets = 200;
	int bad_packets_time = 600;
	int tupleinterval = 1000;
	int seqinterval = 1000;
	int timenormalizer = 30;
	int totptimeinterval = 30;

#ifdef PC1

	arguments.moniturl = "http://packet.radiolocus.com/monitoring-web/v1/g/m/u/test123";
	arguments.tupleurl = "http://packet.radiolocus.com/v1/p/u/test123";
	arguments.sensor_id = "test123";
	arguments.interface_name = "wlan0-mon0";

#else

	char dev[200];
	char url1[200];
	char server_url[200];
	char monitserver_url[200];
	char moniturl1[200];
	char sensor_id[200];
	char sensor_token[200];
	bool set = false;
	struct uci_context *ctx;
	struct uci_context *ctx1;
	struct uci_package *pkg1;
	struct uci_package *pkg2;
	struct list_head *q;
	struct list_head *r;
	struct uci_rl *rl;
	struct uci_sid *sid;
	INIT_LIST_HEAD(&rls);
	INIT_LIST_HEAD(&sidls);
	ctx = uci_alloc_context();
	ctx1 = uci_alloc_context();
	ucimap_init(&bluetooth_map);
	ucimap_init(&sensorid_map);


	if ((argc >= 2) && !strcmp(argv[1], "-s")) {
		uci_set_savedir(ctx, "./test/save");
		set = true;
	}

	uci_set_confdir(ctx, "/etc/config");
	uci_load(ctx, "bluetooth", &pkg1);
	uci_load(ctx1, "sensorid", &pkg2);

	ucimap_parse(&bluetooth_map, pkg1);
	ucimap_parse(&sensorid_map, pkg2);

	list_for_each(q, &rls) {
		const unsigned char *ipaddr;
		rl = list_entry(q, struct uci_rl, list);
		sensor_tupleversion = rl->tupleversion;
		noOfCurlingThreads = rl->curlingthreads;
		tupleinterval = rl->tupleinterval;
		maxTuplesInBatch = rl->maxtuplesinbatch;
		seqinterval = rl->seqinterval;
		timenormalizer = rl->timenormalizer;
		totptimeinterval = rl->totptimeinterval;
		dbmsignal_limit = rl->dbmsignallimit;
		bad_packets = rl->badpacketslimit;
		bad_packets_time = rl->badpacketstimerlimit;
		custom_flag = CUSTOMFLAG;
		sprintf(url1,"%s",rl->url);
		sprintf(moniturl1,"%s",rl->moniturl);
		sprintf(dev,"%s",rl->moninf);
	}

	list_for_each(r, &sidls) {
		sid = list_entry(r, struct uci_sid, list);
		sprintf(sensor_id,"%s",sid->id);
		sprintf(sensor_token,"%s",sid->sensortoken);
	}

	sprintf(server_url, "%s/%s", url1, sensor_id);
	sprintf(monitserver_url, "%s/%s", moniturl1, sensor_id);
	ucimap_cleanup(&bluetooth_map);
	ucimap_cleanup(&sensorid_map);
	uci_free_context(ctx);
	uci_free_context(ctx1);
	arguments.moniturl = monitserver_url;
	arguments.tupleurl = server_url;
	arguments.sensor_id = sensor_id;
	arguments.sensor_token = sensor_token;
	arguments.interface_name = dev;

#endif

	queue = createQueue(maxQueueSize);
	arguments.queue = queue;
	arguments.dropcounter = &dropCounter;
	arguments.max_packets = &max_packets;
	arguments.seq_interval = &seqinterval;
	arguments.time_normalizer = &timenormalizer;
	arguments.totp_time_interval = &totptimeinterval;
	arguments.max_tuples_in_batch = &maxTuplesInBatch;
	arguments.tupleinterval = &tupleinterval;
	arguments.sensor_tupleversion = &sensor_tupleversion;
	arguments.custom_flag = &custom_flag;
	arguments.tuplecounter = &tuplecounter;
	arguments.bad_packets = &bad_packets;
	arguments.bad_packets_time = &bad_packets_time;
	arguments.dbmsignal_limit = &dbmsignal_limit;

	tTupleSender = malloc(sizeof(pthread_t)*noOfCurlingThreads);
	pthread_create(&tDropCounter, NULL, dropcounter_method, (void *)&arguments);
	pthread_create(&tHciThread, NULL, hcithread_method, (void *)&arguments);
	for (i = 0; i < noOfCurlingThreads; i++)
	pthread_create(&tTupleSender[i], NULL, tuplesender_method, (void *)&arguments);
	#ifdef PC
	int j  = 10;
	while(j != 0) {
	#else
	while(1) {
	#endif
		sleep(5);

		//pcap thread
		int a = pthread_tryjoin_np(tHciThread, &ret);
		if (a == EBUSY) {
			//printf("hci thread busy\n");
		} else if (a == 0) {
			//printf("hci thread complete\n");
			sleep(5);
			pthread_create(&tHciThread, NULL, hcithread_method, (void *)&arguments);
		} else {
			//printf("hci thread error\n");
		}

		//dropcounter thread
		a = pthread_tryjoin_np(tDropCounter, &ret);
		if (a == EBUSY) {
			//printf("tDropCounter busy\n");
		} else if (a == 0) {
                        //printf("tDropCounter complete\n");
			sleep(5);
			pthread_create(&tDropCounter, NULL, dropcounter_method, (void *)&arguments);
		} else {
			//printf("tDropCounter error\n");
		}

		for (i = 0; i < noOfCurlingThreads; i++) {
			a = pthread_tryjoin_np(tTupleSender[i], &ret);
			if (a == EBUSY) {
				//printf("tTupleSender busy\n");
			} else if (a == 0) {
				//printf("tTupleSender complete\n");
				sleep(5);
				pthread_create(&tTupleSender[i], NULL, tuplesender_method, (void *)&arguments);
			} else {
				//printf("tTupleSender error\n");
			}
		}
	#ifdef PC
		j--;
	#endif
	}
	#ifdef PC
	printf("done\n");
	#endif
	pthread_join(tHciThread, &ret);
	pthread_join(tDropCounter, &ret);
	for (i = 0; i < noOfCurlingThreads; i++){
		pthread_join(tTupleSender[i], &ret);
	}
	destroyQueue(queue);
	free(queue);
	free(tTupleSender);
}
