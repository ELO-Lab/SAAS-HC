#include <cmath>
#include <map>

#include "adaptive_evaporation.hpp"
#include "thop.h"
#include "ants.h"

// Hyperparameters
bool adaptive_evaporation_flag;
float min_rho = 0.01;
float max_rho = 0.5;

const float rho_diff = max_rho - min_rho;

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

void calculate_entropy(float &entropy, const std::map<std::pair<long int, long int>, long int> &occurence, const long int &total_edge_count)
{
    entropy = 0;
    for (const auto &item : occurence)
    {
        const float p_ij = item.second * 1.0 / total_edge_count;
        entropy -= p_ij * log2(p_ij);
    }
}

void update_rho(void)
{
    std::map<std::pair<long int, long int>, long int> occurence;
    long int total_edge_count;
    float entropy, min_entropy, max_entropy;

    count_ant_edges(occurence, total_edge_count);
    calculate_entropy(entropy, occurence, total_edge_count);

    min_entropy = -log2(n_ants * 1.0 / total_edge_count);
    max_entropy = -log2(1.0 / total_edge_count);

    // rho = min_rho + rho_diff * (entropy - min_entropy) / (max_entropy - min_entropy);
    rho = max_rho - rho_diff * (entropy - min_entropy) / (max_entropy - min_entropy);
}