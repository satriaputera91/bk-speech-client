/*
 ============================================================================
 Name        : BKQueue
 Author      : satriaputera91@gmail.com
 Version     :
 Copyright   : PT Bahasa Kinerja Utama
 Description : queue text result
 ============================================================================
 */

#ifndef BK_QUEUE_RESULT_H_
#define BK_QUEUE_RESULT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

struct elt {
	struct elt *next;
	char out_brw[1024 * 1000];
	int len;
	int is_regular_response;
};

struct BKQueue {
	struct elt *head;
	struct elt *tail;
};

struct BKQueue *bk_speech_create_res(void);
void bk_speech_write_res(struct BKQueue *q, char *out_brw);
int bk_speech_empty_res(const struct BKQueue *q);
int bk_speech_read_res(struct BKQueue *q, char *out_brw, int *len);
void bk_speech_destroy_res(struct BKQueue *q);

#ifdef __cplusplus
}
#endif

#endif /* BK_QUEUE_RESULT_H_ */
