#include "algo_config.h"

int_fast8_t verbose = 1;

bool node_clustering_flag = false;

#if ADAPT_RHO_MACRO
bool adaptive_evaporation_flag = true;
#else
bool adaptive_evaporation_flag = false;
#endif

#if LS_N2_MACRO
bool ls_n_square_flag = true;
#else
bool ls_n_square_flag = false;
#endif

#if O1_EVAP_MACRO
bool o1_evap_flag = true;
#else
bool o1_evap_flag = false;
#endif

#if ES_ANT_MACRO
bool es_ant_flag = true;
#else
bool es_ant_flag = false;
#endif

#if TREE_MAP_MACRO
bool tree_map_flag = true;
#else
bool tree_map_flag = false;
#endif

#if CMAES_MACRO
bool cmaes_flag = true;
#else
bool cmaes_flag = false;
#endif

#if MIN_MAX_RHO_TUNING_MACRO
bool min_max_rho_tuning_flag = true;
#else
bool min_max_rho_tuning_flag = false;
#endif

bool indv_max_minus = true;
// bool indv_max_minus = false;
bool random_initial_std = true;
// bool random_initial_std = false;
bool fitness_entropy_flag = true;
// bool fitness_entropy_flag = false;
// replace_worst_by_bestever --> kh tốt :V đừng bật
bool replace_worst_by_bestever = false;

// config here don't work
bool ipopcmaes_flag = false;
bool bipopcmaes_flag = false;
bool iGreedyLevyFlag = false;
