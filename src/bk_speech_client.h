/*
 ============================================================================
 Name        : BKSpeechClient
 Author      : satriaputera91@gmail.com
 Version     :
 Copyright   : PT Bahasa Kinerja Utama
 Description : speech client recognation
 ============================================================================
*/

#ifndef BK_SPEECH_CLIENT_H_
#define BK_SPEECH_CLIENT_H_
#include "bk_http_url_connection.h"
#include "bk_queue_result.h"

#define _GNU_SOURCE

#ifdef __cplusplus
extern "C" {
#endif

struct RecognationConfig {
	char *entity;
	char *oauth_url;
	char *url;
	char *user_credential;
	int timeout;
	int chunksize;
};

struct SpeechData {
	char *token_api;
	char *session_id;
	char *audio;
	char *utterance_id;
	char *ca_oauth;
	int offset;
	int len;
	int state;
};

struct SpeechBK {
	struct RecognationConfig rec_config;
	struct SpeechData speech_data;
	struct BKQueue *response;
};

typedef struct SpeechBK SpeechClient;

BKCode bk_speech_opt_int(SpeechClient *speech_client, BKRecognationOpt opt,
		int parm);
BKCode bk_speech_opt_string(SpeechClient *speech_client, BKRecognationOpt opt,
		const char *parm);
BKCode get_ca_oauth(SpeechClient *speech_client);
BKCode get_token(SpeechClient *speech_client);
BKCode bk_speech_init(SpeechClient *speech_client);
BKCode bk_speech_recognation(SpeechClient *speech_client, char *bytes,
		size_t len);

void bk_speech_cleanup(SpeechClient *speech_client);
void bk_speech_free_all(SpeechClient *speech_client);
void bk_speech_info(SpeechClient *speech_client);

#ifdef __cplusplus
}
#endif

#endif /* BK_SPEECH_CLIENT_H_ */
