#include <cmath>
#include <map>

#include "adaptive_evaporation.h"
#include "thop.h"
#include "ants.h"
#include "es_aco.h"
#include "es_ant.h"

// Hyperparameters
double min_rho = 0.01;
double max_rho = 0.99;

double min_indv_ants = 2;
double max_indv_ants = 50;

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

void calculate_entropy(double &entropy, const std::map<std::pair<long int, long int>, long int> &occurence, const long int &total_edge_count)
{
    entropy = 0;
    for (const auto &item : occurence)
    {
        const double p_ij = item.second * 1.0 / total_edge_count;
        entropy -= p_ij * log2(p_ij);
    }
}

void update_rho(void)
{

    double rho_diff = max_rho - min_rho;
    double indv_ants_diff = max_indv_ants - min_indv_ants;

    std::map<std::pair<long int, long int>, long int> occurence;
    long int total_edge_count;
    double entropy, min_entropy, max_entropy;

    count_ant_edges(occurence, total_edge_count);
    calculate_entropy(entropy, occurence, total_edge_count);

    min_entropy = -log2(n_ants * 1.0 / total_edge_count);
    max_entropy = -log2(1.0 / total_edge_count);

    rho = min_rho + rho_diff * (entropy - min_entropy) / (max_entropy - min_entropy);
    // rho = max_rho - rho_diff * (entropy - min_entropy) / (max_entropy - min_entropy);

    if (cmaes_flag || ipopcmaes_flag || bipopcmaes_flag)
    {
        indv_ants = (unsigned int)(min_indv_ants + indv_ants_diff * (entropy - min_entropy) / (max_entropy - min_entropy));
        // indv_ants =  (unsigned int)(max_indv_ants - indv_ants_diff * (entropy - min_entropy) / (max_entropy - min_entropy));

        resize_ant_colonies();
    }

#if ES_ANT_MACRO
    if (es_ant_flag)
    {
        n_ant_per_ind = min_indv_ants + indv_ants_diff * (entropy - min_entropy) / (max_entropy - min_entropy);
        // n_ant_per_ind =  max_indv_ants - indv_ants_diff * (entropy - min_entropy) / (max_entropy - min_entropy);
    }
#endif
}