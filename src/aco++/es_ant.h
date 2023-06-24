#ifndef _ES_ANT_H_
#define _ES_ANT_H_

#include <libcmaes/eo_matrix.h>

// Hyperparameters
extern bool es_ant_flag;
extern double par_a_mean, par_b_mean, par_c_mean,
    par_a_stepsize, par_b_stepsize, par_c_stepsize,
    alpha_max, beta_max,
    alpha_mean, beta_mean, rho_mean,
    alpha_stepsize, beta_stepsize, rho_stepsize,
    q_0_mean, q_0_stepsize, rand_seed_stepsize;
extern size_t min_n_ants;

extern double par_a, par_b, par_c;

void es_ant_force_set_parameters(void);
void es_ant_init(void);
void es_ant_construct_and_local_search(void);
double edge_weight(size_t, size_t);
void repair_candidates(dMat &);
void es_ant_deallocate();

#endif