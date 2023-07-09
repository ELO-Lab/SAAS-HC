#ifndef ES_ACO_H /* only include ones */ 
#define ES_ACO_H 

#include "boundary_cmaes.h"
#include "ants.h"
#include "utilities.h"
#include "inout.h"
#include "thop.h"
#include "timer.h"
#include "ls.h"

#define ALPHA_IDX 1
#define BETA_IDX  2
#define PAR_A_IDX 3
#define PAR_B_IDX 4
#define PAR_C_IDX 5

extern double lowerBounds[];
extern double upperBounds[];

extern int cmaes_flag;

extern boundary_cmaes optimizer;

void _es_construct_solutions(int index);

void _es_local_search(int index);

//   index of current offspring         genotype   number of dims
double eval_function (int index, double const *x, unsigned long N);

void es_aco_init();

void es_aco_construct_solutions();

void es_aco_export_result();

void es_aco_exit();

#endif