#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count, char *auth)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // Step 2: add the host
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Step 3 (optional): add headers and/or cookies, according to the protocol format
    if (cookies != NULL) {
       memset(line, 0, LINELEN);
       strcat(line, "Cookie: ");

       for(int i = 0; i < cookies_count - 1; i++) {
        strcat(line, cookies[i]);
        strcat(line, "; ");
       }

       strcat(line, cookies[cookies_count - 1]);
       
       compute_message(message, line);
    }

    if (auth) {
        memset(line, 0, LINELEN);
        sprintf(line, "Authorization: Bearer %s", auth);
        compute_message(message, line);
    }

    // Step 4: add final new line
    compute_message(message, "");
    return message;
}

char *compute_post_request(char *host, char *url, char* content_type, char **body_data,
                            int body_data_fields_count, char **cookies, int cookies_count, char *auth)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *body_data_buffer = calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL and protocol type
    memset(line, 0, LINELEN);
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    // Step 2: add the host
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if (auth) {
        memset(line, 0, LINELEN);
        sprintf(line, "Authorization: Bearer %s", auth);
        compute_message(message, line);
    }

    /* Step 3: add necessary headers (Content-Type and Content-Length are mandatory)
            in order to write Content-Length you must first compute the message size
    */
    memset(line, 0, LINELEN);
    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    memset(line, 0, LINELEN);
    for(int i = 0; i < body_data_fields_count - 1; i++) {
        strcat(body_data_buffer, body_data[i]);
        strcat(body_data_buffer, "&");
    }

    strcat(body_data_buffer, body_data[body_data_fields_count - 1]);

    memset(line, 0, LINELEN);
    sprintf(line, "Content-Length: %lu", strlen(body_data_buffer));
    compute_message(message, line);

    // Step 4 (optional): add cookies
    if (cookies != NULL) {
       memset(line, 0, LINELEN);
       strcat(line, "Cookie: ");

       for(int i = 0; i < cookies_count - 1; i++) {
        strcat(line, cookies[i]);
        strcat(line, "; ");
       }

       strcat(line, cookies[cookies_count - 1]);
       
       compute_message(message, line);
    }
    // Step 5: add new line at end of header
    compute_message(message, "");

    // Step 6: add the actual payload data
    memset(line, 0, LINELEN);

    strcat(message, body_data_buffer);

    free(line);
    return message;
}

char * compute_delete_request(char *host, char *url, char **cookies, int cookies_count, char *auth) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    sprintf(line, "DELETE %s HTTP/1.1", url);
    compute_message(message, line);

    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if (cookies != NULL) {
       memset(line, 0, LINELEN);
       strcat(line, "Cookie: ");

       for(int i = 0; i < cookies_count - 1; i++) {
        strcat(line, cookies[i]);
        strcat(line, "; ");
       }

       strcat(line, cookies[cookies_count - 1]);
       
       compute_message(message, line);
    }

    if(auth) {
        memset(line, 0, LINELEN);
        sprintf(line, "Authorization: Bearer %s", auth);
        compute_message(message, line);
    }

    compute_message(message, "");

    free(line);
    return message;
}

char *compute_put_request(char *host, char *url, char* content_type, char **body_data,
                            int body_data_fields_count, char **cookies, int cookies_count, char *auth)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *body_data_buffer = calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL and protocol type
    memset(line, 0, LINELEN);
    sprintf(line, "PUT %s HTTP/1.1", url);
    compute_message(message, line);
    
    // Step 2: add the host
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if (auth) {
        memset(line, 0, LINELEN);
        sprintf(line, "Authorization: Bearer %s", auth);
        compute_message(message, line);
    }

    /* Step 3: add necessary headers (Content-Type and Content-Length are mandatory)
            in order to write Content-Length you must first compute the message size
    */
    memset(line, 0, LINELEN);
    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    memset(line, 0, LINELEN);
    for(int i = 0; i < body_data_fields_count - 1; i++) {
        strcat(body_data_buffer, body_data[i]);
        strcat(body_data_buffer, "&");
    }

    strcat(body_data_buffer, body_data[body_data_fields_count - 1]);

    memset(line, 0, LINELEN);
    sprintf(line, "Content-Length: %lu", strlen(body_data_buffer));
    compute_message(message, line);

    // Step 4 (optional): add cookies
    if (cookies != NULL) {
       memset(line, 0, LINELEN);
       strcat(line, "Cookie: ");

       for(int i = 0; i < cookies_count - 1; i++) {
        strcat(line, cookies[i]);
        strcat(line, "; ");
       }

       strcat(line, cookies[cookies_count - 1]);
       
       compute_message(message, line);
    }
    // Step 5: add new line at end of header
    compute_message(message, "");

    // Step 6: add the actual payload data
    memset(line, 0, LINELEN);

    strcat(message, body_data_buffer);

    free(line);
    return message;
}
