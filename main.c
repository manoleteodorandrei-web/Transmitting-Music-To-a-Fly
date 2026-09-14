#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "connecttome.h"
#include "audio_pipeline.h"


Graph* load_csv(const char* filename, uint32_t nr_neurons) {
    FILE* file = fopen(filename, "r");
    if (!file) return NULL;

    printf("Allocating memory for %u neurons...\n", nr_neurons);

    char line[256];
    uint32_t source, target;
    float weight;
    uint64_t total_edges = 0;

    printf("Parsing synapses (Pass 1/2: Counting edges)...\n");
    fgets(line, sizeof(line), file); // We skip the header here

    // Allocate array to compute CSR row pointers
    uint64_t* out_degree = (uint64_t*)calloc(nr_neurons, sizeof(uint64_t));
    if (!out_degree) {
        fclose(file);
        return NULL;
    }

    while(fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%u,%u,%f", &source, &target, &weight) == 3) {
            if (source < nr_neurons) {
                out_degree[source]++;
                total_edges++;
            }
        }
    }

    printf("Total valid biological edges found: %lu\n", total_edges);

    // Allocate CSR arrays
    uint64_t* row_pointers = (uint64_t*)malloc((nr_neurons + 1) * sizeof(uint64_t));
    uint32_t* target_ids = (uint32_t*)malloc(total_edges * sizeof(uint32_t));
    float* weights = (float*)malloc(total_edges * sizeof(float));

    if (!row_pointers || !target_ids || !weights) {
        if (row_pointers) free(row_pointers);
        if (target_ids) free(target_ids);
        if (weights) free(weights);
        free(out_degree);
        fclose(file);
        return NULL;
    }

    // Compute prefix sums for row_pointers
    row_pointers[0] = 0;
    for (uint32_t i = 0; i < nr_neurons; i++) {
        row_pointers[i + 1] = row_pointers[i] + out_degree[i];
    }

    printf("Parsing synapses (Pass 2/2: Populating arrays)...\n");
    rewind(file);
    fgets(line, sizeof(line), file); // skip header again

    uint64_t* current_offset = (uint64_t*)malloc(nr_neurons * sizeof(uint64_t));
    memcpy(current_offset, row_pointers, nr_neurons * sizeof(uint64_t));

    while(fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%u,%u,%f", &source, &target, &weight) == 3) {
            if (source < nr_neurons) {
                uint64_t idx = current_offset[source]++;
                target_ids[idx] = target;
                weights[idx] = weight;
            }
        }
    }

    free(current_offset);
    free(out_degree);
    fclose(file);
    
    printf("Successfully linked %lu biological edges into contiguous CSR memory!\n", total_edges);
    
    // Now initialize the graph
    Graph* Graf = init_graph(nr_neurons, total_edges, row_pointers, target_ids, weights);
    
    return Graf;
}

int main() {
    uint32_t FLIES_TOTAL_NEURONS = 135000;
    Graph* Brain = load_csv("mapped_connections.csv", FLIES_TOTAL_NEURONS);
    int frames_per_chunk = 1024;

    if(!Brain) {
        printf("Failed to load!\n");
        return 1;
    }

    if (init_audio_stream("Pulsewidth.wav") < 0) {
        free_graph(Brain);
        return 1;
    }

    double freq_magnitudes[512];
    int chunks_processed = 0;
    
    double peak_bass = 1.0;
    double peak_treble = 1.0;

    printf("Starting the Engine...\n");

    while(read_and_process_audio(frames_per_chunk, freq_magnitudes) > 0) {
        chunks_processed++;

        if (chunks_processed % 1000 == 0) {
            printf("Simulation running... processed %d audio chunks.\n", chunks_processed);
        }

        double raw_bass = freq_magnitudes[2];
        double raw_treble = freq_magnitudes[20];
        
        if (raw_bass > peak_bass) peak_bass = raw_bass;
        if (raw_treble > peak_treble) peak_treble = raw_treble;
        
        float scaled_bass = (float)(raw_bass / peak_bass);
        float scaled_treble = (float)(raw_treble / peak_treble);

        if (scaled_bass > 1.0f) scaled_bass = 1.0f;
        if (scaled_treble > 1.0f) scaled_treble = 1.0f;

        Brain->neurons[56806].voltage += (scaled_bass * 5.0f);
        Brain->neurons[95985].voltage += (scaled_treble * 5.0f);
        
        simulate_network_tick(Brain);
    }
    printf("Song finished! Total chunks processed: %d\n", chunks_processed);

    uint64_t total_network_firings = 0;
    for (uint32_t i = 0; i < Brain->nr_neurons; i++) {
        total_network_firings += Brain->neurons[i].fire_count;
    }

    printf("\n--- Simulation Results ---\n");
    printf("Global Network Firings: %lu\n", total_network_firings);
    printf("Left Wing Firings: %u\n", Brain->neurons[109555].fire_count);
    printf("Right Wing Firings: %u\n", Brain->neurons[95985].fire_count);

    if (Brain->neurons[109555].fire_count > Brain->neurons[95985].fire_count) {
        printf("Conclusion: The fly leaned left, It disliked the bass line.\n");
    } else if (Brain->neurons[95985].fire_count > Brain->neurons[109555].fire_count) {
        printf("Conclusion; The fly leaned right. It engaged with the rhythm.\n");
    } else {
        printf("Conclusion: Symmetrical response or no output reached.\n");
    }

    close_audio_stream();
    free_graph(Brain);
    return 0;
}