#include "algo_config.h"

int_fast8_t verbose = 1;

bool node_clustering_flag = false;
bool adaptive_evaporation_flag = false;

#ifdef TREE_MAP
bool tree_map_flag = true;
#else
bool tree_map_flag = false;
#endif

bool es_ant_flag = true;
// bool es_ant_flag = false;

// bool o1_evap_flag = true;
bool o1_evap_flag = false;

// bool ls_n_square_flag = true;
bool ls_n_square_flag = false;
