#ifndef _CUSTOM_STRATEGY_HPP_
#define _CUSTOM_STRATEGY_HPP_

#include <libcmaes/bipopcmastrategy.h>
#include <libcmaes/cmaes.h>
#include <array>

#define STRATEGY libcmaes::BIPOPCMAStrategy
#define PARAMETER libcmaes::CMAParameters
#define GENO_PHENO libcmaes::GenoPheno<libcmaes::NoBoundStrategy>
#define COVARIANCE_UPDATE libcmaes::ACovarianceUpdate
#define OPTIMIZER libcmaes::ESOptimizer<libcmaes::Custom_Strategy<COVARIANCE_UPDATE, GENO_PHENO>, PARAMETER<GENO_PHENO>>

namespace libcmaes
{
    template <class TCovarianceUpdate, class TGenoPheno>
    class Custom_Strategy : public STRATEGY<TCovarianceUpdate, TGenoPheno>
    {
    public:
        Custom_Strategy(libcmaes::FitFunc &func, PARAMETER<TGenoPheno> &parameters);
        ~Custom_Strategy();
        uint_fast64_t get_lambda(void);
        void generation_run(void);

    private:
        uint_fast8_t bipop_case;
        void prepare(void);
        void bipop_for_loop(void);
        void cma_in(void);
        void cma_out(void);

        // bipop local variables
        std::array<int, 2> bipop_budgets = {{0, 0}}; // 0: r1, 1: r2
        libcmaes::CMASolutions bipop_best_run;
        int_fast32_t bipop_r = 0;
    };
}

extern OPTIMIZER *optim_ptr;
#endif