/*
 ============================================================================
 Name        : BKSpeechClient
 Author      : satriaputera91@gmail.com
 Version     :
 Copyright   : PT Bahasa Kinerja Utama
 Description : speech client recognation
 ============================================================================
 */

#include <json-c/json.h>

#include "bk_speech_client.h"

BKCode bk_speech_opt_int(SpeechClient *speech_client, BKRecognationOpt opt,
		int parm) {

	struct RecognationConfig *rec_config = &speech_client->rec_config;
	if (opt == BK_RECOGNATION_CHUNKSIZE) {
		if (parm >= 3200 && parm <= 9600) {
			rec_config->chunksize = parm;
		} else {
			syslog(LOG_ERR, "error chunck set only 3200 ~ 9600");
			return BK_ERROR_OPT_INT;
		}
	} else if (opt == BK_RECOGNATION_TIMEOUT) {
		if (parm > 0) {
			rec_config->timeout = parm;
		} else {
			syslog(LOG_ERR, "error set timeout more than 1 sec");
			return BK_ERROR_OPT_INT;

		}
	} else {
		return BK_ERROR_OPT_INT;
	}
	syslog(LOG_INFO, "%d", parm);

	return BK_SPEECH_OK;
}

BKCode bk_speech_opt_string(SpeechClient *speech_client, BKRecognationOpt opt,
		const char *parm) {

	struct RecognationConfig *rec_config = &speech_client->rec_config;
	if (opt == BK_RECOGNATION_URL) {
		rec_config->url = malloc(strlen(parm) + 1);
		strcpy(rec_config->url, parm);
	} else if (opt == BK_RECOGNATION_ENTITY) {
		rec_config->entity = malloc(strlen(parm) + 1);
		strcpy(rec_config->entity, parm);
	} else if (opt == BK_RECOGNATION_OAUTH_URL) {
		rec_config->oauth_url = malloc(strlen(parm) + 1);
		strcpy(rec_config->oauth_url, parm);
	} else if (opt == BK_RECOGNATION_USER_CREDENTIAL) {
		rec_config->user_credential = malloc(strlen(parm) + 1);
		strcpy(rec_config->user_credential, parm);
	} else {
		return BK_ERROR_OPT_STRING;
	}
	syslog(LOG_INFO, "%s", parm);

	return BK_SPEECH_OK;
}

BKCode get_ca_oauth(SpeechClient *speech_client) {
	char result[2048];
	struct SpeechData *speech_data = &speech_client->speech_data;
	struct RecognationConfig *rec_config = &speech_client->rec_config;

	memset(result, 0, strlen(result));
	oauth_certificate(rec_config->oauth_url, result);
	speech_data->ca_oauth = malloc(strlen(result) + 1);

	strcpy(speech_data->ca_oauth, result);

	printf("%s \n", speech_data->ca_oauth);

	return BK_SPEECH_OK;
}

void create_session(SpeechClient *speech_client) {
	struct SpeechData *speech_data = &speech_client->speech_data;
	speech_data->session_id = malloc(37);
	generate_uuid(speech_data->session_id);
}

BKCode get_init(SpeechClient *speech_client) {
	struct RecognationConfig *rec_config = &speech_client->rec_config;
	struct SpeechData *speech_data = &speech_client->speech_data;
	struct ConfigURL config_url;
	BKCode bk;

	char result[1024];
	char stan[1024];
	char authorization[1024];

	/*struct json_object *jobj*/;
	struct json_object *jmessage;
	struct json_object *jbk;
	struct json_object *jdata;
	struct json_object *jcode;

	speech_data->state = BK_STATE_GET_INIT;

	generate_uuid(stan);
	config_url.URL = strdup(rec_config->url);
	sprintf(authorization, "Authorization: Bearer %s", speech_data->token_api);

	config_url.authorization = &authorization[0];
	config_url.content_type = "Content-Type: application/json";

	jmessage = json_object_new_object();
	jbk = json_object_new_object();
	jdata = json_object_new_object();

	json_object_object_add(jbk, "cmd", json_object_new_string("init"));
	json_object_object_add(jbk, "entity",
			json_object_new_string(rec_config->entity));
	json_object_object_add(jbk, "protocol", json_object_new_string("stt"));
	json_object_object_add(jbk, "stan", json_object_new_string(stan));
	json_object_object_add(jbk, "time",
			json_object_new_int64(curent_time_milis()));
	json_object_object_add(jbk, "version", json_object_new_string("1.0"));
	json_object_object_add(jbk, "type", json_object_new_int(0));
	json_object_object_add(jdata, "session_id",
			json_object_new_string(speech_data->session_id));
	json_object_object_add(jbk, "data", jdata);
	json_object_object_add(jmessage, "bk", jbk);
	/*printf("json init: %s", json_object_to_json_string(jmessage));*/

	config_url.body = strdup(json_object_to_json_string(jmessage));
	config_url.timeout = rec_config->timeout;

	memset(result, 0, sizeof(result));

	syslog(LOG_INFO, "%s", config_url.body);
	bk = http_post(&config_url, speech_data->ca_oauth, result);
	syslog(LOG_INFO, "%s", result);

	json_object_put(jmessage);
	http_cleanup(&config_url);

	if (bk == BK_SPEECH_OK) {
		jmessage = json_tokener_parse(result);

		if (jmessage != NULL) {

			json_object_object_get_ex(jdata, "code", &jcode);
			if (json_object_get_int(jcode) != 0) {
				bk = BK_ERROR_GET_INIT_API;
				syslog(LOG_ERR, "%s", bk_speech_strerr(bk));

			}
			json_object_put(jmessage);
		} else {
			bk = BK_ERROR_GET_INIT_API;
			syslog(LOG_ERR, "%s failed %s", bk_speech_strerr(bk), result);

		}
	}

	return bk;
}

BKCode bos(SpeechClient *speech_client) {
	struct RecognationConfig *rec_config = &speech_client->rec_config;
	struct SpeechData *speech_data = &speech_client->speech_data;
	struct ConfigURL config_url;
	BKCode bk;

	char result[1024];
	char stan[37];
	char authorization[512];

	/*struct json_object *jobj;*/
	struct json_object *jmessage;
	struct json_object *jbk;
	struct json_object *jdata;
	struct json_object *jvalue;
	struct json_object *jcode;

	speech_data->len = 0;
	speech_data->offset = 0;
	speech_data->state = BK_STATE_BOS;
	speech_data->audio = NULL;

	generate_uuid(stan);

	config_url.URL = strdup(rec_config->url);

	sprintf(authorization, "Authorization: Bearer %s", speech_data->token_api);
	config_url.authorization = &authorization[0];

	config_url.content_type = "Content-Type: application/json";

	jmessage = json_object_new_object();
	jbk = json_object_new_object();
	jdata = json_object_new_object();

	json_object_object_add(jbk, "cmd", json_object_new_string("bos"));
	json_object_object_add(jbk, "entity",
			json_object_new_string(rec_config->entity));
	json_object_object_add(jbk, "protocol", json_object_new_string("stt"));
	json_object_object_add(jbk, "stan", json_object_new_string(stan));
	json_object_object_add(jbk, "time",
			json_object_new_int64(curent_time_milis()));
	json_object_object_add(jbk, "version", json_object_new_string("1.0"));
	json_object_object_add(jbk, "type", json_object_new_int(0));

	json_object_object_add(jdata, "session_id",
			json_object_new_string(speech_data->session_id));
	json_object_object_add(jbk, "data", jdata);
	json_object_object_add(jmessage, "bk", jbk);

	config_url.body = strdup(json_object_to_json_string(jmessage));
	config_url.timeout = rec_config->timeout;

	json_object_put(jmessage);
	memset(result, 0, sizeof(result));
	syslog(LOG_INFO, "%s", config_url.body);
	bk = http_post(&config_url, speech_data->ca_oauth, result);
	syslog(LOG_INFO, "%s", result);

	if (bk == BK_SPEECH_OK) {
		jmessage = json_tokener_parse(result);

		if (jmessage != NULL) {
			json_object_object_get_ex(jmessage, "bk", &jbk);
			json_object_object_get_ex(jbk, "data", &jdata);
			json_object_object_get_ex(jbk, "code", &jcode);

			if (json_object_get_int(jcode) == 0) {
				json_object_object_get_ex(jdata, "utterance_id", &jvalue);
				speech_data->utterance_id = strdup(
						json_object_get_string(jvalue));

			} else {
				bk = BK_ERROR_BOS_API;
				syslog(LOG_ERR, "%s", bk_speech_strerr(bk));
			}

			json_object_put(jmessage);
			http_cleanup(&config_url);
		} else {
			bk = BK_ERROR_BOS_API;
			syslog(LOG_ERR, "%s failed %s", bk_speech_strerr(bk), result);

		}
	}
	return bk;

}

BKCode audio(SpeechClient *speech_client, char *base64audio) {

	struct RecognationConfig *rec_config = &speech_client->rec_config;
	struct SpeechData *speech_data = &speech_client->speech_data;
	struct ConfigURL config_url;
	BKCode bk;

	char result[4096];
	char stan[37];
	char authorization[512];

	struct json_object *jmessage;
	struct json_object *jbk;
	struct json_object *jdata;
	struct json_object *jvalue;
	struct json_object *jtext;
	struct json_object *jcode;

	struct json_object *jarray_text;
	struct json_object *jresult_type;
	struct json_object *jresult_value;

	int array_length;
	int i;

	speech_data->state = BK_STATE_AUDIO;

	generate_uuid(stan);
	config_url.URL = strdup(rec_config->url);
	sprintf(authorization, "Authorization: Bearer %s", speech_data->token_api);
	config_url.authorization = &authorization[0];
	config_url.content_type = "Content-Type: application/json";

	jmessage = json_object_new_object();
	jbk = json_object_new_object();
	jdata = json_object_new_object();

	json_object_object_add(jbk, "cmd", json_object_new_string("audio"));
	json_object_object_add(jbk, "entity",
			json_object_new_string(rec_config->entity));
	json_object_object_add(jbk, "protocol", json_object_new_string("stt"));
	json_object_object_add(jbk, "stan", json_object_new_string(stan));
	json_object_object_add(jbk, "time",
			json_object_new_int64(curent_time_milis()));
	json_object_object_add(jbk, "version", json_object_new_string("1.0"));
	json_object_object_add(jbk, "type", json_object_new_int(0));
	json_object_object_add(jdata, "session_id",
			json_object_new_string(speech_data->session_id));
	json_object_object_add(jdata, "offset",
			json_object_new_int64(speech_data->offset));
	json_object_object_add(jdata, "len",
			json_object_new_int64(speech_data->len));
	json_object_object_add(jdata, "audio", json_object_new_string(base64audio));
	json_object_object_add(jdata, "utterance_id",
			json_object_new_string(speech_data->utterance_id));
	json_object_object_add(jbk, "data", jdata);
	json_object_object_add(jmessage, "bk", jbk);

	config_url.body = strdup(json_object_to_json_string(jmessage));
	config_url.timeout = rec_config->timeout;

	memset(result, 0, sizeof(result));

	json_object_put(jmessage);

	syslog(LOG_INFO, "%s", config_url.body);
	bk = http_post(&config_url, speech_data->ca_oauth, result);
	syslog(LOG_INFO, "%s", result);

	if (bk == BK_SPEECH_OK) {

		jmessage = json_tokener_parse(result);

		if (jmessage != NULL) {

			json_object_object_get_ex(jmessage, "bk", &jbk);
			json_object_object_get_ex(jbk, "data", &jdata);
			json_object_object_get_ex(jbk, "code", &jcode);

			if (json_object_get_int(jcode) == 0) {
				if (json_object_object_get_ex(jdata, "text", &jtext)) {

					array_length = json_object_array_length(jtext);

					for (i = 0; i < array_length; i++) {

						jarray_text = json_object_array_get_idx(jtext, i);

						if (json_object_object_get_ex(jarray_text, "type",
								&jresult_type)) {

							if (strncmp(json_object_get_string(jresult_type),
									"final", 5) == 0) {

								if (json_object_object_get_ex(jarray_text,
										"value", &jresult_value)) {

									bk_speech_write_res(speech_client->response,
											(char *) json_object_get_string(
													jresult_value));
								}
							}
						}

					}
				}

				json_object_object_get_ex(jdata, "offset", &jvalue);
				speech_data->offset = json_object_get_int(jvalue);
			} else {
				bk = BK_ERROR_AUDIO_API;
				syslog(LOG_ERR, "%s", bk_speech_strerr(bk));

			}

			json_object_put(jmessage);

		} else {
			bk = BK_ERROR_AUDIO_API;
			syslog(LOG_ERR, "%s failed %s", bk_speech_strerr(bk), result);

		}
	}

	if (base64audio != NULL) {
		free(base64audio);
	}

	http_cleanup(&config_url);

	return bk;
}

BKCode eos(SpeechClient *speech_client) {

	struct RecognationConfig *rec_config = &speech_client->rec_config;
	struct SpeechData *speech_data = &speech_client->speech_data;
	struct ConfigURL config_url;
	BKCode bk;

	char result[4096];
	char stan[37];
	char authorization[512];

	struct json_object *jmessage;
	struct json_object *jbk;
	struct json_object *jdata;
	struct json_object *jcode;
	struct json_object *jtext;
	struct json_object *jarray_text;
	struct json_object *jresult_type;
	struct json_object *jresult_value;
	int array_length, i;

	generate_uuid(stan);

	speech_data->state = BK_STATE_EOS;

	config_url.URL = strdup(rec_config->url);
	sprintf(authorization, "Authorization: Bearer %s", speech_data->token_api);
	config_url.authorization = &authorization[0];
	config_url.content_type = "Content-Type: application/json";

	jmessage = json_object_new_object();
	jbk = json_object_new_object();
	jdata = json_object_new_object();

	json_object_object_add(jbk, "cmd", json_object_new_string("eos"));
	json_object_object_add(jbk, "entity",
			json_object_new_string(rec_config->entity));
	json_object_object_add(jbk, "protocol", json_object_new_string("stt"));
	json_object_object_add(jbk, "stan", json_object_new_string(stan));
	json_object_object_add(jbk, "time",
			json_object_new_int64(curent_time_milis()));
	json_object_object_add(jbk, "version", json_object_new_string("1.0"));
	json_object_object_add(jbk, "type", json_object_new_int(0));
	json_object_object_add(jdata, "session_id",
			json_object_new_string(speech_data->session_id));
	json_object_object_add(jdata, "utterance_id",
			json_object_new_string(speech_data->utterance_id));

	json_object_object_add(jbk, "data", jdata);
	json_object_object_add(jmessage, "bk", jbk);
	/*printf("json init: %s", json_object_to_json_string(jmessage));*/

	config_url.body = strdup(json_object_to_json_string(jmessage));
	config_url.timeout = rec_config->timeout;

	json_object_put(jmessage);
	memset(result, 0, sizeof(result));

	syslog(LOG_INFO, "%s", config_url.body);
	bk = http_post(&config_url, speech_data->ca_oauth, result);
	syslog(LOG_INFO, "%s", result);

	if (bk == BK_SPEECH_OK) {
		jmessage = json_tokener_parse(result);
		json_object_object_get_ex(jmessage, "bk", &jbk);

		if (jbk != NULL) {

			json_object_object_get_ex(jbk, "data", &jdata);
			json_object_object_get_ex(jbk, "code", &jcode);

			if (json_object_get_int(jcode) == 0) {

				json_object_object_get_ex(jdata, "text", &jtext);
				array_length = json_object_array_length(jtext);

				for (i = 0; i < array_length; i++) {

					jarray_text = json_object_array_get_idx(jtext, i);

					if (json_object_object_get_ex(jarray_text, "type",
							&jresult_type)) {

						if (strncmp(json_object_get_string(jresult_type),
								"final", 5) == 0) {

							if (json_object_object_get_ex(jarray_text, "value",
									&jresult_value)) {

								bk_speech_write_res(speech_client->response,
										(char *) json_object_get_string(
												jresult_value));

							}
						}
					}
				}
			} else {
				bk = BK_ERROR_EOS_API;
				syslog(LOG_ERR, "%s", bk_speech_strerr(bk));
			}
			json_object_put(jmessage);
		} else {
			bk = BK_ERROR_EOS_API;
			syslog(LOG_ERR, "%s failed %s", bk_speech_strerr(bk), result);
		}
	}

	http_cleanup(&config_url);
	return bk;
}

BKCode get_token(SpeechClient *speech_client) {

	struct RecognationConfig *rec_config = &speech_client->rec_config;
	struct SpeechData *speech_data = &speech_client->speech_data;
	struct ConfigURL config_url;
	BKCode bk;
	char result[1024];
	char authorization[512];
	char body[64];

	struct json_object *jmessage;
	struct json_object *tmp;

	config_url.URL = strdup(rec_config->oauth_url);
	sprintf(authorization, "Authorization: Basic %s",
			base64encode(rec_config->user_credential,
					strlen(rec_config->user_credential)));
	config_url.authorization = &authorization[0];
	config_url.content_type = "Content-Type: application/x-www-form-urlencoded";

	strcpy(body, "grant_type=client_credentials");
	strcat(body, "&scope=speech");
	config_url.body = strdup(body);
	config_url.timeout = rec_config->timeout;

	memset(result, 0, sizeof(result));

	syslog(LOG_INFO, "%s", config_url.body);
	bk = http_post(&config_url, speech_data->ca_oauth, result);
	syslog(LOG_INFO, "%s", result);

	if (bk == BK_SPEECH_OK) {
		jmessage = json_tokener_parse(result);

		if (jmessage != NULL) {

			if (json_object_object_get_ex(jmessage, "access_token", &tmp)) {
				speech_data->token_api = strdup(json_object_get_string(tmp));
			} else {
				bk = BK_ERROR_GET_TOKEN;
				syslog(LOG_ERR, "%s", bk_speech_strerr(bk));

			}

			json_object_put(jmessage);
			http_cleanup(&config_url);

		} else {
			bk = BK_ERROR_GET_TOKEN;
			syslog(LOG_ERR, "%s failed %s", bk_speech_strerr(bk), result);

		}
	}

	return bk;
}

BKCode bk_speech_init(SpeechClient *speech_client) {
	struct RecognationConfig *rec_config = &speech_client->rec_config;
	struct SpeechData *speech_data = &speech_client->speech_data;

	if (rec_config->entity == NULL || strlen(rec_config->entity) < 0) {
		syslog(LOG_ERR, "error configuration");
		return BK_ERROR_SET_CONFIG;
	}
	if (rec_config->oauth_url == NULL || strlen(rec_config->oauth_url) < 0) {
		syslog(LOG_ERR, "error configuration");

		return BK_ERROR_SET_CONFIG;
	}
	if (rec_config->url == NULL || strlen(rec_config->url) < 0) {
		syslog(LOG_ERR, "error configuration");

		return BK_ERROR_SET_CONFIG;
	}
	if (rec_config->user_credential == NULL
			|| strlen(rec_config->user_credential) < 0) {
		syslog(LOG_ERR, "error configuration");

		return BK_ERROR_SET_CONFIG;
	}

	speech_data->audio = NULL;
	speech_data->ca_oauth = NULL;
	speech_data->len = 0;
	speech_data->offset = 0;
	speech_data->session_id = NULL;
	speech_data->state = 0;
	speech_data->token_api = NULL;
	speech_data->utterance_id = NULL;

	if (rec_config->chunksize == 0)
		rec_config->chunksize = 6400;

	if (rec_config->timeout == 0)
		rec_config->timeout = 30;

	if (get_token(speech_client)) {
		syslog(LOG_ERR, "error get token");
		return BK_ERROR_GET_TOKEN;
	}

	create_session(speech_client);
	curl_global_init(CURL_GLOBAL_ALL);
	speech_client->response = bk_speech_create_res();

	return BK_SPEECH_OK;
}

BKCode bk_speech_recognation(SpeechClient *speech_client, char *bytes,
		size_t len) {

	struct RecognationConfig *rec_config = &speech_client->rec_config;
	struct SpeechData *speech_data = &speech_client->speech_data;

	char audio_speech[rec_config->chunksize];
	BKCode bk;
	int remainder;
	int iteration = 1;

	bk = get_init(speech_client);

	if (bk == BK_SPEECH_OK) {

		bk = bos(speech_client);

		if (bk == BK_SPEECH_OK) {

			remainder = len;

			while (remainder > 0) {
				if (remainder >= rec_config->chunksize) {
					speech_data->len = rec_config->chunksize;
					remainder = remainder - speech_data->len;

				} else {
					speech_data->len = len - speech_data->offset;
					remainder = remainder - speech_data->len;
				}

				memset(audio_speech, 0, rec_config->chunksize);
				memcpy(audio_speech, bytes + speech_data->offset,
						speech_data->len);

				/*printf("iter %3d : remainder : %d\t offset : %d \tlength:%d \n",
				 iteration, remainder, speech_data->offset, speech_data->len)*/;

				bk = audio(speech_client,
						base64encode(audio_speech, speech_data->len));

				if (bk != BK_SPEECH_OK) {
					break;
				}

				speech_data->offset = speech_data->offset + speech_data->len;
				iteration++;
			}
			bk = eos(speech_client);
		}
	}

	return bk;

}

void bk_speech_cleanup(SpeechClient *speech_client) {
	struct SpeechData *speech_data = &speech_client->speech_data;

	if (speech_data->token_api != NULL) {
		free(speech_data->token_api);
	}

	if (speech_data->session_id != NULL) {
		free(speech_data->session_id);
	}

	if (speech_data->audio != NULL) {
		free(speech_data->audio);
	}

	if (speech_data->utterance_id != NULL) {
		free(speech_data->utterance_id);
	}

	if (speech_data->ca_oauth != NULL) {
		free(speech_data->ca_oauth);
	}

	curl_global_cleanup();
	bk_speech_destroy_res(speech_client->response);
}

void bk_speech_free_all(SpeechClient *speech_client) {
	struct RecognationConfig *rec_config = &speech_client->rec_config;

	if (rec_config->url != NULL) {
		free(rec_config->url);
	}

	if (rec_config->entity != NULL) {
		free(rec_config->entity);
	}

	if (rec_config->oauth_url != NULL) {
		free(rec_config->oauth_url);
	}

	if (rec_config->user_credential != NULL) {
		free(rec_config->user_credential);
	}
}

void bk_speech_info(SpeechClient *speech_client) {

	char info[2048];
	struct RecognationConfig *rec_config = &speech_client->rec_config;
	sprintf(info,
			"URL\t\t: %s\nEntity\t\t: %s\nOauthURL\t: %s\nTimeout\t\t: %d\nChunckSize\t: %d\n",
			rec_config->url, rec_config->entity, rec_config->oauth_url,
			rec_config->timeout, rec_config->chunksize);
	printf("----- info configuration-----\n%s \n", info);
}
