#ifndef _ADAPTIVE_EVAPORATION_H_
#define _ADAPTIVE_EVAPORATION_H_

// Hyperparameters
extern double init_rho, min_rho, max_rho;
extern unsigned int init_indv_ants;
extern double min_indv_ants, max_indv_ants;
extern double left_rho, _mid_rho, right_rho;
extern double min_min_rho, max_max_rho;

extern double fitness_entropy;

void init_adaptive_mechanism(void);

void adaptive_mechanism(void);

void update_rho();

#endif