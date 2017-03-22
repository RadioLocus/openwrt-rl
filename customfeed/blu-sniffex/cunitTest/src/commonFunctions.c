#include "../lib/commonFunctions.h"

int add(int a, int b){
 return (a+b);
}

char* allocateString(char *string){

    char *newAlloc = (char *)malloc(strlen(string) + 1);
    strcpy(newAlloc, string);
    return newAlloc;

}

char* combineString(char *old, char* new){
    size_t old_len = 0, new_len = 0;
    if (old != NULL){
        old_len = strlen(old);
    }else{
        if (new != NULL) return strdup(new); else return NULL;
    }
    if (new != NULL){
    new_len = strlen(new);
    }else{
        if (old != NULL) return strdup(old); else return NULL;
    }
    const size_t out_len = old_len + new_len + 1;
    // allocate a pointer to the new string
    char *out = malloc(out_len);

    // concat both strings and return
    memcpy(out, old, old_len);
    memcpy(out + old_len, new, new_len+1);
    return out;
}


//replace substring
char *
str_replace ( const char *string, const char *substr, const char *replacement ){
  char *tok = NULL;
  char *newstr = NULL;
  char *oldstr = NULL;
  char *head = NULL;
 
  if (string == NULL) return NULL;
  /* if either substr or replacement is NULL, duplicate string a let caller handle it */
  if ( substr == NULL || replacement == NULL ) return strdup (string);
  newstr = strdup (string);
  head = newstr;
  while ( (tok = strstr ( head, substr ))){
    oldstr = newstr;
    newstr = malloc ( strlen ( oldstr ) - strlen ( substr ) + strlen ( replacement ) + 1 );
    /*failed to alloc mem, free old string and return NULL */
    if ( newstr == NULL ){
      free (oldstr);
      return NULL;
    }
    memcpy ( newstr, oldstr, tok - oldstr );
    memcpy ( newstr + (tok - oldstr), replacement, strlen ( replacement ) );
    memcpy ( newstr + (tok - oldstr) + strlen( replacement ), tok + strlen ( substr ), strlen ( oldstr ) - strlen ( substr ) - ( tok - oldstr ) );
    memset ( newstr + strlen ( oldstr ) - strlen ( substr ) + strlen ( replacement ) , 0, 1 );
    /* move back head right after the last replacement */
    head = newstr + (tok - oldstr) + strlen( replacement );
    free (oldstr);
  }
  return newstr;
}


char * strautocat(char **buffer, const char *str1)  //char **buffer, const char *format, vargs? Ive done this before
{
    //assert(str1 != NULL); assert(buffer != NULL);
    if (str1 == NULL) return NULL;

    if(*buffer == NULL){*buffer = (char *)calloc(sizeof(char)*1,sizeof(char));}

    size_t required_size;
    //    allocated_size = malloc_usable_size(*buffer);
    required_size  = strlen(str1)+strlen(*buffer)+1;

    //while(required_size > allocated_size)
    //{
    //    printf("Growing buffer from %d to accomidate %d\n", allocated_size, required_size);
        *buffer = (char *)realloc(*buffer, required_size);
        //allocated_size = malloc_usable_size(*buffer);
    //}
    strcat(*buffer, str1);
    return *buffer;
}



/*//append 2 stings
char *str_append(char *s1, char *s2)
{
    char *str ;
    if (s2 == NULL) return s1 == NULL ? NULL: strdup(s1);
    if (s1 == NULL){
    str = strdup(s2);
    return str;   

    }
    
    else {
    str = malloc(strlen(s1) + strlen(s2) + 1 * sizeof(*s1));
    if (str == NULL)
        return NULL;
    str[0] = '\0';
     
    strcat(str, s1);
    strcat(str, s2);
    return str;   
    }
}*/
//void str_append(char *s1, char *s2)
//{
//    char *str ;
//    if (s2 == NULL) return;
//    if (s1 == NULL){
//    s1 = strdup(s2);
//    return;   
//
//    }
//    
//    else {
//    s1 = (char *)realloc(s1, strlen(s1) + strlen(s2) + 1 * sizeof(*s1));
//    if (s1 == NULL)
//        return ;
//     
//    strcat(s1, s2);
//    return;   
//    }
//}
