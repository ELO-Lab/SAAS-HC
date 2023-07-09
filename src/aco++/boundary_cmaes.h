#include <stdio.h>
#include <stddef.h> /* NULL */
#include <stdlib.h> /* free() */
#include <float.h> /* DBL_MAX */
#include <functional>

#include "cmaes_interface.h"
#include "boundary_transformation.h"
#include "utilities.h"

class boundary_cmaes {
private:
    cmaes_t evo; 
    cmaes_boundary_transformation_t boundaries;
    std::function<double(int , double const * , unsigned long)> eval_function;

    int is_feasible(double const *x, unsigned long N) {
        N = (long) x[0]; 
        return 1;
    }

public:
    unsigned long dimension;
    double *arFunvals, *x_in_bounds, *const*pop;

    boundary_cmaes(){}

    void init(std::function<double(int , double const * , unsigned long)> eval_function, 
                const double *lowerBounds, const double *upperBounds){
        this->eval_function = eval_function;

        arFunvals = cmaes_init(&evo, 0, NULL, NULL, seed, 0, "cmaes_initials.par");
        dimension = (unsigned long)cmaes_Get(&evo, "dimension");
        
        const int nb_bounds = dimension;
        cmaes_boundary_transformation_init(&boundaries, lowerBounds, upperBounds, nb_bounds);

        printf("%s\n", cmaes_SayHello(&evo));

        x_in_bounds = cmaes_NewDouble(dimension); 
        cmaes_ReadSignals(&evo, "cmaes_signals.par");  
    }

    void run_a_generation(){
        int i;
        pop = cmaes_SamplePopulation(&evo);

        for (i = 0; i < cmaes_Get(&evo, "lambda"); ++i)
        {
            cmaes_boundary_transformation(&boundaries, pop[i], x_in_bounds, dimension);

            while (!is_feasible(x_in_bounds, dimension))
            {
                cmaes_ReSampleSingle(&evo, i);
                cmaes_boundary_transformation(&boundaries, pop[i], x_in_bounds, dimension);
            }
            arFunvals[i] = eval_function(i, x_in_bounds, dimension);
        }

        cmaes_UpdateDistribution(&evo, arFunvals);

        cmaes_ReadSignals(&evo, "cmaes_signals.par");
        fflush(stdout);
    }

    void end(){
        int i;
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

    void boundary_cmaes_exit()
    {
        cmaes_exit(&evo);
        cmaes_boundary_transformation_exit(&boundaries);
        free(x_in_bounds);
    }

    bool termination_condition(){
        if (cmaes_TestForTermination(&evo)) return true;
        return false;
    }
};