#ifndef _ES_ANT_HPP_
#define _ES_ANT_HPP_

// Hyperparameters
extern double par_a, par_b, par_c, cluster_alpha, cluster_beta;
extern bool es_ant_flag;
// extern long int n_generation_each_iteration;

void es_ant_init(void);
void es_ant_construct_and_local_search(void);
double make_ant_weight(size_t, size_t);

#endif