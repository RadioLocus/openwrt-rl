#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "../lib/metrics.h"
#include "../lib/prober.h"
#include "../lib/curler.h"
#define CUSTOMFLAG      1
#ifdef PC
/*do nothing*/
#else
#include "../lib/ucitest.h"
#endif



struct thread_args {
    struct Queue* queue;
    int *dropcounter;
    int *max_packets;
    int *max_tuples_in_batch;
    int *sensor_tupleversion;
    int *custom_flag;
    int *tuplecounter;
    int *dbmsignal_limit;
    int *bad_packets;
    int *bad_packets_time;
    char *interface_name;
    char *sensor_id;
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
    char *tuple;
    char *url = (char *)args->tupleurl;
    #ifdef PC
    int i = 5;
    while(i>0){
    #else
    while(1){
    #endif
        sleep(3);
        //tuple = consumer_dequeue(queue);
        tuple = consumer_dequeue_all(queue, *max_tuples_in_batch);
        if (tuple != NULL)
        {
            if (CURLE_OK != curl_post_str(url, tuple)){
                *dropCounter = *dropCounter + 1;
                enqueue(queue, tuple, 1);
            }
        }
        free(tuple);
        #ifdef PC
        i--;
        #endif
    }
}

void *pcapthread_method(void *arg)
{
    int max_packets, sensor_tupleversion, tuplecounter, custom_flag, bad_packets_time, bad_packets, dbmsignal_limit;
    char enqueue_str[200];
    struct thread_args *args = (struct thread_args *)arg;
    struct Queue* queue = args->queue; 
    char *interface_name = args->interface_name;
    char *sensor_id = args->sensor_id;
    max_packets = *(args->max_packets);
    sensor_tupleversion = *(args->sensor_tupleversion);
    tuplecounter = *(args->tuplecounter);
    custom_flag = *(args->custom_flag);
    bad_packets_time = *(args->bad_packets_time);
    bad_packets = *(args->bad_packets);
    dbmsignal_limit = *(args->dbmsignal_limit);
    generate_tuple(interface_name, queue, max_packets, sensor_tupleversion, custom_flag, tuplecounter, sensor_id, bad_packets_time, bad_packets, dbmsignal_limit);
}


#ifdef PC
/*do nothing*/
#else
/************for configs***********************/

struct list_head rls;
struct list_head sidls;
struct uci_sectionmap radiolocus;
struct uci_sectionmap sensorid;

int
radiolocus_add_interface(struct uci_map *map, void *section)
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
        }
};
struct my_optmap radiolocus_options[] = {
        {
                .map = {
                        UCIMAP_OPTION(struct uci_rl, tupleversion),
                        .type = UCIMAP_INT,
                        .name = "tupleversion",
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

struct uci_sectionmap radiolocus = {
        UCIMAP_SECTION(struct uci_rl, map),
        .type = "radiolocus",
        .alloc = radiolocus_allocate,
        .init = radiolocus_init_interface,
        .add = radiolocus_add_interface,
        .options = &radiolocus_options[0].map,
        .n_options = ARRAY_SIZE(radiolocus_options),
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

struct uci_sectionmap *radiolocus_smap[] = {
        &radiolocus,
};

struct uci_sectionmap *sensorid_smap[] = {
        &sensorid,
};


struct uci_map radiolocus_map = {
        .sections = radiolocus_smap,
        .n_sections = ARRAY_SIZE(radiolocus_smap),
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
    pthread_t tPcapThread;
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
        ucimap_init(&radiolocus_map);
        ucimap_init(&sensorid_map);

        if ((argc >= 2) && !strcmp(argv[1], "-s")) {
                uci_set_savedir(ctx, "./test/save");
                set = true;
        }

        uci_set_confdir(ctx, "/etc/config");
        uci_load(ctx, "radiolocus", &pkg1);
        uci_load(ctx1, "sensorid", &pkg2);

        ucimap_parse(&radiolocus_map, pkg1);

        ucimap_parse(&sensorid_map, pkg2);
        list_for_each(q, &rls) {
                const unsigned char *ipaddr;

                rl = list_entry(q, struct uci_rl, list);
                sensor_tupleversion = rl->tupleversion;
                noOfCurlingThreads = rl->curlingthreads;
                maxTuplesInBatch = rl->maxtuplesinbatch;
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

        }

        sprintf(server_url, "%s/%s", url1, sensor_id);
        sprintf(monitserver_url, "%s/%s", moniturl1, sensor_id);
        ucimap_cleanup(&radiolocus_map);
        ucimap_cleanup(&sensorid_map);
        uci_free_context(ctx);
        uci_free_context(ctx1);
        arguments.moniturl = monitserver_url;
        arguments.tupleurl = server_url;
        arguments.sensor_id = sensor_id;
        arguments.interface_name = dev;

#endif
    queue = createQueue(maxQueueSize);
    arguments.queue = queue;
    arguments.dropcounter = &dropCounter;
    arguments.max_packets = &max_packets;
    arguments.max_tuples_in_batch = &maxTuplesInBatch;
    arguments.sensor_tupleversion = &sensor_tupleversion;
    arguments.custom_flag = &custom_flag;
    arguments.tuplecounter = &tuplecounter;
    arguments.bad_packets = &bad_packets;
    arguments.bad_packets_time = &bad_packets_time;
    arguments.dbmsignal_limit = &dbmsignal_limit;

    tTupleSender = malloc(sizeof(pthread_t)*noOfCurlingThreads);
    pthread_create(&tDropCounter, NULL, dropcounter_method, (void *)&arguments);
    pthread_create(&tPcapThread, NULL, pcapthread_method, (void *)&arguments);
    for (i = 0; i < noOfCurlingThreads; i++)
        pthread_create(&tTupleSender[i], NULL, tuplesender_method, (void *)&arguments);
    #ifdef PC
    int j  = 10;
    while(j != 0){
    #else
    while(1){
    #endif
        sleep(5);
        int a = pthread_tryjoin_np(tPcapThread, &ret);
        //pcap thread
        if (a==EBUSY){
                //printf("pcapthread busy\n");
        }else if(a==0){
                //printf("pcapthread complete\n");
                sleep(5);
                pthread_create(&tPcapThread, NULL, pcapthread_method, (void *)&arguments);
        }else{
                //printf("pcapthread error\n");
        }
        //dropcounter thread
        a = pthread_tryjoin_np(tDropCounter, &ret);
        if (a==EBUSY){
                //printf("tDropCounter busy\n");
        }else if(a==0){
                sleep(5);
                pthread_create(&tDropCounter, NULL, dropcounter_method, (void *)&arguments);
        }else{
                //printf("tDropCounter error\n");
        }
        for (i = 0; i < noOfCurlingThreads; i++){
        a = pthread_tryjoin_np(tTupleSender[i], &ret);
        if (a==EBUSY){
                //printf("tTupleSender busy\n");
        }else if(a==0){

                printf("tTupleSender complete\n");
                sleep(5);
                pthread_create(&tTupleSender[i], NULL, tuplesender_method, (void *)&arguments);
        }else{
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
        pthread_join(tPcapThread, &ret);
        pthread_join(tDropCounter, &ret);
        for (i = 0; i < noOfCurlingThreads; i++){
            pthread_join(tTupleSender[i], &ret);
        }
        destroyQueue(queue);
        free(queue);
        free(tTupleSender);
}
