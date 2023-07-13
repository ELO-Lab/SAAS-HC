#include "algo_config.h"
#if ES_ANT_MACRO

#ifndef _ES_ANT_H_
#define _ES_ANT_H_

#include <libcmaes/eo_matrix.h>

// Hyperparameters
extern double rand_seed_std;
extern std::size_t min_n_ants;

void es_ant_set_default_hyperparameters(void);
void es_ant_init(void);
void es_ant_construct_and_local_search(void);
void repair_candidates(dMat &);
void es_ant_deallocate();

#endif
#endif