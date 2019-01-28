/*
 ============================================================================
 Name        : BKUtils
 Author      : satriaputera91@gmail.com
 Version     :
 Copyright   : PT Bahasa Kinerja Utama
 Description : function utils
 ============================================================================
*/

#include "bk_utils.h"

#include <openssl/pem.h>
#include <uuid/uuid.h>

void generate_uuid(char *uuid) {

	uuid_t binuuid;
	uuid_generate_random(binuuid);

	uuid_unparse(binuuid, uuid);
}

double curent_time_milis() {
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
}

char *trimwhitespace(char *str) {
	char *end;

	// Trim leading space
	while (isspace((unsigned char )*str))
		str++;

	if (*str == 0)  // All spaces?
		return str;

	// Trim trailing space
	end = str + strlen(str) - 1;
	while (end > str && isspace((unsigned char )*end))
		end--;

	// Write new null terminator character
	end[1] = '\0';

	return str;
}

char *base64encode(const void *b64_encode_this, int encode_this_many_bytes) {
	BIO *b64_bio, *mem_bio;
	BUF_MEM *mem_bio_mem_ptr;
	b64_bio = BIO_new(BIO_f_base64());
	mem_bio = BIO_new(BIO_s_mem());
	BIO_push(b64_bio, mem_bio);
	BIO_set_flags(b64_bio, BIO_FLAGS_BASE64_NO_NL);
	BIO_write(b64_bio, b64_encode_this, encode_this_many_bytes);
	BIO_flush(b64_bio);
	BIO_get_mem_ptr(mem_bio, &mem_bio_mem_ptr);
	BIO_set_close(mem_bio, BIO_NOCLOSE);
	BIO_free_all(b64_bio);
	BUF_MEM_grow(mem_bio_mem_ptr, (*mem_bio_mem_ptr).length + 1);
	(*mem_bio_mem_ptr).data[(*mem_bio_mem_ptr).length] = '\0';
	return (*mem_bio_mem_ptr).data;
}

char *base64decode(const void *b64_decode_this, int decode_this_many_bytes) {
	BIO *b64_bio, *mem_bio;
	char *base64_decoded = calloc((decode_this_many_bytes * 3) / 4 + 1,
			sizeof(char));
	b64_bio = BIO_new(BIO_f_base64());
	mem_bio = BIO_new(BIO_s_mem());
	BIO_write(mem_bio, b64_decode_this, decode_this_many_bytes);
	BIO_push(b64_bio, mem_bio);
	BIO_set_flags(b64_bio, BIO_FLAGS_BASE64_NO_NL);
	int decoded_byte_index = 0;
	while (0 < BIO_read(b64_bio, base64_decoded + decoded_byte_index, 1)) {
		decoded_byte_index++;
	}
	BIO_free_all(b64_bio);
	return base64_decoded;
}

const char *bk_speech_strerr(BKCode bkcode) {

	char * strerr = NULL;

	if (bkcode == BK_ERROR_OPT_INT) {
		strerr = "error set optional integer value";
	} else if (bkcode == BK_ERROR_OPT_STRING) {
		strerr = "error set optional string value";
	} else if (bkcode == BK_ERROR_SET_CONFIG) {
		strerr = "please check again configuration!";
	} else if (bkcode == BK_ERROR_GET_TOKEN) {
		strerr = "token not valid";
	} else if (bkcode == BK_ERROR_CA_OAUTH) {
		strerr = "token not valid";
	} else if (bkcode == BK_ERROR_HTTP_POST) {
		strerr = "error connection to notula server api ";
	} else if (bkcode == BK_ERROR_GET_INIT_API) {
		strerr = "error process get init stream audio TTS";
	} else if (bkcode == BK_ERROR_BOS_API) {
		strerr = "error process begin stream audio TTS";
	} else if (bkcode == BK_ERROR_AUDIO_API) {
		strerr = "error process stream audio data TTS";
	} else if (bkcode == BK_ERROR_EOS_API) {
		strerr = "error process end of stream audio TTS";
	} else if (bkcode == BK_ERROR_SPEECH_RECOGNATION) {
		strerr = "error proces recognation TTS";
	}

	return strerr;
}
