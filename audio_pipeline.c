#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <fftw3.h>
#include "audio_pipeline.h"

FILE* audio_file;

static int initialized = 0;
static int16_t* raw_buffer = NULL;
static double* in = NULL;
static fftw_complex* out = NULL;
static fftw_plan plan;

int init_audio_stream(const char* filename) {
    audio_file = fopen(filename, "rb");
    if(!audio_file) return -1;
    fseek(audio_file, 44, SEEK_SET); // Skip WAV header
    return 0;
}

int read_and_process_audio(int frames, double* freq_magnitudes) {
    if(!audio_file) return 0;

    // Allocate FFT memory and hardware plan once
    if (!initialized) {
        raw_buffer = (int16_t*)malloc(frames * sizeof(int16_t));
        in = (double*)fftw_malloc(sizeof(double) * frames);
        out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * frames);
        plan = fftw_plan_dft_r2c_1d(frames, in, out, FFTW_ESTIMATE);
        initialized = 1;
    }

    size_t read_count = fread(raw_buffer, sizeof(int16_t), frames, audio_file);
    if (read_count == 0) {
        return 0; // File finished
    }

    for (int i = 0; i < frames; i++) {
        in[i] = (double)raw_buffer[i];
    }

    fftw_execute(plan);

    for(int i = 0;  i < (frames / 2); i++) {
        // We use Pythagora Theorem to calculate the magnitude of the complex number, where out[i][0] is the real part and out[i][1] is the imaginary part
        freq_magnitudes[i] = sqrt((out[i][0] * out[i][0]) + (out[i][1] * out[i][1]));
    }

    return read_count;
}

void close_audio_stream() {
    if (audio_file) fclose(audio_file);
    
    // Free the static memory when the simulation finishes
    if (initialized) {
        fftw_destroy_plan(plan);
        fftw_free(in);
        fftw_free(out);
        free(raw_buffer);
        initialized = 0;
    }
}