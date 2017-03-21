#include "../lib/prober.h"
#define PC 1

void dbg(char *fmt, ...)
{
        char buf[LOGBUFFER_SIZE] = {'\0'};
        va_list msg;

        va_start(msg,fmt);
        vsnprintf(buf, sizeof(buf), fmt, msg);
        va_end(msg);
        buf[sizeof(buf) - 1] = '\0';
        printf("\n\n---------------------------------------\n");
        printf("%s",buf);
        printf("\n---------------------------------------\n\n");

}

uint16_t radiotap_get(struct pkg_util_info *rd,const u_char *packet,uint32_t len)
{
        struct ieee80211_radiotap_header *radiotap;
        struct ieee80211_radiotap_iterator iterator;
        //double a;
        radiotap=(struct ieee80211_radiotap_header *)packet;
        radiotap->it_len = cpu_to_le16(radiotap->it_len);
        radiotap->it_present = cpu_to_le32(radiotap->it_present);
        //dbg("header size:%ld",radiotap->it_len);
        //dbg("it_version:%ld", radiotap->it_version);
        //dbg("it_pad:%ld", radiotap->it_pad);
        //dbg("it_present:%p", radiotap->it_present);

        if (radiotap->it_version>PKTHDR_RADIOTAP_VERSION){
                log_err(LEVEL7,(char *)"Unsuported version of radiotap");
                return 0;
        }

        if (radiotap->it_len<8 || radiotap->it_len>len){
                log_err(LEVEL7,(char *)"Invalid packet length");
                return 0;
        }

        ieee80211_radiotap_iterator_init(&iterator,radiotap,radiotap->it_len);
        while (ieee80211_radiotap_iterator_next(&iterator)>=0){

//!TODO we should check also  IEEE80211_RADIOTAP_DB_ANTSIGNAL IEEE80211_RADIOTAP_DB_ANTNOISE
                switch (iterator.this_arg_index){
                        //printf("%ld",iterator.this_arg_index);
                        case IEEE80211_RADIOTAP_CHANNEL:
                                rd->channel=(uint8_t)ieee80211mhz2chan((uint32_t)((iterator.this_arg[1])*256+(iterator.this_arg[0]))) | 0x80;
                                //rd->channel=0;
                                break;
                        case IEEE80211_RADIOTAP_DBM_ANTSIGNAL:
                                //printf("**%p* %p*\n",iterator.this_arg, *iterator.this_arg);
                                rd->signal=(uint8_t)*iterator.this_arg-256;
                                break;
                        case IEEE80211_RADIOTAP_DBM_ANTNOISE:
                                rd->noise=*iterator.this_arg-256;
                                break;
                }
        }
        return radiotap->it_len;
}




//global variables
//with all settings
//void dbg_mac(const char *name,u8 mac[6])
//{
//        //printf("%s %02x:%02x:%02x:%02x:%02x:%02x\n",name,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
//}


//print log priority:LOG_ERR (system)


/*set hardware address in rd structure

INPUT:
        u8 *src_addr  -  Source address 
        u8 *dst_addr  -  Destination address
        u8 *bssid_addr-  Address of the bssid
OUTPUT: 
        struct pkg_util_info rd - informations parsed from packet

RETURN: void
*/
void rd_addr_set(struct pkg_util_info *rd,u8 *src_addr, u8 *dst_addr, u8 *bssid_addr)
{

        if (src_addr!=NULL){
                memcpy(rd->sa,src_addr,sizeof(u8)*sizeof(rd->sa));
        }

        if (dst_addr!=NULL){
                memcpy(rd->da,dst_addr,sizeof(u8)*sizeof(rd->da));
        }

        if (bssid_addr!=NULL){
                memcpy(rd->bssid,bssid_addr,sizeof(u8)*sizeof(rd->bssid));
        }
}

void management_element_init(struct information_element *inf_element,const u_char *frame_body,unsigned int frame_length)
{

        inf_element->frame_body=frame_body;
        inf_element->frame_length=frame_length;
        inf_element->crt_index=0;
        inf_element->tag_number=-1;
        inf_element->tag_len=0;
        inf_element->tag_info=NULL;


}

/* 
ATTENTION:
        inf_element must be first initialised using management_element_init(...)
INPUT-OUTPUT: 
        struct information_element *inf_element -  srtucture which contains information parsed from management frames 
RETURN:
        on success: 0 if all elements were parsed
                    tag length if there are other elements to parse
        on error  : -1
*/
int management_element_parse(struct information_element *inf_element)
{

        if (inf_element->frame_length==0){
                //log_err(LEVEL7,(char *)"Malformated packet: Zero length!");
                return -1;
        }

        if (inf_element->frame_body==NULL){
                //log_err(LEVEL7,(char *)"Malformated packet: NULL information element!");
                return -1;
        }

        //we parsed all data in the packet
        if (inf_element->crt_index>=inf_element->frame_length){
                return 0;
        }

        /*!!!!!!!!!!!!!!!!!!!!XXXXX error on 64 bits*/
        //!BUG 9 PROBLEM IN 64 bits architectures
        if (sizeof(inf_element->tag_number) + sizeof(inf_element->tag_len) + inf_element->crt_index >= inf_element->frame_length ){
                //log_err(LEVEL8,(char *)"Malformated packet[element]: Invalid length of the packet");
                return -1;
        }

        inf_element->tag_number=(u8)*inf_element->frame_body;
        inf_element->tag_len=(u8) *(inf_element->frame_body+sizeof(inf_element->tag_number));

        /*!!!!!!!!!!!!!!!!!!!!XXXXX error on 64 bits*/
        //!BUG 9 PROBLEM IN 64 bits architectures
        if (inf_element->crt_index + sizeof(inf_element->tag_number) + sizeof(inf_element->tag_len) + inf_element->tag_len > inf_element->frame_length){
                //log_err(LEVEL8,(char *)"Malformated packet[element]: Invalid length of the packet (2)");
                return -1;
        }


        inf_element->tag_info=(u8 *)(inf_element->frame_body+sizeof(inf_element->tag_number) + sizeof(inf_element->tag_len));
        inf_element->frame_body=inf_element->frame_body+sizeof(inf_element->tag_number) + sizeof(inf_element->tag_len)+inf_element->tag_len;
        inf_element->crt_index+=sizeof(inf_element->tag_number) + sizeof(inf_element->tag_len) + inf_element->tag_len;

        return inf_element->tag_len;
}

void log_err(int debug_level,char *fmt, ...)
{
        char buf[LOGBUFFER_SIZE] = {'\0'};
        va_list msg;
        va_start(msg,fmt);
        vsnprintf(buf, sizeof(buf), fmt, msg);
        va_end(msg);
        buf[sizeof(buf) - 1] = '\0';
//      log_write(debug_level, LOG_ERR, buf);

        //printf("%s\n",buf);

}


int do_frame_management(struct pkg_util_info *rd, struct ieee80211_frame *f80211, const u_char *management_packet,unsigned int pkg_length)
{
        int fc_len;

        fc_len=-1;
        switch (rd->type_subtype){
                case IEEE80211_FRAME_MANAGEMENT_PROBE_REQ:
                        fc_len=IEEE80211_FRAME_MANAGEMENT_H_SIZE;
                        rd_addr_set(rd,f80211->addr2,f80211->addr1,f80211->addr3);
                        if (IEEE80211_FRAME_MANAGEMENT_H_SIZE>pkg_length){
                                //log_err(LEVEL5,(char *)"Malformated packet: invalid length of management packet");
                                return -1;
                        }
                        do_frame_management_probe(rd,f80211,(management_packet+IEEE80211_FRAME_MANAGEMENT_H_SIZE),pkg_length-IEEE80211_FRAME_MANAGEMENT_H_SIZE);
                        break;
                default:
                        //log_err(LEVEL4,(char *)"Invalid subtype for DATA MANAGEMENT or NOT A PROBE REQ");
                        return -1;
        }

        return fc_len;
}


void do_frame_management_probe(struct pkg_util_info *rd, struct ieee80211_frame *f80211, const u_char *probe_packet,unsigned int probe_length)
{
        struct information_element inf_element;
        uint16_t capability;


        management_element_init(&inf_element,probe_packet,probe_length);

        while(management_element_parse(&inf_element)>0){


                //ELEMENT ID
                switch (inf_element.tag_number){

                        case IEEE802_INF_ELEMENT_SSID:
                                if (inf_element.tag_len<=IEEE802_INF_ELEMENT_SSID_MAX_LEN){
                                        if (inf_element.tag_len>0){
                                                strncpy(rd->ssid,(char *)inf_element.tag_info,inf_element.tag_len);
                                                rd->ssid[inf_element.tag_len]='\0';
                                        }else{
                                                rd->ssid[0]='\0';
                                        }
                                }else{
                                        //log_err(LEVEL8,(char *)"Malformated pkg. Invalid ssid");
                                        rd->ssid[0]='\0';
                                }
                                break;

                }
        }
}




//init for util informations
void rd_init(struct pkg_util_info *rd)
{
        
//      rd->enc=0;
        rd->max_rate=0;
        rd->channel=0;
        rd->signal=-1;
        rd->noise=-1;   
        rd->protocol=-1;
        rd->type=-1;
        rd->subtype=-1;
        rd->type_subtype=-1;
        rd->flag_ds=-1;
        rd->is_protected=-1;
        memset(rd->da,0xff,sizeof(rd->da));
        memset(rd->sa,0xff,sizeof(rd->sa));
        memset(rd->bssid,0xff,sizeof(rd->bssid));
        rd->ssid[0]='\0';
        rd->seen=0;
        rd->no=0;
        rd->cipher=CIPHER_NULL;
        rd->auth=AUTH_NULL;


        rd->wpa=NULL;
}


void rd_destroy(struct pkg_util_info *rd)
{
        if (rd->wpa!=NULL){
                free(rd->wpa);
                rd->wpa=NULL;
        }
}


void got_pkg(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{
        
        //Queue *queue = (Queue *) args;
        //do_pkg(header,packet,queue, 1);
        
//}
//void do_pkg(const struct pcap_pkthdr *header, const u_char *packet,int radiotap_exist)
//{

        struct pkg_util_info rd;
        struct ieee80211_frame *f80211;
        struct tm *time_struct;
        uint16_t radiotap_len;
        uint64_t millis;
        float nanos;
        char filter_pkg;
        char arrival_time[80];
        int fc_len;
        int frame_body,i;
        int radiotap_exist = 1;
        char tuple[1000];
        char *ssid = NULL;
        char *substr = ",";
        char *replacestr = "_";
        char *sensor_id;



        const uint8_t invalid_bssid0[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

        Configuration* conf = (Configuration * ) args;
	int bad_packets_timer_limit = conf->bad_packets_time;
	int bad_packets_timer = conf->bad_packets_timer;
	int bad_packets_counter_limit = conf->bad_packets;
	//int bad_packets = conf->bad_packets_count;
	int dbmsignal_limit = conf->dbmsignal_limit;
        int sensor_tupleversion = conf->sensor_tupleversion;
        int tuplecounter = conf->tuplecounter;
        int sensor_customflag = conf->custom_flag;
        struct Queue *queue = conf->queue;
        sensor_id = conf->sensor_id;
        
        //printf ("test: %d\n", conf->sensor_tupleversion);
        //printf("in do pkg \n");

        if (header->len<8){
                log_err(LEVEL5,(char *)"Invalid header len");
                return;
        }
        //dbg_hex("pachet",packet,header->len); 
        rd_init(&rd);
        rd.seen=header->ts.tv_sec;
        time ( &rd.seen );

        if (radiotap_exist){
                radiotap_len=radiotap_get(&rd,packet,header->len);
                if (radiotap_len==0){
                        log_err(LEVEL7,(char *)"Invalid radiotap header");
                        return;
                }
        }else{
                radiotap_len=0;
        }




        f80211=(struct ieee80211_frame *)(packet+radiotap_len);


        rd.protocol=f80211->frame_type&0x03;
        rd.type=(f80211->frame_type&0x0C)>>2;
        rd.subtype=(f80211->frame_type&0xF0)>>4;
        rd.type_subtype=(rd.type<<4)|rd.subtype;
        rd.flag_ds=f80211->flags&0x03;
        rd.is_protected=f80211->flags&0x40>>6;




        if (rd.protocol>0){
                log_err(LEVEL7,(char *)"Unsuported protocol!");
                return;
        }
        if (rd.type>2){
                log_err(LEVEL7,(char *)"Unsuported type");
                return;
        }


        fc_len=-1;
        frame_body=-1;

        //parse header for specific type of frame
        switch (rd.type){
                case IEEE80211_FRAME_MANAGEMENT:
                        fc_len=do_frame_management(&rd,f80211,(packet+radiotap_len),header->len-radiotap_len);
                        rd.no=rd.no | NO_MANAGEMENT;
                        break;
                default:
                        log_err(LEVEL4,(char *)"Invalid frame type");
                        return;
        }
        if (rd.signal < -1){
        ssid = str_replace(rd.ssid, substr , replacestr);
	if (rd.signal > dbmsignal_limit){
        	if (sensor_tupleversion == 1){
        	        sprintf(tuple,"%d,%s,1,1,%ld.%.6ld,%02x:%02x:%02x:%02x:%02x:%02x,%d,%s,%d\n",sensor_tupleversion, sensor_id,header->ts.tv_sec, header->ts.tv_usec, \
        	                                        rd.sa[0],rd.sa[1],rd.sa[2],rd.sa[3],rd.sa[4],rd.sa[5],rd.signal,ssid,sensor_customflag);
        	}
        	else if (sensor_tupleversion == 2){
        	        sprintf(tuple,"%d,%s,1,1,%ld.%.6ld,%02x:%02x:%02x:%02x:%02x:%02x,%d,%s,%d\n",sensor_tupleversion, sensor_id,header->ts.tv_sec, header->ts.tv_usec, \
        	                                        rd.sa[0],rd.sa[1],rd.sa[2],rd.sa[3],rd.sa[4],rd.sa[5],rd.signal,ssid,sensor_customflag);
        	}
        	else if (sensor_tupleversion == 3){
        	        conf->tuplecounter++;
        	        sprintf(tuple,"%d,%s,%d,1,%ld.%.6ld,%02x:%02x:%02x:%02x:%02x:%02x,%d,%s,%d\n",sensor_tupleversion, sensor_id, tuplecounter,header->ts.tv_sec, header->ts.tv_usec, \
        	                                        rd.sa[0],rd.sa[1],rd.sa[2],rd.sa[3],rd.sa[4],rd.sa[5],rd.signal,ssid,sensor_customflag);
        	}
        	else {
        	        printf("Unsupported tupleversion");
        	        exit(1);
        	}
		enqueue(queue, tuple, 0);
		free(ssid);
		rd_destroy(&rd);
        }else{
                conf->bad_packets_count++;
                int time_diff = (int)time(NULL) - bad_packets_timer;
                if(time_diff >= bad_packets_timer_limit){
                        bad_packets_timer = (int)time(NULL);
                        conf->bad_packets_count = 0;
                }
                else if(conf->bad_packets_count > bad_packets_counter_limit && time_diff < bad_packets_timer_limit){
			printf("rebooting...\n");
                        system("reboot");
                }
        }
       }
}


char * generate_tuple(char * device_name, struct Queue* queue, int max_packets, int sensor_tupleversion, int custom_flag, int tuplecounter, char *sensor_id, int bad_packets_time, int bad_packets, int dbmsignal_limit)
{
        char errbuf[PCAP_ERRBUF_SIZE];          /* error buffer */
        pcap_t *handle;                         /* packet capture handle */
        bpf_u_int32 mask;                       /* subnet mask */
        bpf_u_int32 net;                        /* ip */
        char filter_exp[] = "type mgt and subtype probe-req";           /* filter expression [3] */
        struct bpf_program fp;                  /* compiled filter program (expression) */
        int num_packets = max_packets;                   /* number of packets to capture */

        /* def. of the structure: */
        //Configuration *conf;
        //conf = malloc(sizeof(Configuration));
        //conf->queue = queue;
        //conf->sensor_tupleversion = sensor_tupleversion;
        //conf->tuplecounter = tuplecounter;
        //conf->sensor_id = sensor_id;
        //conf->custom_flag = custom_flag;
        Configuration conf;
        conf.queue = queue;
        conf.sensor_tupleversion = sensor_tupleversion;
        conf.tuplecounter = tuplecounter;
        conf.custom_flag = custom_flag;
        conf.sensor_id = sensor_id;
	conf.bad_packets_timer = (int)time(NULL);
	conf.bad_packets_time = bad_packets_time;
	conf.bad_packets = bad_packets;
	conf.bad_packets_count = 0;
	conf.dbmsignal_limit = dbmsignal_limit;
#ifdef PC
        while(1){
        sleep(1);
        enqueue(queue, "3,124c1c76-412a-4a7e-96ff-9476130c2e0c,69905,1,1448186403,4c:bb:58:33:2c:d1,-86.0,Inorbit,1", 0);
        }
#else
        /* get network number and mask associated with capture device */
        if (pcap_lookupnet(device_name, &net, &mask, errbuf) == -1) {
                fprintf(stderr, "Couldn't get netmask for device %s: %s\n",
                    device_name, errbuf);
                net = 0;
                mask = 0;
        }

        /* open capture device */
        handle = pcap_open_live(device_name, SNAP_LEN, 1, 1000, errbuf);
        if (handle == NULL) {
                fprintf(stderr, "Couldn't open device %s: %s\n", device_name, errbuf);
                exit(EXIT_FAILURE);
        }

        /* compile the filter expression */
        if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) {
                fprintf(stderr, "Couldn't parse filter %s: %s\n",
                    filter_exp, pcap_geterr(handle));
                exit(EXIT_FAILURE);
        }

        /* apply the compiled filter */
        if (pcap_setfilter(handle, &fp) == -1) {
                fprintf(stderr, "Couldn't install filter %s: %s\n",
                    filter_exp, pcap_geterr(handle));
                exit(EXIT_FAILURE);
        }

        /* now we can set our callback function */
        //printf("at loop");
        pcap_loop(handle, num_packets, got_pkg,(u_char*) &conf);

        /* cleanup */
        pcap_freecode(&fp);
        pcap_close(handle);
#endif
}



