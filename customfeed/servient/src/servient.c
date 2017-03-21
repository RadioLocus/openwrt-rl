#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <json-c/json.h>
#include <json-c/json_tokener.h>
#include "ucitest.h"
#include "curler.h"
#include "metrics.h"

#define FLAG_RPC_FAILURE 0
#define FLAG_RPC_SUCCESS 1
#define FLAG_RUN_KALIVE 2
#define INVALID_DATA 0
#define VALID_DATA 1
#define ID "id"
#define VERSION "version"
#define DATA "data"
#define EVENT "event"
#define COMMENT "comment"
#define SUCCESSVALUE "successValue"
#define TOKEN "token"
#define REMOTEAUTH "remoteAuth"
#define RETRY "retry"
#define SPLIT_DELIMETER_STRING "[="
#define SPLIT_DELIMETER_WORD "=, "
#define FALSE 0
#define TRUE 1
#define CURL_TO_SEND_STATUS_2 "/token/"
#define CURL_TO_SEND_STATUS_3 "/status/"
#define POSTFIELD_TO_SEND_STATUS_1 "sensor="
#define POSTFIELD_TO_SEND_STATUS_2 "&token="
#define POSTFIELD_TO_SEND_STATUS_3 "&status="
#define POSTFIELD_TO_SEND_STATUS_4 "&output="
#define CURL_TO_SEND_KEEPALIVE_2 "/token/"
#define POSTFIELD_TO_SEND_KEEPALIVE_1 "sensor="
#define POSTFIELD_TO_SEND_KEEPALIVE_2 "&token="

/*****************************/
// for self auth SSL 
#define SKIP_PEER_VERIFICATION 0
#define SKIP_HOSTNAME_VERIFICATION 0

/*****************************/

char *eventData;
char *tokenForSensor;
char *eventType;
char *bashOutput;
int flag=FLAG_RUN_KALIVE;
int validate=INVALID_DATA;
char sensor_id[200];
char g_keepalive_url[200];
char g_exit_status_url[200];
char g_remoteAuth[200];
char g_sensortoken[200];
char sensor_id[200];
char channel_url1[200];
char g_monit_url[200];
int channel_sleep;
int g_sslselfsigned;

int g_rpc_timeout, g_max_curl_count, g_keepalive_sleep, g_exit_status_sleep, g_channel_sleep, g_aliveTimeInterval, g_aliveTime=0, doNotKillFlag=0,g_curl_timeout ;
pid_t pid, ppid;

time_t startTime=0;
pthread_t tKLive;
pthread_t tExecutor;


struct list_head rls;
struct uci_sectionmap servient;

struct list_head sidls;
struct uci_sectionmap sensorid;

FILE *logger;

int
servient_add_interface(struct uci_map *map, void *section)
{
        struct uci_servient *rl = section;

        list_add_tail(&rl->list, &rls);

        return 0;
}
int
servient_init_interface(struct uci_map *map, void *section, struct uci_section *s)
{
        struct uci_servient *rl = section;

        INIT_LIST_HEAD(&rl->list);
        return 0;
}
struct ucimap_section_data *
servient_allocate(struct uci_map *map, struct uci_sectionmap *sm, struct uci_section *s)
{
        struct uci_servient *p = malloc(sizeof(struct uci_servient));
        memset(p, 0, sizeof(struct uci_servient));
        return &p->map;
}

int
sensorid_add_interface(struct uci_map *map, void *section)
{
        struct uci_sid *sid = section;

        list_add_tail(&sid->list, &sidls);

        return 0;
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


struct my_optmap servient_options[] = {
	{
		.map = {
			UCIMAP_OPTION(struct uci_servient, channel_url),
			.type = UCIMAP_STRING,
			.name = "channel_url",
			.data.s.maxlen = 200,
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_servient, channel_sleep),
			.type = UCIMAP_INT,
			.name = "channel_sleep",
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_servient, sslselfsigned),
			.type = UCIMAP_INT,
			.name = "sslselfsigned",
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_servient, keepalive_url),
			.type = UCIMAP_STRING,
			.name = "keepalive_url",
			.data.s.maxlen = 200,
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_servient, exit_status_url),
			.type = UCIMAP_STRING,
			.name = "exit_status_url",
			.data.s.maxlen = 200,
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_servient, monit_url),
			.type = UCIMAP_STRING,
			.name = "monit_url",
			.data.s.maxlen = 200,
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_servient, rpc_timeout),
			.type = UCIMAP_INT,
			.name = "rpc_timeout",
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_servient, curl_timeout),
			.type = UCIMAP_INT,
			.name = "curl_timeout",
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_servient, max_curl_count),
			.type = UCIMAP_INT,
			.name = "max_curl_count",
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_servient, keepalive_sleep),
			.type = UCIMAP_INT,
			.name = "keepalive_sleep",
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_servient, aliveTimeInterval),
			.type = UCIMAP_INT,
			.name = "aliveTimeInterval",
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_servient, exit_status_sleep),
			.type = UCIMAP_INT,
			.name = "exit_status_sleep",
		},

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
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_sid, sensortoken),
			.type = UCIMAP_STRING,
			.name = "sensortoken",
			.data.s.maxlen = 200,
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_sid, servertoken),
			.type = UCIMAP_STRING,
			.name = "servertoken",
			.data.s.maxlen = 200,
		}
	},

};


struct uci_sectionmap servient = {
	UCIMAP_SECTION(struct uci_servient, map),
	.type = "servient",
	.alloc = servient_allocate,
	.init = servient_init_interface,
	.add = servient_add_interface,
	.options = &servient_options[0].map,
	.n_options = ARRAY_SIZE(servient_options),
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


struct uci_sectionmap *servient_smap[] = {
	&servient,
};

struct uci_map servient_map = {
	.sections = servient_smap,
	.n_sections = ARRAY_SIZE(servient_smap),
};


struct uci_sectionmap *sensorid_smap[] = {
	&sensorid,
};

struct uci_map sensorid_map = {
	.sections = sensorid_smap,
	.n_sections = ARRAY_SIZE(sensorid_smap),
};



struct sharedData
{
	CURL *curl;
	char *tokenForSensor;
	char *eventData;
	char *eventType;
	char *id;
	char *retry;
	char *comment;
	char *remoteAuth;
	int successValue;
	int version;
};

typedef struct sharedData dataSet;

struct json_object* json_validate(const char *str, enum json_tokener_error *error)
{
	struct json_tokener* tok;
	struct json_object* obj;

	tok = json_tokener_new();
	if (!tok)
		return NULL;
	obj = json_tokener_parse_ex(tok, str, strlen(str));
	*error = tok->err;
	if(tok->err != json_tokener_success) {
		if (obj != NULL)
			json_object_put(obj);
		obj = NULL;
	}

	json_tokener_free(tok);
	return obj;
}

/*int sentMonitReq(char *key, int value){
   
    char monitmetrics[500], monitURL[200];
    int count=0, ret;
    sprintf(monitmetrics, "{ \"monitoringMetricList\": [ { \"key\": \"%s\", \"value\": %d, \"sensorTs\": { \"millis\": %d } } ] }", key, value, time(NULL));
    fprintf(logger, monitmetrics);
    sprintf(monitURL, "http://packet.radiolocus.com/v1/p/u/%s", sensor_id);
    fprintf(logger, monitURL);
    ret = curl_post_str(monitURL , monitmetrics);
    while (ret != CURLE_OK && count <= 3){
       ret = curl_post_str(monitURL , monitmetrics);
       count++;
    }
    return ret;
}*/




/*METHOD:valiating Incoming Data*/
int validateData(char *data,dataSet **inputData){
	struct json_object *json_obj;
	struct json_object *json_obj_id;
	struct json_object *json_obj_version;
	struct json_object *json_obj_data;
	struct json_object *json_obj_retry;
	struct json_object *json_obj_event;
	struct json_object *json_obj_comment;
	struct json_object *json_obj_token;
	struct json_object *json_obj_remoteAuth;
	struct json_object *json_obj_successValue;

	enum json_tokener_error jerr;
        //fprintf(logger,"inside validate orginal data=%s\n",data);
        //fflush(logger);
        char strTrunc[10];
        char strData[200];
        strncpy (strTrunc, data, 5);
        strTrunc[5]='\0';
        if(strcmp(strTrunc,"data:")==0){
        strncpy(strData,data+5,strlen(data));
        }else{
        return INVALID_DATA;
        }
        //fprintf(logger,"inside validate orginal data=%s\n",strData);
        //fflush(logger);

	//fprintf(logger,"tuple is gonna be validated\n");
       	//fflush(logger);
        json_obj=json_validate(strData, &jerr);        
	//fprintf(logger,"tuple is validated now\n");
       	//fflush(logger);
	
	if(json_obj==NULL){
		//fprintf(logger,"tuple is invalid\n");
        	//fflush(logger);
		return INVALID_DATA;
	}else{
		//fprintf(logger,"1 finding version version \n");
        	//fflush(logger);
		json_obj_version = json_object_object_get(json_obj, VERSION);
		(*inputData)->version = json_object_get_int(json_obj_version);
		//fprintf(logger,"2 finding version version %d\n", (*inputData)->version);
        	//fflush(logger);

		if ((*inputData)->version == 1){
		//fprintf(logger,"inside version 1\n");
        	//fflush(logger);
			json_obj_event = json_object_object_get(json_obj, EVENT);
			if(json_obj_event==NULL)
				return INVALID_DATA;
			(*inputData)->eventType = json_object_get_string(json_obj_event);
			//fprintf(logger,"inside version 1 EVENT done\n");
        		//fflush(logger);

			json_obj_data = json_object_object_get(json_obj, DATA);
			//fprintf(logger,"1 finding data\n");
			if(json_obj_data==NULL){
				//fprintf(logger,"2 finding data\n");
				return INVALID_DATA;
			}
			(*inputData)->eventData = json_object_get_string(json_obj_data);
			//fprintf(logger,"inside version 1 DATA done\n");
        		//fflush(logger);

			json_obj_token = json_object_object_get(json_obj, TOKEN);
			if(json_obj_token==NULL)
				return INVALID_DATA;
			(*inputData)->tokenForSensor=json_object_get_string(json_obj_token);
			//fprintf(logger,"inside version 1 token done\n");
        		//fflush(logger);

			json_obj_remoteAuth = json_object_object_get(json_obj, REMOTEAUTH);
			if(json_obj_remoteAuth==NULL)
				return INVALID_DATA;
			(*inputData)->remoteAuth=json_object_get_string(json_obj_remoteAuth);
			//fprintf(logger,"inside version 1 remoteauth done\n");
        		//fflush(logger);
			//fprintf(logger,"tuple is valid\n");
        		//fflush(logger);


		} else{
			//fprintf(logger,"version of received tuple is %s \n", (*inputData)->version);
        		//fflush(logger);
			return INVALID_DATA;
		}

	}
	//fprintf(logger,"tuple with  version %d  is valid\n", (*inputData)->version);
        //fflush(logger);
	return VALID_DATA;
}

/*METHOD:test method i.e. bash-executor(dummy RPC)*/
int bashExecutor(char *eventData)
{
	//sleep(80);
	FILE *fp;
	int status;
	char path[1024];
        //char *output;

	fp=popen(eventData,"r");
	if(fp==NULL){
		//printf("ERROR\n");
	}
        free(bashOutput);
        bashOutput = (char *) malloc(1);
        bashOutput[0] = '\0'; 
	while(fgets(path,1024,fp)!=NULL){
                bashOutput = (char *) realloc(bashOutput, strlen(bashOutput)+1024);
                strcat(bashOutput, path);
        }
	status=pclose(fp);
        //fprintf(logger, "done bash executing with status %d, ## output is %s\n", status, bashOutput);
	//fflush(logger);
	if(status!=0){
		return 0;
	}
	else{
		return 1;
	}}

/*METHOD:execute CURL*/
CURLcode curlExecutor(CURL *curl,char url[],char postField[])
{
        struct curl_slist *headers=NULL;
        headers = curl_slist_append(headers, "Content-Type: text/plain");
	CURLcode res;
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postField);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(postField));
	if (g_sslselfsigned !=1){
    //#ifdef SKIP_PEER_VERIFICATION
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    //#endif

    //#ifdef SKIP_HOSTNAME_VERIFICATION
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    //#endif
	}else{
	#ifdef SKIP_PEER_VERIFICATION
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    	#endif

    	#ifdef SKIP_HOSTNAME_VERIFICATION
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    	#endif
	}

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	res = curl_easy_perform(curl);
	if(res != CURLE_OK){
		fprintf(stderr, "ERROR:CURL EXECUTION FAILED: %s\n",
				curl_easy_strerror(res));
		fflush(logger);
	}
        curl_slist_free_all(headers);
	return res;
}

/*METHOD:send STATUS to server*/
CURLcode callToSendStatus(CURL *curl,char *status,char *tokenForSensor){
	//fprintf(logger, "INSIDE callToSendStatus\n");
	//fflush(logger);
        char urlForSendingResultPart1[200];
        sprintf(urlForSendingResultPart1,"%s", g_exit_status_url);
	strcat(urlForSendingResultPart1,sensor_id);
	//fprintf(logger, "INSIDE callToSendStatus urlForSendingResultPart1 %s \n", urlForSendingResultPart1);
	//fflush(logger);

	char urlForSendingResultPart2[200]=CURL_TO_SEND_STATUS_2;
	strcat(urlForSendingResultPart2,tokenForSensor);
	//fprintf(logger, "INSIDE callToSendStatus urlForSendingResultPart2 %s \n", urlForSendingResultPart2);
	//fflush(logger);

	char urlForSendingResultPart3[200]=CURL_TO_SEND_STATUS_3;
	strcat(urlForSendingResultPart3,status);

	strcat(urlForSendingResultPart1,urlForSendingResultPart2);
	strcat(urlForSendingResultPart1,urlForSendingResultPart3);
	//fprintf(logger, "INSIDE callToSendStatus part2 urlForSendingResultPart1 %s bash %s \n", urlForSendingResultPart1, bashOutput);
	//fflush(logger);
	if(bashOutput == NULL)
		bashOutput="";

	char *pField4 = (char *) malloc(1);
        pField4[0] = '\0';
        pField4 = (char *) realloc(pField4, strlen(POSTFIELD_TO_SEND_STATUS_4)+strlen(bashOutput)+1);
        strcat(pField4,POSTFIELD_TO_SEND_STATUS_4);
	strcat(pField4,bashOutput);

        //fprintf(logger, "%u sending keepalive -> inside THREAD:KEEPALIVE status: %s\n",(unsigned)time(NULL), status);
        //fflush(logger);
	return curlExecutor(curl,urlForSendingResultPart1,pField4);
}

CURLcode callToSendKAlive(CURL *curl, char *tokenForSensor){
    char urlForSendingKALivePart1[200];
    sprintf(urlForSendingKALivePart1, "%s", g_keepalive_url);
    strcat(urlForSendingKALivePart1,sensor_id);

    char urlForSendingKALivePart2[200]=CURL_TO_SEND_KEEPALIVE_2;
    strcat(urlForSendingKALivePart2,tokenForSensor);
    strcat(urlForSendingKALivePart1,urlForSendingKALivePart2);
    
    char postField1[200]=POSTFIELD_TO_SEND_KEEPALIVE_1;
    strcat(postField1,sensor_id);
    char postField2[200]=POSTFIELD_TO_SEND_KEEPALIVE_2;
    strcat(postField2,tokenForSensor);
    strcat(postField1,postField2);
    
    return curlExecutor(curl, urlForSendingKALivePart1,postField1);
}

/*METHOD: check if remote token matches, else failure*/
int checkRemoteAuth(dataSet *data) {

	//fprintf(logger,"inside checkRemoteAuth %s \n", data->eventType);
        //fflush(logger);
	//fprintf(logger,"inside checkRemoteAuth %s %s\n", data->remoteAuth, g_remoteAuth);
        //fflush(logger);



	if (strcmp(data->remoteAuth,g_remoteAuth) != 0){
		return 1;
	}else{
		return 0;
	}
}

int sendFailureStatus(dataSet *data){
        int count = 1;
	char *status="1";
	//fprintf(logger,"inside sendFailureStatus %s %s tokenforsensor = %s , status =%s \n",  data->remoteAuth, g_remoteAuth, data->tokenForSensor, status);
	//fflush(logger);
	data->curl = curl_easy_init();
        CURLcode result=callToSendStatus(data->curl,status,data->tokenForSensor);
	while(result!=CURLE_OK&&count<g_max_curl_count){
		result=callToSendStatus(data->curl,status,data->tokenForSensor);
		count++;
	}
}
int killProcess(){
	struct Metric* metrics[1];
	char timestamp[20], moniturl[200];
	char *metricsStr;

	//fprintf(logger,"INSIDE KILL PROCESS\n");
	//fflush(logger);
	if(g_aliveTime!=0 && doNotKillFlag!=1){
		if(time(NULL) > g_aliveTime + g_aliveTimeInterval){
			ppid = getppid();
			//fprintf(logger,"parent process id : %d.. ######LETS KILL!", ppid);
			//fflush(logger);
			if(kill(ppid, 9)!=0){
				//fprintf(logger,"could not kill parent process\n");
				//fflush(logger);
			}else{
 
		        struct Metric* metrics[1];              
		        char timestamp[20],moniturl[200];                     
		        char *metricsStr;                       
			int ret, ccount=0;
			sprintf(timestamp, "%u\n", (unsigned)time(NULL));    
			metrics[0] = createMetric("SERVIENT", 2, atof(timestamp));       
			json_object* metricJson = createMetricJson(metrics,1);   
			metricsStr = json_object_to_json_string(metricJson);     
			//sprintf(moniturl, "http://uk.packet.radiolocus.com/monitoring-web/v1/g/m/u/%s", sensor_id);
			ret = 	curl_post_jsonstr(g_monit_url, metricsStr);
			while(ret !=CURLE_OK && ccount < 3){
				ret = curl_post_jsonstr(g_monit_url, metricsStr);
				ccount++;
			}
			free(metrics[0]);       
			free(metricsStr);


			//fprintf(logger,"killed parent. now dying\n");
			//fflush(logger);
			exit(0);
			}
		}
		//fprintf(logger,"NOT KILLING PROCESS %u %d\n", (unsigned)time(NULL), g_aliveTime + g_aliveTimeInterval);
		//fflush(logger);
		return FALSE;
	}
	//fprintf(logger,"NOT KILLING PROCESS\n");
	//fflush(logger);
	return FALSE;
}


int checkTimeOutForExecutor(){
	if(startTime!=0){
		if(time(NULL) > startTime + g_rpc_timeout){
			return TRUE;
		}
	}
	return FALSE;
}

int executeTermination()
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	int cancelCheck =pthread_cancel(tExecutor);
	return cancelCheck;

}

int kAliveTermination()
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	int cancelCheck =pthread_cancel(tKLive);
	return cancelCheck;

}


/*METHOD:thread for sending keepAlive*/
void *keepAliveThread(void *data)
{
	//fprintf(logger,"Inside THREAD:KEEP-ALIVE\n");
        //fflush(logger);
	dataSet *dataKALive;
	dataKALive=(dataSet*)data;
	//fprintf(logger,"token in thread=%s\n",dataKALive->tokenForSensor);
        //fflush(logger);
	while(flag==FLAG_RUN_KALIVE){
		callToSendKAlive(dataKALive->curl,dataKALive->tokenForSensor);

		int retCheck=checkTimeOutForExecutor();
		if(retCheck==TRUE){
			int termCheck=executeTermination();
			if(termCheck==0)
				flag=FLAG_RPC_FAILURE;
		}
		sleep(g_keepalive_sleep);
	}
	//fprintf(logger,"token in thread=%s flag %d\n",dataKALive->tokenForSensor, flag);
        //fflush(logger);

	int count=0;
	if(flag==FLAG_RPC_SUCCESS){
		//call api to send success status;
		char *resultStatus="0";
		CURLcode result=callToSendStatus(dataKALive->curl,resultStatus,dataKALive->tokenForSensor);
		while(result!=CURLE_OK&&count<g_max_curl_count){
			callToSendKAlive(dataKALive->curl,dataKALive->tokenForSensor);
			result=callToSendStatus(dataKALive->curl,resultStatus,dataKALive->tokenForSensor);
			count++;
			sleep(g_exit_status_sleep);

		}}else if(flag==FLAG_RPC_FAILURE){
			//call api to send failure status;
			char *resultStatus="1";
			CURLcode result=callToSendStatus(dataKALive->curl,resultStatus,dataKALive->tokenForSensor);
			while(result!=CURLE_OK&&count<g_max_curl_count){
				callToSendKAlive(dataKALive->curl,dataKALive->tokenForSensor);
				result=callToSendStatus(dataKALive->curl,resultStatus,dataKALive->tokenForSensor);
				count++;
				sleep(g_exit_status_sleep);
			}
		}
}

/*METHOD:thread for calling rpc and returning back the STATUS to server*/
void *executorThread(void *data){
        dataSet *dataKALive;
        dataKALive=(dataSet*)data;
        //fprintf(logger, "%u data is %s -> inside THREAD:EXECUTOR\n",(unsigned)time(NULL), dataKALive->eventData);
        //fflush(logger);
	//call rpc
	startTime = time(NULL);
	if (strcmp(dataKALive->eventData,"debugOn") == 0){
		//fprintf(logger, "inside DEBUGON\n");
		//fflush(logger);
		doNotKillFlag = 1;	
		flag=1;
	}else if (strcmp(dataKALive->eventData,"debugOff") == 0){
		//fprintf(logger, "inside DEBUGOFF\n");
		//fflush(logger);
		doNotKillFlag = 0;
		flag=1;
	}else{
	flag=bashExecutor(dataKALive->eventData);
	}

}

/*METHOD:method for connecting sensor channel*/
CURLcode connectSensorChannel(CURL *curl){
	CURLcode res;
	char auth_basic[200];
	if(curl) {
		struct curl_slist *chunk = NULL;
		/* Remove a header curl would otherwise add by itself */ 
		chunk = curl_slist_append(chunk, "Accept:");
 
		/* Add a custom header */ 
		chunk = curl_slist_append(chunk, "Another: yes");

		/*add the auth basic */
		sprintf(auth_basic, "Authorization: Basic %s", g_sensortoken);
		chunk = curl_slist_append(chunk, auth_basic);

		/* set our custom set of headers */ 
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

                curl_easy_setopt(curl, CURLOPT_TIMEOUT, g_curl_timeout);
    //#ifdef SKIP_PEER_VERIFICATION
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    //#endif

    //#ifdef SKIP_HOSTNAME_VERIFICATION
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    //#endif


		curl_easy_setopt(curl, CURLOPT_URL, channel_url1);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		setvbuf(stdout, NULL, _IONBF, 0);
		res = curl_easy_perform(curl);
	}
	return res;

}


/*METHOD:Main */
int main(int argc,char *argv[])
{
    /*********ucitest.c********************/
    struct uci_context *ctx;
    struct uci_context *ctx1;
    struct uci_package *pkg1;
    struct uci_package *pkg2;
    struct list_head *q;
    struct list_head *r;
    struct uci_servient *rl;
    struct uci_sid *sid;
    bool set = false;
    char channelurl2[200];
    char l_monit_url[200];
    
    INIT_LIST_HEAD(&rls);
    INIT_LIST_HEAD(&sidls);
    ctx = uci_alloc_context();
    ctx1 = uci_alloc_context();
    ucimap_init(&servient_map);
    ucimap_init(&sensorid_map);
    
    uci_set_confdir(ctx, "/etc/config");
    uci_load(ctx, "servient", &pkg1);
    uci_load(ctx1, "sensorid", &pkg2);
    ucimap_parse(&servient_map, pkg1);
    ucimap_parse(&sensorid_map, pkg2);
    list_for_each(q, &rls) {
    
	rl = list_entry(q, struct uci_servient, list);
	if (rl->keepalive_url != NULL){
		sprintf(g_keepalive_url,"%s",rl->keepalive_url);
	}
	if (rl->exit_status_url != NULL){
		sprintf(g_exit_status_url,"%s",rl->exit_status_url);
	}
	if (rl->monit_url != NULL){
		sprintf(l_monit_url,"%s",rl->monit_url);
	}
	if (rl->rpc_timeout >= 0){
	        g_rpc_timeout = rl->rpc_timeout;
	}
	if (rl->curl_timeout >= 0){
	        g_curl_timeout = rl->curl_timeout;
	}
	if (rl->max_curl_count >= 0){
		g_max_curl_count = rl->max_curl_count;
	}
	if (rl->keepalive_sleep >= 0){
		g_keepalive_sleep = rl->keepalive_sleep;
	}
	if (rl->aliveTimeInterval >= 0){
		g_aliveTimeInterval = rl->aliveTimeInterval;
	}
	if (rl->exit_status_sleep >= 0){
		g_exit_status_sleep = rl->exit_status_sleep;
	}
	if (rl->channel_sleep >= 0){
		g_channel_sleep = rl->channel_sleep;
	}
	if (rl->sslselfsigned >= 0){
		g_sslselfsigned = rl->sslselfsigned;
	}
	if (rl->channel_url != NULL){
		sprintf(channelurl2,"%s",rl->channel_url);
	}

    }
    
    list_for_each(r, &sidls) {
    	sid = list_entry(r, struct uci_sid, list);
    	sprintf(sensor_id,"%s",sid->id);
	if (sid->sensortoken != NULL){
		sprintf(g_sensortoken,"%s",sid->sensortoken);
	}
	if (sid->servertoken != NULL){
		sprintf(g_remoteAuth,"%s",sid->servertoken);
	}

    	
    }

    sprintf(g_monit_url, "%s%s", l_monit_url, sensor_id);
    sprintf(channel_url1, "%s%s/ka/true", channelurl2, sensor_id);
    ucimap_cleanup(&servient_map);
    ucimap_cleanup(&sensorid_map);
    uci_free_context(ctx);
    uci_free_context(ctx1);
    g_aliveTime = time(NULL);
        struct Metric* metrics[1];              
        char timestamp[20],moniturl[200];                     
        char *metricsStr;                       
	int ret, ccount=0;
	sprintf(timestamp, "%u\n", (unsigned)time(NULL));    
	metrics[0] = createMetric("SERVIENT", 1, atof(timestamp));       
	json_object* metricJson = createMetricJson(metrics,1);   
	metricsStr = json_object_to_json_string(metricJson);     
	//sprintf(moniturl, "http://uk.packet.radiolocus.com/monitoring-web/v1/g/m/u/%s", sensor_id);
	ret = 	curl_post_jsonstr(g_monit_url, metricsStr);
	while(ret !=CURLE_OK && ccount < 3){
		ret = curl_post_jsonstr(g_monit_url, metricsStr);
		ccount++;
	}
	free(metrics[0]);       
	free(metricsStr);
    //sentMonitReq("SERVIENT", 1);

        int connToExec[2];
	if (argc >1){
		if (strcmp("test",argv[1]) == 0){
			CURL *curl = curl_easy_init();
                        CURLcode res=connectSensorChannel(curl);
                        if(res != CURLE_OK){
                                fprintf(stderr, "curl_easy_perform() failed: %s\n",
                                                curl_easy_strerror(res));

        			fflush(logger);
				exit(1);
                        }

                        curl_easy_cleanup(curl);
			exit(0);
		}

	}
	pipe(connToExec);

	if (fork() > 0)
	{
		close(connToExec[0]);
		dup2(connToExec[1], STDOUT_FILENO);
		close(connToExec[1]);
                logger = fopen("/tmp/log/servient.txt", "w+");
    		//fprintf(logger, "**************This is testing for fprintf...\n");
		//fflush(logger);
		while(TRUE){
			CURL *curl = curl_easy_init();
			CURLcode res=connectSensorChannel(curl);
			//fprintf(logger, "new try\n");
			//fflush(logger);

			if(res != CURLE_OK){
				fprintf(stderr, "curl_easy_perform() failed: %s\n",
						curl_easy_strerror(res));
        			fflush(logger);
			}

			curl_easy_cleanup(curl);
			//fprintf(logger, "g_channel_sleep : %d\n",g_channel_sleep);
			//fflush(logger);
			sleep(g_channel_sleep);
		}
		fclose(logger);
	}
	else{
		char buffer[200],c;
		int i=0, buffer_complete_flag=0;
		ssize_t nbytes;
		close(connToExec[1]);
		dup2(connToExec[0], STDIN_FILENO);
		close(connToExec[0]);
                logger = fopen("/tmp/log/servient.txt", "w+");
                while((nbytes = read(STDIN_FILENO, &c, 1))>0){
			if (buffer_complete_flag == 1){
				i = 0;
				buffer_complete_flag = 0;
			}
		buffer[i++]=c;
		//fprintf(logger, "%c \n",c);
		//fflush(logger);
                if (c == '\n'){
				buffer[i]='\0';
				buffer_complete_flag = 1;
			//fflush(logger);
			//fprintf(logger,"TRYING TO KILL PROCESS\n");
			//fflush(logger);
			killProcess();
			dataSet *data;
			data=(dataSet *)malloc(sizeof(dataSet));
			flag=FLAG_RUN_KALIVE;

			int validate=INVALID_DATA;
			//fprintf(logger, "%u buffer is %s inside EXECUTOR going to validate %d\n",(unsigned)time(NULL), buffer, sizeof(buffer));
			//fflush(logger);
			validate=validateData(buffer,&data);
			//fprintf(logger, "finished with validation %d\n", validate);
			//fflush(logger);
			//check if remoteAuth is valid, else send failure
			if (validate==VALID_DATA){
				//fprintf(logger, "VALID!\n");
				//fflush(logger);
			if (strcmp(data->eventType,"COMMAND")==0){
				//fprintf(logger, "INSIDE COMMAND!\n");
				//fflush(logger);
			if (checkRemoteAuth(data)){
				//send failure
				sendFailureStatus(data);
				//fprintf(logger, "invalid remote token. Sennding error %s\n", buffer);
        			//fflush(logger);
			}else{
			//fprintf(logger, "%u buffer is %s inside EXECUTOR, validate %d\n",(unsigned)time(NULL), buffer, validate);
			//fflush(logger);

			if(validate==VALID_DATA){
                               //fprintf(logger, "%u inside validate \n",(unsigned)time(NULL));
                               //fflush(logger);
				g_aliveTime = time(NULL);
				data->curl = curl_easy_init();
                               pthread_create(&tKLive, NULL, keepAliveThread,(void*) data);
                               pthread_create(&tExecutor, NULL, executorThread,(void*) data);
                               void* ret;
                               pthread_join(tKLive, &ret);
                               pthread_join(tExecutor, &ret);
				curl_easy_cleanup(data->curl);
			}else{
				//fprintf(logger, "invalid data sorry \n");
				//fflush(logger);
			}
			}}}else{//fprintf(logger, "INVALID!\n");fflush(logger);
			}
		}} fclose(logger);}
		return 0;
}
