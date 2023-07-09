#include <stdio.h>
#include <stddef.h> /* NULL */
#include <stdlib.h> /* free() */
#include <float.h> /* DBL_MAX */
#include <functional>

#include "cmaes_interface.h"
#include "boundary_transformation.h"

#define ALPHA_IDX 1
#define BETA_IDX  2
#define PAR_A_IDX 3
#define PAR_B_IDX 4
#define PAR_C_IDX 5

class bounded_cmaes {
    public:
        cmaes_t evo; 
        cmaes_boundary_transformation_t boundaries;
        std::function<double<double const *x, unsigned long N> eval_function;

        unsigned long dimension;
        double *arFunvals, *x_in_bounds, *const*pop;

        //                      alpha   beta par_a par_b par_c
        double lowerBounds[] = { 0.0f,  0.0f, 0.0f, 0.0f, 0.0f}; 
        double upperBounds[] = {10.0f, 10.0f, 1.0f, 1.0f, 1.0f};

    bounded_cmaes(){}

    void init(std::function<double<double const *x, unsigned long N> eval_function){
        this->eval_function = eval_function;

        arFunvals = cmaes_init(&evo, 0, NULL, NULL, 0, 0, "cmaes_params.par");
        dimension = (unsigned long)cmaes_Get(&evo, "dimension");
        
        const int nb_bounds = dimension;
        cmaes_boundary_transformation_init(&boundaries, lowerBounds, upperBounds, nb_bounds);

        printf("%s\n", cmaes_SayHello(&evo));

        x_in_bounds = cmaes_NewDouble(dimension); 
        cmaes_ReadSignals(&evo, "cmaes_signals.par");  
    }

    void run_a_generation(){
        pop = cmaes_SamplePopulation(&evo);

        for (i = 0; i < cmaes_Get(&evo, "lambda"); ++i)
        {
            cmaes_boundary_transformation(&boundaries, pop[i], x_in_bounds, dimension);

            while (!is_feasible(x_in_bounds, dimension))
            {
                cmaes_ReSampleSingle(&evo, i);
                cmaes_boundary_transformation(&boundaries, pop[i], x_in_bounds, dimension);
            }
            arFunvals[i] = eval_function(x_in_bounds, dimension);
        }

        cmaes_UpdateDistribution(&evo, arFunvals);

        cmaes_ReadSignals(&evo, "cmaes_signals.par");
        fflush(stdout);
    }

    void end(){
        printf("Stop:\n%s\n", cmaes_TestForTermination(&evo));
        cmaes_WriteToFile(&evo, "all", "cmaes_data/allcmaes.dat");

        cmaes_boundary_transformation(&boundaries,
                                      (double const *)cmaes_GetPtr(&evo, "xmean"),
                                      x_in_bounds, dimension);

        for (i = 0; i < dimension; i++)
        {
            printf("%.3f, ", x_in_bounds[i]);
        }
    }

    double get(const char *keyword){
        return cmaes_Get(&evo, keyword);
    }

    ~bounded_cmaes()
    {
        cmaes_exit(&evo);
        cmaes_boundary_transformation_exit(&boundaries);
        free(x_in_bounds);
    }
}