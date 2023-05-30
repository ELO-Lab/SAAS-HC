#ifndef _ADAPT_ANT_HPP_
#define _ADAPT_ANT_HPP_

#define ES_ANT_DIM 9
#define SEED_MIN 0
#define SEED_MAX 2147483647

// Hyperparameters
extern bool adapt_ant;
extern double
    cluster_alpha,
    cluster_alpha_std,
    cluster_beta, cluster_beta_std,
    par_a, par_a_std,
    par_b, par_b_std,
    par_c, par_c_std,
    rho_std, alpha_std, beta_std;

void es_ant_init(void);
void es_construct_solution(void);

#endif