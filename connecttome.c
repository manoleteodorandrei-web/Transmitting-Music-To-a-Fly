#include <stdio.h>
#include <stdlib.h>
#include "connecttome.h"

Graph* init_graph(uint64_t num_neurons, uint64_t num_synapses, uint64_t* row_pointers, uint32_t* target_ids, float* weights) {
    Graph* Graf = (Graph*)malloc(sizeof(Graph));
    if (!Graf) return NULL;

    Graf->nr_neurons = num_neurons;
    Graf->nr_synapses = num_synapses;

    Graf->neurons = (Neuron*)malloc(num_neurons * sizeof(Neuron));
    if (!Graf->neurons) {
        free(Graf);
        return NULL;
    }

    for (uint64_t i = 0; i < num_neurons; i++) {
        Graf->neurons[i].id = i;
        Graf->neurons[i].voltage = 0.0f;
        Graf->neurons[i].fire_count = 0;
        Graf->neurons[i].limit = 1.0f; // Default Limit 
    }
    
    Graf->row_pointers = row_pointers;
    Graf->target_ids = target_ids;
    Graf->weights = weights;

    return Graf;
}

void free_graph(Graph* Graf) {
    if(!Graf) return;

    if (Graf->neurons) 
        free(Graf->neurons);
    if (Graf->row_pointers) 
        free(Graf->row_pointers);
    if (Graf->target_ids)
        free(Graf->target_ids);
    if (Graf->weights)
        free(Graf->weights);

    free(Graf);
}

void simulate_network_tick(Graph* Graf) {
    for (uint32_t i = 0; i < Graf->nr_neurons; i++) {
        Neuron* curr_neuron = &Graf->neurons[i];

        // Apply leak factor to decay lingering voltage
        curr_neuron->voltage *= 0.9f;

        if(curr_neuron->voltage < 0.0f) {
            curr_neuron->voltage = 0.0f;
        }

        if (curr_neuron->voltage >= curr_neuron->limit) {

            curr_neuron->fire_count++;
            curr_neuron->voltage = 0.0f;
        
            uint64_t start = Graf->row_pointers[i];
            uint64_t end = Graf->row_pointers[i+1];

            for (uint64_t e = start; e < end; e++) {
                Graf->neurons[Graf->target_ids[e]].voltage += Graf->weights[e];
            }
        }
    }
}