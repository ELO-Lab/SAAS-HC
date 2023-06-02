#ifndef _ADAPT_ANT_HPP_
#define _ADAPT_ANT_HPP_

#define NODE_CLUSTERING_VERSION 0
#if NODE_CLUSTERING_VERSION
#define ES_ANT_DIM 10
#endif
#if 1 - NODE_CLUSTERING_VERSION
#define ES_ANT_DIM 5
#endif

#define SEED_MIN 0
#define SEED_MAX 2147483647

// Hyperparameters
extern double
    seed_stepsize_init,
    par_a, par_a_stepsize_init,
    par_b, par_b_stepsize_init,
    par_c, par_c_stepsize_init,
    cluster_alpha, cluster_alpha_stepsize_init,
    cluster_beta, cluster_beta_stepsize_init,
    alpha_stepsize_init, beta_stepsize_init,
    rho_stepsize_init, q_0_stepsize_init;
extern bool adapt_ant;
extern unsigned char n_generation_each_iteration, cmaes_algo;

void es_ant_init(void);
void es_construct_solution(void);

#endif