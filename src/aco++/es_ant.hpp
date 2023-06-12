#ifndef _ES_ANT_HPP_
#define _ES_ANT_HPP_

#define NODE_CLUSTERING_VERSION 0
#if NODE_CLUSTERING_VERSION == 1
#define ES_ANT_DIM 10
#endif
#if NODE_CLUSTERING_VERSION == 0
#define ES_ANT_DIM 5
#endif

#define SEED_MIN 0
#define SEED_MAX 2147483647

// Hyperparameters
extern double par_a, par_b, par_c, cluster_alpha, cluster_beta;
extern bool es_ant_flag;
// extern long int n_generation_each_iteration;

void es_ant_init(void);
void es_ant_construct_and_local_search(void);

#endif