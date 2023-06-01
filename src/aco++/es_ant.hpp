#ifndef _ADAPT_ANT_HPP_
#define _ADAPT_ANT_HPP_

#define ES_ANT_DIM 9
#define SEED_MIN 0
#define SEED_MAX 2147483647

// Hyperparameters
extern bool adapt_ant;
extern double cluster_alpha, cluster_alpha_step_size, cluster_beta,
    cluster_beta_step_size, par_a, par_a_step_size, par_b, par_b_step_size,
    par_c, par_c_step_size, rho_step_size, alpha_step_size, beta_step_size;
extern unsigned char n_generation_each_iteration;

void es_ant_init(void);
void es_construct_solution(void);

#endif