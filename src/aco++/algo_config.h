#ifndef _ALGO_CONFIG_H_
#define _ALGO_CONFIG_H_

#include "stdint.h"

// #define MIN_MAX_RHO_TUNING_MACRO true // have bugs // not implemeted
// #define LS_N2_MACRO true // have bugs
// #define Q_0_TUNING_MACRO true // not implemeted
// #define SEED_TUNING_MACRO true // not implemeted
// #define RHO_TUNING_MACRO true
// #define ES_ANT_MACRO true
// #define O1_EVAP_MACRO true
#define TREE_MAP_MACRO true
#define ADAPT_RHO_MACRO true
#define CMAES_MACRO true

extern int_fast8_t verbose;
extern bool node_clustering_flag;      // AACO-NC - Clustering cities
extern bool adaptive_evaporation_flag; // AACO-NC - Dynamic rho based on entropy
extern bool es_ant_flag;               // Online hyperparameter tuning
extern bool tree_map_flag;             // Ant run on binary tree
extern bool o1_evap_flag;              // Evaporating with time complexity O(C)
extern bool ls_n_square_flag;          // Consturct nearest neighbour list for local search in O(n^2)
extern bool cmaes_flag;
extern bool ipopcmaes_flag;
extern bool bipopcmaes_flag;
extern bool iGreedyLevyFlag;

extern bool random_initial_std;
extern bool fitness_entropy_flag;

#endif