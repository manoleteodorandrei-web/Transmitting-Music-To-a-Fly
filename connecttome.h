#ifndef CONNECTTOME_H
#define CONNECTTOME_H

#include <stdint.h>

// represents a single neuron
// a.k.a a neuron
typedef struct Neuron {
    uint64_t id;
    float voltage;
    float limit;
    uint32_t fire_count;
} Neuron;

// the Network (CSR format)
typedef struct Graph {
    uint64_t nr_neurons;
    uint64_t nr_synapses;
    Neuron* neurons; // array of all neurons 

    // CSR Arrays
    float* weights;
    uint32_t* target_ids;
    uint64_t* row_pointers; 
} Graph;


// Core Memory Functions
Graph* init_graph(uint64_t num_neurons, uint64_t num_synapses, uint64_t* row_pointers, uint32_t* target_ids, float* weights);
void free_graph(Graph* graph);
void simulate_network_tick(Graph* Graf);

#endif