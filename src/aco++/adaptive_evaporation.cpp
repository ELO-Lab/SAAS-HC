#include <cmath>
#include <map>

#include "adaptive_evaporation.h"
#include "thop.h"
#include "ants.h"
#include "es_aco.h"
#include "es_ant.h"

#define PATH_ENTROPY_IDX 0
#define FITNESS_ENTROPY_IDX 1

// Hyperparameters
double init_rho, min_rho, max_rho;
unsigned int init_indv_ants;
double min_indv_ants, max_indv_ants;

std::size_t entropy_idx;
double entropies[2], min_entropies[2], max_entropies[2];
double fitness_entropy;

void init_adaptive_mechanism(void)
{
    if (verbose > 0)
        printf("\nrho=%f, indv_ants=%d ", rho, indv_ants);

    rho = init_rho;
    indv_ants = init_indv_ants;

    if (verbose > 0)
        printf("--> rho=%f, indv_ants=%d\n", rho, indv_ants);
}

template <class KeyT, class numericA, class numericB>
double compute_entropy(const std::map<KeyT, numericA> &occurence, const numericB &total_count)
{
    double entropy = 0;
    double p_ij;
    for (const auto &item : occurence)
    {
        p_ij = double(item.second) / total_count;
        entropy -= p_ij * log2(p_ij);
    }

    return entropy;
}

void calculate_path_entropy()
{
    std::map<std::pair<long int, long int>, long int> occurence;
    std::pair<long int, long int> edge;
    std::size_t i, j;
    long int total_edge_count = 0;

    for (i = 0; i < n_ants; i++)
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
                occurence[edge] = 0;
            occurence[edge] += 1;
            total_edge_count += 1;
        }
    }

    entropies[PATH_ENTROPY_IDX] = compute_entropy(occurence, total_edge_count);
    min_entropies[PATH_ENTROPY_IDX] = -log2(n_ants * 1.0 / total_edge_count);
    max_entropies[PATH_ENTROPY_IDX] = -log2(1.0 / total_edge_count);
}

void calculate_fitness_entropy()
{
    std::map<long int, long int> occurence;
    std::size_t i;

    for (i = 0; i < n_ants; i++)
    {
        long int ant_fitness = ant[i].fitness;
        if (occurence.find(ant_fitness) == occurence.end())
            occurence[ant_fitness] = 0;

        occurence[ant_fitness] += 1;
    }

    fitness_entropy = compute_entropy(occurence, n_ants);
    entropies[FITNESS_ENTROPY_IDX] = fitness_entropy;
    min_entropies[FITNESS_ENTROPY_IDX] = -log2(1.0);
    max_entropies[FITNESS_ENTROPY_IDX] = -log2(1.0 / n_ants);
}

void update_indv_ants()
{
    const double _mid_value = (max_indv_ants - min_indv_ants) * (entropies[entropy_idx] - min_entropies[entropy_idx]) / (max_entropies[entropy_idx] - min_entropies[entropy_idx]);
    if (indv_max_minus)
        indv_ants = (unsigned int)(max_indv_ants - _mid_value);
    else
        indv_ants = (unsigned int)(min_indv_ants + _mid_value);
}

void update_rho()
{
#if MIN_MAX_RHO_TUNING_MACRO
    const double sum_rho = left_rho + _mid_rho + right_rho;
    left_rho = left_rho / sum_rho * (max_max_rho - min_min_rho);
    _mid_rho = _mid_rho / sum_rho * (max_max_rho - min_min_rho);
    min_rho = min_min_rho + left_rho;
    max_rho = min_rho + _mid_rho;
#endif
    rho = min_rho + (max_rho - min_rho) * (entropies[entropy_idx] - min_entropies[entropy_idx]) / (max_entropies[entropy_idx] - min_entropies[entropy_idx]);
    // rho = max_rho - (max_rho - min_rho) * (entropies[entropy_idx] - min_entropies[entropy_idx]) / (max_entropies[entropy_idx] - min_entropies[entropy_idx]);
}

void adaptive_mechanism(void)
{
    calculate_path_entropy();
    calculate_fitness_entropy();
    entropy_idx = fitness_entropy_flag;

#if MIN_MAX_RHO_TUNING_MACRO
#else
    update_rho();
#endif

    if (cmaes_flag || ipopcmaes_flag || bipopcmaes_flag || es_ant_flag)
    {
        update_indv_ants();
        if (!es_ant_flag)
            resize_ant_colonies();
        ES_ACO_DEBUG(
            printf("\t\troute_entropy=%.4f ∈ [%.4f,%.4f], fitness_entropy=%.4f ∈ [%.4f,%.4f], rho=%.4f, indv_ants=%d\n",
                   entropies[PATH_ENTROPY_IDX], min_entropies[PATH_ENTROPY_IDX], max_entropies[PATH_ENTROPY_IDX],
                   entropies[FITNESS_ENTROPY_IDX], min_entropies[FITNESS_ENTROPY_IDX], max_entropies[FITNESS_ENTROPY_IDX],
                   rho, indv_ants);)
    }
}
