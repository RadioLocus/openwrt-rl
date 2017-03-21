#include "../lib/curler.h"

#define MAX_ATTEMPTS 3
/*****************************/
// for self auth SSL 
#define SKIP_PEER_VERIFICATION 0
#define SKIP_HOSTNAME_VERIFICATION 0

/*****************************/



/* callback for curl fetch */
size_t curl_callback (void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;                             /* calculate buffer size */
    struct curl_fetch_st *p = (struct curl_fetch_st *) userp;   /* cast pointer to fetch struct */
    p->payload = (char *) realloc(p->payload, p->size + realsize + 1);
    if (p->payload == NULL) {
      fprintf(stderr, "ERROR: Failed to expand buffer in curl_callback");
      free(p->payload);
      return -1;
    }
    memcpy(&(p->payload[p->size]), contents, realsize);
    p->size += realsize;
    p->payload[p->size] = 0;
    return realsize;
}

/* fetch and return url body via curl */
CURLcode curl_fetch_url(CURL *ch, const char *url, struct curl_fetch_st *fetch) {
    CURLcode rcode;                   /* curl result code */
    int i;
    fetch->payload = (char *) calloc(1, sizeof(fetch->payload));
    if (fetch->payload == NULL) {
        fprintf(stderr, "ERROR: Failed to allocate payload in curl_fetch_url");
        return CURLE_FAILED_INIT;
    }
    fetch->size = 0;
    curl_easy_setopt(ch, CURLOPT_URL, url);
    curl_easy_setopt(ch, CURLOPT_WRITEFUNCTION, curl_callback);
    curl_easy_setopt(ch, CURLOPT_WRITEDATA, (void *) fetch);
    curl_easy_setopt(ch, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    curl_easy_setopt(ch, CURLOPT_TIMEOUT, 10);
    curl_easy_setopt(ch, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(ch, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(ch, CURLOPT_MAXREDIRS, 1);
    for (i = 0; i < MAX_ATTEMPTS; i++) {
        printf("try %d", i);
        if ((rcode = curl_easy_perform(ch)) == CURLE_OK){
            printf("rcode success %d\n", rcode);
            fflush(stdout);
            break;
        }
        printf("rcode %d", rcode);
    }
    return rcode;
}

/* fetch and return url body via curl */
CURLcode curl_post_jsonstr(char *postURL, char *postString)
{
    CURL *ch;                                               /* curl handle */
    CURLcode rcode;                                         /* curl result code */
    struct curl_fetch_st curl_fetch;                        /* curl fetch struct */
    struct curl_fetch_st *cf = &curl_fetch;                 /* pointer to fetch struct */
    struct curl_slist *headers = NULL;                      /* http headers to send with request */
    if ((ch = curl_easy_init()) == NULL) {
        fprintf(stderr, "ERROR: Failed to create curl handle in fetch_session");
        return 1;
    }
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");

    #ifdef SKIP_PEER_VERIFICATION
        curl_easy_setopt(ch, CURLOPT_SSL_VERIFYPEER, 0L);
    #endif

    #ifdef SKIP_HOSTNAME_VERIFICATION
        curl_easy_setopt(ch, CURLOPT_SSL_VERIFYHOST, 0L);
    #endif

    curl_easy_setopt(ch, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(ch, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(ch, CURLOPT_POSTFIELDS, postString);
    rcode = curl_fetch_url(ch, postURL, cf);
    curl_easy_cleanup(ch);
    curl_slist_free_all(headers);
    if (rcode != CURLE_OK || cf->size < 1) {
        fprintf(stderr, "ERROR: Failed to fetch url (%s) - curl said: %s",
            postURL, curl_easy_strerror(rcode));
        return 2;
    }
    if (cf->payload != NULL) {
        free(cf->payload);
        return 0;
    } else {
        fprintf(stderr, "ERROR: Failed to populate payload");
        free(cf->payload);
        return 3;
    }

}


/* fetch and return url body via curl */
CURLcode curl_post_str(char *postURL, char *postString)
{
    CURL *ch;                                               /* curl handle */
    CURLcode rcode;                                         /* curl result code */
    struct curl_fetch_st curl_fetch;                        /* curl fetch struct */
    struct curl_fetch_st *cf = &curl_fetch;                 /* pointer to fetch struct */
    struct curl_slist *headers = NULL;                      /* http headers to send with request */
    if ((ch = curl_easy_init()) == NULL) {
        fprintf(stderr, "ERROR: Failed to create curl handle in fetch_session");
        return 1;
    }
    headers = curl_slist_append(headers, "Accept: text/plain");
    headers = curl_slist_append(headers, "Content-Type: text/plain");

    #ifdef SKIP_PEER_VERIFICATION
        curl_easy_setopt(ch, CURLOPT_SSL_VERIFYPEER, 0L);
    #endif

    #ifdef SKIP_HOSTNAME_VERIFICATION
        curl_easy_setopt(ch, CURLOPT_SSL_VERIFYHOST, 0L);
    #endif

    curl_easy_setopt(ch, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(ch, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(ch, CURLOPT_POSTFIELDS, postString);
    rcode = curl_fetch_url(ch, postURL, cf);
    curl_easy_cleanup(ch);
    curl_slist_free_all(headers);
    if (rcode != CURLE_OK) {
        fprintf(stderr, "ERROR: Failed to fetch url (%s) - curl said: %s",
            postURL, curl_easy_strerror(rcode));
        free(cf->payload);
        return 2;
    }
    else {
        free(cf->payload);
        return 0;
    }

}
