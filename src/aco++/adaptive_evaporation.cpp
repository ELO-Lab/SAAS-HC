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

double entropy;
double fitness_entropy;

void init_adaptive_mechanism(void){
    printf("\nrho=%f, indv_ants=%d ", rho, indv_ants);
    rho = init_rho;
    indv_ants = init_indv_ants;
    printf("--> rho=%f, indv_ants=%d\n", rho, indv_ants);
}

void count_ant_edges(std::map<std::pair<long int, long int>, long int> &occurence, long int &total_edge_count)
{
    occurence.clear();
    total_edge_count = 0;
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
}

void calculate_entropy(const std::map<std::pair<long int, long int>, long int> &occurence, const long int &total_edge_count)
{
    entropy = 0;
    for (const auto &item : occurence)
    {
        const double p_ij = item.second * 1.0 / total_edge_count;
        entropy -= p_ij * log2(p_ij);
    }
}



void calculate_entropy_fitness(){
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

void update_rho(void)
{
    calculate_entropy_fitness();

    double rho_diff = max_rho - min_rho;
    double indv_ants_diff = max_indv_ants - min_indv_ants;

    std::map<std::pair<long int, long int>, long int> occurence;
    long int total_edge_count;
    double min_entropy, max_entropy;

    count_ant_edges(occurence, total_edge_count);
    calculate_entropy(occurence, total_edge_count);

    min_entropy = -log2(n_ants * 1.0 / total_edge_count);
    max_entropy = -log2(1.0 / total_edge_count);

    rho = min_rho + rho_diff * (entropy - min_entropy) / (max_entropy - min_entropy);
    // rho = max_rho - rho_diff * (entropy - min_entropy) / (max_entropy - min_entropy);

    if (cmaes_flag || ipopcmaes_flag || bipopcmaes_flag)
    {
        indv_ants = (unsigned int)(min_indv_ants + indv_ants_diff * (entropy - min_entropy) / (max_entropy - min_entropy));
        // indv_ants =  (unsigned int)(max_indv_ants - indv_ants_diff * (entropy - min_entropy) / (max_entropy - min_entropy));

#if ES_ANT_MACRO
#else
        resize_ant_colonies();
#endif
    }
}

void adaptive_mechanism(void){
    double rho_diff = max_rho - min_rho;
    double indv_ants_diff = max_indv_ants - min_indv_ants;

    double min_entropy, max_entropy;

    calculate_entropy_fitness();

    min_entropy = -log2(1.0);
    max_entropy = -log2(1.0 / ant.size());

    printf("\t\tfitness_entropy=%f,[%f, %f], ", fitness_entropy, min_entropy, max_entropy);
    rho = min_rho + rho_diff * (fitness_entropy - min_entropy) / (max_entropy - min_entropy);
    // rho = max_rho - rho_diff * (fitness_entropy - min_entropy) / (max_entropy - min_entropy);
    printf("rho=%f, ", rho);

    if (cmaes_flag || ipopcmaes_flag || bipopcmaes_flag)
    {
        indv_ants = (unsigned int)(min_indv_ants + indv_ants_diff * (fitness_entropy - min_entropy) / (max_entropy - min_entropy));
        // indv_ants =  (unsigned int)(max_indv_ants - indv_ants_diff * (fitness_entropy - min_entropy) / (max_entropy - min_entropy));
        printf("indv_ants=%d\n", indv_ants);
        resize_ant_colonies();
    }
}