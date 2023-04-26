#ifndef _ADAPTIVE_EVAPORATION_HPP_
#define _ADAPTIVE_EVAPORATION_HPP_

// Hyperparameters
extern bool adaptive_evaporation_flag;
extern double min_rho;
extern double max_rho;

void init_adaptive_evaporation(void);
void update_rho(void);
void _evaporation(void);

#endif