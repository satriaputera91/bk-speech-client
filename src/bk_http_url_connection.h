/*
 ============================================================================
 Name        : BKHttpURLConnection
 Author      : satriaputera91@gmail.com
 Version     :
 Copyright   : PT Bahasa Kinerja Utama
 Description : http connection with curl library
 ============================================================================
*/

#ifndef BK_HTTP_URL_CONNECTION_H_
#define BK_HTTP_URL_CONNECTION_H_
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <curl/curl.h>

#include "bk_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ConfigURL {
	char *URL;
	char *authorization;
	char *content_type;
	char *body;
	int timeout;
};
typedef struct ConfigURL ConfigSpeechBK;

void bk_http_speech_info(ConfigSpeechBK *config_url);
BKCode oauth_certificate(char *hostname, char *result);
BKCode http_post(ConfigSpeechBK *config_url, const char * ca_certificate,
		char * result);
void http_cleanup(ConfigSpeechBK *config_url);

#ifdef __cplusplus
}
#endif

#endif /* BK_HTTP_URL_CONNECTION_H_ */
