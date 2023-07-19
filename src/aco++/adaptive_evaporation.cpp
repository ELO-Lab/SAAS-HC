#include <cmath>
#include <map>

#include "adaptive_evaporation.h"
#include "thop.h"
#include "ants.h"
#include "es_aco.h"
#include "es_ant.h"

// Hyperparameters
double init_rho;
unsigned int init_indv_ants;

double min_rho;
double max_rho;
double min_indv_ants;
double max_indv_ants;

double path_entropy;
double fitness_entropy;

long int total_edge_count;

void init_adaptive_mechanism(void){
    printf("\nrho=%f, indv_ants=%d ", rho, indv_ants);
    rho = init_rho;
    indv_ants = init_indv_ants;
    printf("--> rho=%f, indv_ants=%d\n", rho, indv_ants);
}

void calculate_path_entropy()
{
    std::map<std::pair<long int, long int>, long int> occurence;
    total_edge_count = 0;
    occurence.clear();
    std::pair<long int, long int> edge;
    long int i, j;

    for (i = 0; i < ant.size(); i++)
    {
        for (j = 0; j <= ant[i].tour_size - 3; j++)
        {
            // if (ant[i].tour[j] < ant[i].tour[j + 1]) {
            //     edge = std::make_pair(ant[i].tour[j], ant[i].tour[j + 1]);
            // } else {
            //     edge = std::make_pair(ant[i].tour[j+1], ant[i].tour[j]);
            // }
            edge = std::make_pair(ant[i].tour[j], ant[i].tour[j + 1]);

            if (occurence.find(edge) == occurence.end())
            {
                occurence[edge] = 0;
            }
            occurence[edge] += 1;
            total_edge_count += 1;
        }
    }

    path_entropy = 0;
    for (const auto &item : occurence)
    {
        const double p_ij = item.second * 1.0 / total_edge_count;
        path_entropy -= p_ij * log2(p_ij);
    }
}

void calculate_fitness_entropy(){
    fitness_entropy = 0;
    std::map<long int, long int> occurence;
    for (int i = 0; i < ant.size(); i++)
    {
        long int ant_fitness = ant[i].fitness;
        if (occurence.find(ant_fitness) == occurence.end())
            occurence[ant_fitness] = 0;

        occurence[ant_fitness] += 1;
    }
    
    for (const auto &item : occurence)
    {
        const double p_ij = item.second * 1.0 / ant.size();
        fitness_entropy -= p_ij * log2(p_ij);
    }
}

void adaptive_mechanism(void){
    double rho_diff = max_rho - min_rho;
    double indv_ants_diff = max_indv_ants - min_indv_ants;
    int idx;

    double min_path_entropy, max_path_entropy;
    calculate_path_entropy();
    min_path_entropy = -log2(n_ants * 1.0 / total_edge_count);
    max_path_entropy = -log2(1.0 / total_edge_count);

    double min_fitness_entropy, max_fitness_entropy;
    calculate_fitness_entropy();
    min_fitness_entropy = -log2(1.0);
    max_fitness_entropy = -log2(1.0 / ant.size());

    double entropy[2]       = {path_entropy,        fitness_entropy};
    double min_entropy[2]   = {min_path_entropy,    min_fitness_entropy};
    double max_entropy[2]   = {max_path_entropy,    max_fitness_entropy};
    // indx = 1 --> using path_entropy
    // indx = 2 --> using fitness_entropy

    idx = 1;

    rho = min_rho + rho_diff * (entropy[idx] - min_entropy[idx]) / (max_entropy[idx] - min_entropy[idx]);
    // rho = max_rho - rho_diff * (entropy[idx] - min_entropy[idx]) / (max_entropy[idx] - min_entropy[idx]);

    idx = 1;
    
    // indv_ants = (unsigned int)(min_indv_ants + indv_ants_diff * (entropy[idx] - min_entropy[idx]) / (max_entropy[idx] - min_entropy[idx]));
    indv_ants =  (unsigned int)(max_indv_ants - indv_ants_diff * (entropy[idx] - min_entropy[idx]) / (max_entropy[idx] - min_entropy[idx]));

    ES_ACO_DEBUG(
        printf("\t\troute_entropy=%.4f ∈ [%.4f,%.4f], fitness_entropy=%.4f ∈ [%.4f,%.4f], rho=%.4f, indv_ants=%d\n",
                path_entropy, min_path_entropy, max_path_entropy, fitness_entropy, min_fitness_entropy, max_fitness_entropy, rho, indv_ants);
    )
    resize_ant_colonies();
}

// void update_rho(void)
// {
//     calculate_entropy_fitness();

//     double rho_diff = max_rho - min_rho;
//     double indv_ants_diff = max_indv_ants - min_indv_ants;

//     std::map<std::pair<long int, long int>, long int> occurence;
//     long int total_edge_count;
//     double min_entropy, max_entropy;

//     count_ant_edges(occurence, total_edge_count);
//     calculate_entropy(occurence, total_edge_count);

//     min_entropy = -log2(n_ants * 1.0 / total_edge_count);
//     max_entropy = -log2(1.0 / total_edge_count);

//     rho = min_rho + rho_diff * (path_entropy - min_entropy) / (max_entropy - min_entropy);
//     // rho = max_rho - rho_diff * (path_entropy - min_entropy) / (max_entropy - min_entropy);

//     if (cmaes_flag || ipopcmaes_flag || bipopcmaes_flag)
//     {
//         adaptive_mechanism();
//         // indv_ants = (unsigned int)(min_indv_ants + indv_ants_diff * (path_entropy - min_entropy) / (max_entropy - min_entropy));
//         // indv_ants =  (unsigned int)(max_indv_ants - indv_ants_diff * (path_entropy - min_entropy) / (max_entropy - min_entropy));

// #if ES_ANT_MACRO
// #else
//         // resize_ant_colonies();
// #endif
//     }
// }