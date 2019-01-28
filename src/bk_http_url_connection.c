/*
 ============================================================================
 Name        : BKHttpURLConnection
 Author      : satriaputera91@gmail.com
 Version     :
 Copyright   : PT Bahasa Kinerja Utama
 Description : http connection with curl library
 ============================================================================
*/

#include "bk_http_url_connection.h"

static size_t wrfu(void *ptr, size_t size, size_t nmemb, void *stream) {
	(void) stream;
	(void) ptr;
	return size * nmemb;
}

static size_t read_callback(void *dest, size_t size, size_t nmemb, void *userp) {
	struct WriteThis *wt = (struct WriteThis *) userp;
	size_t buffer_size = size * nmemb;

	if (wt->sizeleft) {
		/* copy as much as possible from the source to the destination */
		size_t copy_this_much = wt->sizeleft;
		if (copy_this_much > buffer_size)
			copy_this_much = buffer_size;
		memcpy(dest, wt->readptr, copy_this_much);

		wt->readptr += copy_this_much;
		wt->sizeleft -= copy_this_much;
		return copy_this_much; /* we copied this many bytes */
	}

	return 0; /* no more data left to deliver */
}

void init_string(struct string *s) {
	s->len = 0;
	s->ptr = malloc(s->len + 1);
	if (s->ptr == NULL) {
		fprintf(stderr, "malloc() failed\n");
		exit(EXIT_FAILURE);
	}
	s->ptr[0] = '\0';
}

static size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s) {
	size_t new_len = s->len + size * nmemb;
	s->ptr = realloc(s->ptr, new_len + 1);
	if (s->ptr == NULL) {
		fprintf(stderr, "realloc() failed\n");
		exit(EXIT_FAILURE);
	}
	memcpy(s->ptr + s->len, ptr, size * nmemb);
	s->ptr[new_len] = '\0';
	s->len = new_len;

	return size * nmemb;
}

static CURLcode sslctx_function(CURL *curl, void *sslctx, void *parm) {
	CURLcode rv = CURLE_ABORTED_BY_CALLBACK;
	X509_STORE *store = NULL;
	X509 *cert = NULL;
	BIO *bio = NULL;
	ERR_clear_error();
	char * mypem = (char *) parm;
	/*get a BIO*/

	bio = BIO_new_mem_buf(mypem, -1);
	if (!bio)
		goto err;

	/* use it to read the PEM formatted certificate from memory into an X509
	 * structure that SSL can use*/

	if (!PEM_read_bio_X509(bio, &cert, 0, NULL))
		goto err;

	/*get a pointer to the X509 certificate store (whicurl may be empty!)*/
	store = SSL_CTX_get_cert_store((SSL_CTX *) sslctx);
	if (!store)
		goto err;

	/*add our certificate to this store*/
	if (!X509_STORE_add_cert(store, cert)) {
		unsigned long error = ERR_peek_last_error();

		/*Ignore error X509_R_CERT_ALREADY_IN_HASH_TABLE whicurl means the
		 * certificate is already in the store. That could happen if
		 * libcurl already loaded the certificate from a ca cert bundle
		 * set at libcurl build-time or runtime.*/

		if (ERR_GET_LIB(error) != ERR_LIB_X509 ||
		ERR_GET_REASON(error) != X509_R_CERT_ALREADY_IN_HASH_TABLE)
			goto err;

		ERR_clear_error();
	}

	rv = CURLE_OK;

	err: if (rv != CURLE_OK) {
		char errbuf[256];
		unsigned long error = ERR_peek_last_error();

		fprintf(stderr, "error adding certificate\n");
		if (error) {
			ERR_error_string_n(error, errbuf, sizeof(errbuf));
			fprintf(stderr, "%s\n", errbuf);
		}
	}

	X509_free(cert);
	BIO_free(bio);
	ERR_clear_error();

	return rv;
}

void bk_http_speech_info(ConfigSpeechBK *config_url) {
	char info[54000];
	sprintf(info,
			"URL\t\t: %s \nauthorization\t: %s\ncontent_type\t: %s\nbody\t\t: %s\ntimeout\t\t: %d\n",
			config_url->URL, config_url->authorization,
			config_url->content_type, config_url->body, config_url->timeout);

	printf("----- Information Http Req -----\n%s \n", info);
}

BKCode oauth_certificate(char *hostname, char *result) {
	CURL *curl;
	CURLcode res;
	curl = curl_easy_init();
	int i = 0;
	if (curl) {

		curl_easy_setopt(curl, CURLOPT_URL, hostname);

		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, *wrfu);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
		curl_easy_setopt(curl, CURLOPT_CERTINFO, 1L);

		res = curl_easy_perform(curl);

		if (!res) {
			printf("\n get certificate \n");
			struct curl_certinfo *certinfo;

			res = curl_easy_getinfo(curl, CURLINFO_CERTINFO, &certinfo);

			if (!res && certinfo) {
				memset(result, 0, strlen(result));
				for (i = 0; i < certinfo->num_of_certs; i++) {
					struct curl_slist *slist;

					for (slist = certinfo->certinfo[i]; slist;
							slist = slist->next) {

						printf("%s \n", slist->data);

						if (strncmp(slist->data,
								"Cert:-----BEGIN CERTIFICATE-----", 32) != 0) {

							continue;
						}

						strcpy(result, &slist->data[5]);

					}
				}
			}

		}

		curl_easy_cleanup(curl);
	}

	return BK_ERROR_CA_OAUTH;
}

BKCode http_post(ConfigSpeechBK *config_url, const char * ca_certificate,
		char * result) {
	CURLcode rv;
	CURL *curl;
	/*double speed_upload, total_time;*/

	struct string s;
	struct WriteThis wt;

	init_string(&s);
	wt.readptr = config_url->body;
	wt.sizeleft = strlen(config_url->body);

	curl = curl_easy_init();
	struct curl_slist *headers = NULL;

	if (curl) {
		/*Set Header*/
		headers = curl_slist_append(headers, config_url->authorization);
		headers = curl_slist_append(headers, config_url->content_type);
		headers = curl_slist_append(headers, "charsets: utf-8");
		headers = curl_slist_append(headers, "Expect:");

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		/* Set Request mode*/
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
		/*Set cerficate*/
		curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

		/*set decoding transfer*/
		curl_easy_setopt(curl, CURLOPT_HTTP_TRANSFER_DECODING, 0);

		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);

		curl_easy_setopt(curl, CURLOPT_CAINFO, NULL);
		curl_easy_setopt(curl, CURLOPT_CAPATH, NULL);

		/*curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, 1L);
		 curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION, *sslctx_function);
		 curl_easy_setopt(curl, CURLOPT_SSL_CTX_DATA, ca_certificate);
		 */
		/* set URL*/
		curl_easy_setopt(curl, CURLOPT_URL, config_url->URL);

		/*Set Read*/
		/* Now specify we want to POST data */
		curl_easy_setopt(curl, CURLOPT_POST, 1L);

		curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
		curl_easy_setopt(curl, CURLOPT_READDATA, &wt);

		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, (long )wt.sizeleft);

		/*set Timeout*/
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, config_url->timeout);

		/*Set Response output*/
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, *writefunc);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

		/* Perform the request, res will get the return code*/
		rv = curl_easy_perform(curl);

		curl_slist_free_all(headers);
		/*curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD, &speed_upload);
		 curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total_time);
		 printf("Speed: %.3f bytes/sec during %.3f seconds\n", speed_upload,
		 total_time);*/

		/*Check for errors*/
		if (rv != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(rv));
			return BK_ERROR_HTTP_POST;

		}
		/*Response data*/
		sprintf(result, "%s", s.ptr);
		free(s.ptr);

		/*always cleanup*/
		curl_easy_cleanup(curl);
	}

	return BK_SPEECH_OK;
}

void http_cleanup(ConfigSpeechBK *config_url) {
	if (config_url->URL != NULL) {
		free(config_url->URL);
	}

	if (config_url->body != NULL) {
		free(config_url->body);

	}
}

