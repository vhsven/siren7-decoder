#include <stdio.h>
#include <stdlib.h>
#include <libsiren/siren7.h>

int read_encoded_file(char* filename, unsigned char** data) {
	int encoded_size;
	FILE* f = fopen(filename, "rb");
	if (f == NULL) {
		fprintf(stderr, "Failed to open file '%s'\n", filename);
		exit(1);
	}
	fseek(f, 56, SEEK_SET);
	size_t result = fread(&encoded_size, 4, 1, f);
	if (result != 1) {
		fputs("Failed to read chunk size\n", stderr);
		exit(1);
	}
	*data = malloc(encoded_size);
	if(*data == NULL) {
		fputs("Failed to allocate input buffer\n", stderr);
		exit(1);	
	}
	result = fread(*data, 1, encoded_size, f);
	if(result != encoded_size) {
		fputs("Failed to read data chunk\n", stderr);
		exit(1);	
	}
	fclose(f);
	
	return encoded_size;
}

int decode(SirenDecoder* decoder, int encoded_size, unsigned char* encoded_data, unsigned char** decoded_data) {
	*decoded_data = malloc(encoded_size * 16);
	unsigned char* out_ptr = *decoded_data;
	int processed = 0;
	while (processed + 40 <= encoded_size) {
		if (Siren7_DecodeFrame(*decoder, encoded_data + processed, out_ptr) != 0) {
			fputs("Failed to decode frame\n", stderr);
			exit(1);
		}
		out_ptr += 640;
		processed += 40;
	}

	return out_ptr - *decoded_data;
}

int write_decoded_file(char* filename, PCMWavHeader header, unsigned char* data, int decoded_size) {
	FILE* f = fopen(filename, "wb");
	if (f == NULL) {
		fprintf(stderr, "Failed to open file '%s'\n", filename);
		exit(1);
	}
	if(fwrite(&header, sizeof(header), 1, f) != 1) {
		fputs("Failed to write header", stderr);
		exit(1);
	}
	if(fwrite(data, 1, decoded_size, f) != decoded_size) {
		fputs("Failed to write data", stderr);
		exit(1);
	}
	fclose(f);
}

int main(int argc, char *argv[]) {
	if(argc != 3) {
		fprintf(stderr, "Exactly 2 arguments required: encoded_filename (input) and decoded_filename (output)");
		exit(1);
	}
	unsigned char* encoded_data = NULL;
	int encoded_size = read_encoded_file(argv[1], &encoded_data);

	SirenDecoder decoder = Siren7_NewDecoder(16000);
	unsigned char* decoded_data = NULL;	
	int decoded_size = decode(&decoder, encoded_size, encoded_data, &decoded_data);

	write_decoded_file(argv[2], decoder->WavHeader, decoded_data, decoded_size);
	
	free(encoded_data);
	free(decoded_data);
	Siren7_CloseDecoder(decoder);
	return 0;
}