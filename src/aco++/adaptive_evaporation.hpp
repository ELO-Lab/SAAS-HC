#ifndef _ADAPTIVE_EVAPORATION_HPP_
#define _ADAPTIVE_EVAPORATION_HPP_

// Hyperparameters
extern bool adaptive_evaporation_flag;
extern float min_rho;
extern float max_rho;

void update_rho(void);
void alternative_global_evaporation(void);

#endif