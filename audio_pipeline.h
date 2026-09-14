#ifndef AUDIO_PIPELINE_H
#define AUDIO_PIPELINE_H

#include <stdint.h>

int init_audio_stream(const char* filename);
int read_and_process_audio(int frames, double* freq_magnitudes);
void close_audio_stream();

#endif