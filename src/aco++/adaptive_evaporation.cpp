#include <cmath>
#include <map>

#include "adaptive_evaporation.h"
#include "thop.h"
#include "ants.h"
#include "es_aco.h"
#include "es_ant.h"

// Hyperparameters
double min_rho;
double max_rho;
double min_indv_ants;
double max_indv_ants;

double entropy, min_entropy, max_entropy;

void count_ant_edges(std::map<std::pair<long int, long int>, long int> &occurence, long int &total_edge_count)
{
    occurence.clear();
    total_edge_count = 0;
    std::pair<long int, long int> edge;
    long int i, j;

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

void update_indv_ants()
{
    indv_ants = (unsigned int)(min_indv_ants + (max_indv_ants - min_indv_ants) * (entropy - min_entropy) / (max_entropy - min_entropy));
    // indv_ants =  (unsigned int)(max_indv_ants - (max_indv_ants - min_indv_ants) * (entropy - min_entropy) / (max_entropy - min_entropy));
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
    rho = min_rho + (max_rho - min_rho) * (entropy - min_entropy) / (max_entropy - min_entropy);
    // rho = max_rho - (max_rho - min_rho) * (entropy - min_entropy) / (max_entropy - min_entropy);
}

void update_with_entropy(void)
{
    std::map<std::pair<long int, long int>, long int> occurence;
    long int total_edge_count;

    count_ant_edges(occurence, total_edge_count);
    calculate_entropy(occurence, total_edge_count);

    min_entropy = -log2(n_ants * 1.0 / total_edge_count);
    max_entropy = -log2(1.0 / total_edge_count);

    if (cmaes_flag || ipopcmaes_flag || bipopcmaes_flag)
        update_indv_ants();
#if MIN_MAX_RHO_TUNING_MACRO
#else
    update_rho();
#endif
}
