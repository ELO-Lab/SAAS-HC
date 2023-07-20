#ifndef _ADAPTIVE_EVAPORATION_H_
#define _ADAPTIVE_EVAPORATION_H_

// Hyperparameters
extern double init_rho, min_rho, max_rho;
extern unsigned int init_indv_ants;
extern double min_indv_ants, max_indv_ants;

extern double entropies, fitness_entropy;

void init_adaptive_mechanism(void);

void adaptive_mechanism(void);

#endif