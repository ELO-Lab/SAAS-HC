#ifndef _ADAPTIVE_EVAPORATION_H_
#define _ADAPTIVE_EVAPORATION_H_

// Hyperparameters
extern double min_rho, max_rho, min_indv_ants, max_indv_ants, init_rho, entropy, fitness_entropy;
extern unsigned int init_indv_ants;

// void update_rho(void);

void init_adaptive_mechanism(void);

void adaptive_mechanism(void);

#endif