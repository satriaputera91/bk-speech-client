/*
 ============================================================================
 Name        : main onwav-sync-upload
 Author      : satriaputera91@gmail.com
 Version     :
 Copyright   : PT Bahasa Kinerja Utama
 Description : Example  Notula BK speech_API client
 ============================================================================
 */

#include <bkspeech/bk_speech_client.h>

size_t wav2bytes(const char *filename, char **byteaudio);

int main(int argc, char *argv[]) {

	SpeechClient speechBK;
	BKCode bc;

	char *byteaudio = NULL;

	char result[1024];
	size_t size = 0;
	int len = 0;

	/* ambil audio format wav, tanpa header (header less) format RAW 16KHZ 16BIT */
	size = wav2bytes(argv[1], &byteaudio);
	if (size == 0) {
		perror("can't read audio data");
		return 0;
	}

	printf("processing... file: %s,size: %li \n", argv[1], size);

	/* setting ukuran pengiriman data per potongan (chunck) <3200 | 6400 |9600> */
	bc = bk_speech_opt_int(&speechBK, BK_RECOGNATION_CHUNKSIZE, 9600);

	if (bc != BK_SPEECH_OK) {
		goto err;
	}

	/* setting koneksi timeout ke server api */
	bc = bk_speech_opt_int(&speechBK, BK_RECOGNATION_TIMEOUT, 30);
	if (bc != BK_SPEECH_OK) {
		goto err;
	}

	/* setting user credential pengguna */
	bc = bk_speech_opt_string(&speechBK, BK_RECOGNATION_USER_CREDENTIAL,
			"lalan:sem@ngat");
	if (bc != BK_SPEECH_OK) {
		goto err;
	}

	/* setting alamat URL Notula BK Speech API
	bc = bk_speech_opt_string(&speechBK, BK_RECOGNATION_URL,
			"https://api-dev.bahasakita.co.id/speech");
	if (bc != BK_SPEECH_OK) {
		goto err;
	}

	/* setting nama pengguna API */
	bc = bk_speech_opt_string(&speechBK, BK_RECOGNATION_ENTITY, "PTBahasakita");
	if (bc != BK_SPEECH_OK) {
		goto err;

	}

	/* setting alamat autentifikasi Notula BK speech API */
	bc = bk_speech_opt_string(&speechBK, BK_RECOGNATION_OAUTH_URL,
			"https://oauth.bahasakita.co.id/api/token");
	if (bc != BK_SPEECH_OK) {
		goto err;
	}

	/* memulai sesi data STT */
	bc = bk_speech_init(&speechBK);
	if (bc != BK_SPEECH_OK) {
		goto err;
	}

	/* mengirim audio file untuk diubah menjadi teks */
	bc = bk_speech_recognation(&speechBK, byteaudio, size);
	if (bc != BK_SPEECH_OK) {
		goto err;
	}

	/* baca hasil dari teks per baris kalimat !!! */
	while (!bk_speech_empty_res(speechBK.response)) {
		memset(result, 0, sizeof(result));
		bk_speech_read_res(speechBK.response, &result[0], &len);
		printf("%s \n", result);
	}

	/* membersihkan sesi data STT */
	bk_speech_cleanup(&speechBK);

	if (byteaudio)
		free(byteaudio);

	/* membersihkan konfigurasi Notula BK Speech API */
	bk_speech_free_all(&speechBK);

	err: if (bc != BK_SPEECH_OK) {
		fprintf(stderr, "bk_speech_api failed: %s\n", bk_speech_strerr(bc));
		assert(bc);

	}

	return 0;
}

size_t wav2bytes(const char *filename, char **byteaudio) {

	FILE *file;
	size_t filesize = 0;

	if ((file = fopen(filename, "rb")) != NULL) {

		fseek(file, 0, SEEK_END);
		filesize = ftell(file) - 44;
		rewind(file);
		fseek(file, 44, SEEK_SET);
		*byteaudio = malloc(filesize * sizeof(char));
		//header less
		long int result = fread(*byteaudio, 1, filesize, file);
		if (result != filesize) {
			filesize = 0;
		}

		fclose(file);

		return filesize;
	}

	return filesize;
}
