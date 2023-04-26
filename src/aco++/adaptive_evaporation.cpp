#include <cmath>
#include <map>

#include "adaptive_evaporation.hpp"
#include "thop.h"
#include "ants.h"

// Hyperparameters
bool adaptive_evaporation_flag = true;
double min_rho = 0;
double max_rho = 1;

long int num_edge;
double min_entropy;
double max_entropy;
double rho_diff_over_entropy_diff;

void init_adaptive_evaporation(void) {
    num_edge = instance.n - 2; // a route has maxium (instance.n-2) edges
    min_entropy = -log2(1.0 / num_edge);
    max_entropy = -log2(1.0 / (num_edge * n_ants));
    rho_diff_over_entropy_diff = (max_rho - min_rho) / (max_entropy - min_entropy);
}

void count_ant_edges(std::map<std::pair<long int, long int>, long int> &occurence)
{
    occurence.clear();
    std::pair<long int, long int> edge;
    long int i, j;

    for (i = 0; i < n_ants; i++)
    {
        for (j = 0; j < ant[i].tour_size - 1; j++)
        {
            if (ant[i].tour[j] == instance.n - 1 || ant[i].tour[j + 1] == instance.n - 1)
            {
                continue;
            }
            edge = std::make_pair(ant[i].tour[j], ant[i].tour[j + 1]);
            if (occurence.count(edge) == 0)
            {
                occurence[edge] = 0;
            }
            occurence[edge] += 1;
        }
    }
}

void calculate_entropy(double &entropy)
{
    std::map<std::pair<long int, long int>, long int> occurence;
    count_ant_edges(occurence);
    entropy = 0;
    for (const auto &item : occurence)
    {
        const double p_ij = item.second * 1.0 / (num_edge * n_ants);
        entropy -= p_ij * log2(p_ij);
    }
}

void update_rho(void)
{
    double entropy;
    calculate_entropy(entropy);
    rho = min_rho + rho_diff_over_entropy_diff * (entropy - min_entropy);
}

void _evaporation(void)
/*
      FUNCTION:      implements the adaptive pheromone trail evaporation
      INPUT:         none
      OUTPUT:        none
      (SIDE)EFFECTS: pheromones are reduced by factor rho
 */
{
    long int i, j;
    const double evaporation_factor = 1 - rho;

    for (i = 0; i <= instance.n - 1; i++)
    {
        for (j = i + 1; j <= instance.n - 1; j++)
        {
            pheromone[i][j] = evaporation_factor * pheromone[i][j];
            pheromone[j][i] = evaporation_factor * pheromone[j][i];
        }
    }
}