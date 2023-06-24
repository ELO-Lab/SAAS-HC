#ifndef _ADAPTIVE_EVAPORATION_H_
#define _ADAPTIVE_EVAPORATION_H_

// Hyperparameters
extern bool adaptive_evaporation_flag;
extern float min_rho;
extern float max_rho;

void update_rho(void);

#endif