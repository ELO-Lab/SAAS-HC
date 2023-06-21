#ifndef _ES_ANT_HPP_
#define _ES_ANT_HPP_

#include <libcmaes/eo_matrix.h>

// Hyperparameters
extern bool es_ant_flag;
extern double par_a, par_b, par_c;
extern size_t min_num_ants;

void es_ant_set_default(void);
void es_ant_init(void);
void es_ant_construct_and_local_search(void);
double make_ant_weight(size_t, size_t);

void repair_candidates(dMat &);

#endif