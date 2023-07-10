#ifndef _ALGO_CONFIG_H_
#define _ALGO_CONFIG_H_

#include "stdint.h"

extern int_fast8_t verbose;
extern bool node_clustering_flag;      // AACO-NC - Clustering cities
extern bool adaptive_evaporation_flag; // AACO-NC - Dynamic rho based on entropy
extern bool es_ant_flag;               // Online hyperparameter tuning
extern bool tree_map_flag;             // Ant run on binary tree
extern bool o1_evap_flag;              // Evaporating with time complexity O(C)
extern bool ls_n_square_flag;          // Consturct nearest neighbour list for local search in O(n^2)

#endif