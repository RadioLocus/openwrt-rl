#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
/* holder for curl fetch */
struct curl_fetch_st {
    char *payload;
    size_t size;
};


CURLcode curl_post_jsonstr(char *, char *);
CURLcode curl_post_str(char *, char *);
