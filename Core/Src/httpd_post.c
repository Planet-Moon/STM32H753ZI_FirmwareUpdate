/*
 * httpd_post.c
 *
 *  Created on: Jun 27, 2024
 *      Author: schroeder
 */


#include "httpd.h"
#include "string.h"
#include "mem.h"

#include "../ota/ota.h"
#include "../ota/ota_utils.h"
#include "../udp/udp.h"

typedef enum  {
    PostAddressee_None,
    PostAddressee_OTA_Begin,
    PostAddressee_OTA_Write,
    PostAddressee_OTA_End
} PostAddressee;

PostAddressee postAddressee = PostAddressee_None;
#define FILENAME_LENGTH 20
char filename[FILENAME_LENGTH];

static uint16_t s_chunks = 0;
static uint32_t s_contentLength = 0;
static uint16_t s_chunkIdx = 0;
static uint32_t s_chunkSize = 0;

/**
 * @ingroup httpd
 * Called when a POST request has been received. The application can decide
 * whether to accept it or not.
 *
 * @param connection Unique connection identifier, valid until httpd_post_end
 *        is called.
 * @param uri The HTTP header URI receiving the POST request.
 * @param http_request The raw HTTP request (the first packet, normally).
 * @param http_request_len Size of 'http_request'.
 * @param content_len Content-Length from HTTP header.
 * @param response_uri Filename of response file, to be filled when denying the
 *        request
 * @param response_uri_len Size of the 'response_uri' buffer.
 * @param post_auto_wnd Set this to 0 to let the callback code handle window
 *        updates by calling 'httpd_post_data_recved' (to throttle rx speed)
 *        default is 1 (httpd handles window updates automatically)
 * @return ERR_OK: Accept the POST request, data may be passed in
 *         another err_t: Deny the POST request, send back 'bad request'.
 */
err_t httpd_post_begin(void *connection, const char *uri, const char *http_request,
                       u16_t http_request_len, int content_len, char *response_uri,
                       u16_t response_uri_len, u8_t *post_auto_wnd){
    struct http_state* hs = (struct http_state*)connection;

    udp_send_message(uri);
    if(parseOTABeginRequest(uri, &s_chunks, &s_contentLength)){
        if(ota_begin(s_contentLength)){
            postAddressee = PostAddressee_OTA_Begin;
        }
    }
    else if(parseOTAWriteRequest(uri, &s_chunkIdx, &s_chunkSize)){
        if(postAddressee == PostAddressee_OTA_Begin){
            postAddressee = PostAddressee_OTA_Write;
        }
    }
    else if(parseOTAEndRequest(uri)){
        if(postAddressee == PostAddressee_OTA_Write){
            ota_end();
            postAddressee = PostAddressee_OTA_End;
        }
    }
    else {
        postAddressee = PostAddressee_None;
    }

    if(postAddressee != PostAddressee_None){
        return ERR_OK;
    }
    else {
        return ERR_VAL;
    }
}

/**
 * @ingroup httpd
 * Called for each pbuf of data that has been received for a POST.
 * ATTENTION: The application is responsible for freeing the pbufs passed in!
 *
 * @param connection Unique connection identifier.
 * @param p Received data.
 * @return ERR_OK: Data accepted.
 *         another err_t: Data denied, http_post_get_response_uri will be called.
 */
err_t httpd_post_receive_data(void *connection, struct pbuf *p) {
    err_t result = ERR_OK;
    char udp_message[40];
    snprintf(udp_message, 40, "Receive data tot_len %d", p->tot_len);
    if(postAddressee == PostAddressee_OTA_Write){
        uint16_t pIdx = 0;
        for(struct pbuf* pb = p; pb != NULL; pb = pb->next){
            snprintf(udp_message, 40, "OTA Chunk %d: Writing %d bytes - %d", s_chunkIdx, pb->len, ++pIdx);
            udp_send_message(udp_message);
            /*
            if(!ota_write(p->payload, p->len)){
                return ERR_MEM;
            }
            */
        }
    }
    pbuf_free(p);
    return result;
}

/**
 * @ingroup httpd
 * Called when all data is received or when the connection is closed.
 * The application must return the filename/URI of a file to send in response
 * to this POST request. If the response_uri buffer is untouched, a 404
 * response is returned.
 *
 * @param connection Unique connection identifier.
 * @param response_uri Filename of response file, to be filled when denying the request
 * @param response_uri_len Size of the 'response_uri' buffer.
 */
void httpd_post_finished(void *connection, char *response_uri, u16_t response_uri_len) {
    struct http_state* hs = (struct http_state*)connection;
    if(postAddressee != PostAddressee_None){
        strcpy(response_uri,"/ok.html");
    }
    return;
}
