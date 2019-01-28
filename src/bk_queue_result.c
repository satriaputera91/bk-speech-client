/*
 ============================================================================
 Name        : BKQueue
 Author      : satriaputera91@gmail.com
 Version     :
 Copyright   : PT Bahasa Kinerja Utama
 Description : queue text result
 ============================================================================
*/

#include "bk_queue_result.h"

struct BKQueue *bk_speech_create_res(void) {
	struct BKQueue *q;

	q = (struct BKQueue *) malloc(sizeof(struct BKQueue));
	q->head = q->tail = 0;

	return q;
}

void bk_speech_write_res(struct BKQueue *q, char *out_brw) {
	int n;
	struct elt *e;

	e = (struct elt *) malloc(sizeof(struct elt));
	assert(e);

	n = sprintf(e->out_brw, "%s", out_brw);
	e->out_brw[n] = '\0';

	e->len = n;

	/* Because I will be the tail, nobody is behind me */
	e->next = 0;
	if (q->head == 0) {
		/* If the queue was empty, I become the head */
		q->head = e;
	} else {
		/* Otherwise I get in line after the old tail */
		q->tail->next = e;
	}

	/* I become the new tail */
	q->tail = e;
}

int bk_speech_empty_res(const struct BKQueue *q) {
	return (q->head == 0);
}

int bk_speech_read_res(struct BKQueue *q, char *out_brw, int *len) {
	int n;
	int ret;

	struct elt *e;
	assert(!bk_speech_empty_res(q));

	n = sprintf(out_brw, "%s", q->head->out_brw);
	out_brw[n] = '\0';
	*len = q->head->len;

	/* patch out first element */
	e = q->head;
	q->head = e->next;

	free(e);
	ret = 0;
	return ret;
}


/* free a queue and all of its elements */
void bk_speech_destroy_res(struct BKQueue *q) {
	char out_brw[1024 * 10];
	int len;

	while (!bk_speech_empty_res(q)) {
		bk_speech_read_res(q, &out_brw[0], &len);
	}

	free(q);
}
