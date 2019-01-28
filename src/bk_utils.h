/*
 ============================================================================
 Name        : BKUtils
 Author      : satriaputera91@gmail.com
 Version     :
 Copyright   : PT Bahasa Kinerja Utama
 Description : function utils
 ============================================================================
*/
#ifndef BKUTILS_H_
#define BKUTILS_H_

#define _SVID_SOURCE
#include <string.h>


#include <stdio.h>
#include <ctype.h>
#include <syslog.h>
#define BK_STATE_GET_INIT 	1
#define BK_STATE_BOS 	2
#define BK_STATE_AUDIO 	3
#define BK_STATE_EOS 	4

#ifdef __cplusplus
extern "C" {
#endif

struct WriteThis {
	const char *readptr;
	int sizeleft;
};

struct string {
	char *ptr;
	size_t len;
};

typedef enum {
	false, true
} bool;

typedef enum {
	BK_RECOGNATION_CHUNKSIZE,
	BK_RECOGNATION_URL,
	BK_RECOGNATION_ENTITY,
	BK_RECOGNATION_OAUTH_URL,
	BK_RECOGNATION_USER_CREDENTIAL,
	BK_RECOGNATION_TIMEOUT
} BKRecognationOpt;

typedef enum {
	BK_SPEECH_OK,
	BK_ERROR_CA_OAUTH,
	BK_ERROR_OPT_INT,
	BK_ERROR_OPT_STRING,
	BK_ERROR_HTTP_POST,
	BK_ERROR_SET_CONFIG,
	BK_ERROR_GET_TOKEN,
	BK_ERROR_GET_INIT_API,
	BK_ERROR_BOS_API,
	BK_ERROR_AUDIO_API,
	BK_ERROR_EOS_API,
	BK_ERROR_SPEECH_RECOGNATION
} BKCode;

void generate_uuid(char *uuid);
double curent_time_milis();
char *trimwhitespace(char *str);
const char *bk_speech_strerr(BKCode bkcode);
char *base64encode(const void *b64_encode_this, int encode_this_many_bytes);
char *base64decode(const void *b64_decode_this, int decode_this_many_bytes);
#ifdef __cplusplus
}
#endif

#endif /* BKUTILS_H_ */
