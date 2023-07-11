#include "algo_config.h"

int_fast8_t verbose = 1;

bool node_clustering_flag = false;
bool adaptive_evaporation_flag = false;

// bool ls_n_square_flag = true;
bool ls_n_square_flag = false;

#ifdef O1_EVAP_MACRO
bool o1_evap_flag = true;
#else
bool o1_evap_flag = false;
#endif

#ifdef ES_ANT_MACRO
bool es_ant_flag = true;
#else
bool es_ant_flag = false;
#endif

#ifdef TREE_MAP_MACRO
bool tree_map_flag = true;
#else
bool tree_map_flag = false;
#endif

// not config here
int cmaes_flag;      // 0 or 1
int ipopcmaes_flag;  // 0 or 1
int bipopcmaes_flag; // 0 or 1
int iGreedyLevyFlag; // 0 or 1
