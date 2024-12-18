/*
 * httpd_post.c
 *
 *  Created on: Jun 27, 2024
 *      Author: schroeder
 */


#include "httpd.h"
#include "string.h"
#include "mem.h"
#include "../IAP_StateMachine/IAP_StateMachine.h"

typedef enum  {
    PostAddressee_None,
    PostAddressee_IAPBinaryFile,
    PostAddressee_IAPCopyBinaryToFlash,
    PostAddressee_IAPLoadApplication
} PostAddressee;

PostAddressee postAddressee = PostAddressee_None;
#define FILENAME_LENGHT 20
char filename[FILENAME_LENGHT];

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
    if(strcmp(uri,"/upload.cgi")==0){
        postAddressee = PostAddressee_IAPBinaryFile;
        IAP_BufferClear();
        return ERR_OK;
    }
    else if(strcmp(uri,"/copyBufferToFlash")==0){
        postAddressee = PostAddressee_IAPCopyBinaryToFlash;
        return ERR_OK;
    }
    else if(strcmp(uri,"/loadApp")==0){
        postAddressee = PostAddressee_IAPLoadApplication;
        return ERR_OK;
    }
    postAddressee = PostAddressee_None;
    return ERR_VAL;
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

    if(postAddressee == PostAddressee_IAPBinaryFile){
        if(!IAP_BufferAppend(p->payload, p->len)){
            result = ERR_MEM;
        }
    }
    else if(postAddressee == PostAddressee_IAPCopyBinaryToFlash){
        if(!IAP_CopyBufferToFlash()){
            result = ERR_MEM;
        }
    }
    else if(postAddressee == PostAddressee_IAPLoadApplication){
        if(IAPrequestState(IAP_STATE_LoadWait)){
            result = ERR_MEM;
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
    postAddressee = PostAddressee_None;
    strcpy(response_uri,"/index.html");
    return;
}
