#ifndef _CUSTOM_STRATEGY_HPP_
#define _CUSTOM_STRATEGY_HPP_

#include <libcmaes/cmaes.h>
#include <Eigen/Dense>
#include <array>

template <class TCovarianceUpdate, class TGenoPheno>
using STRATEGY = libcmaes::BIPOPCMAStrategy<TCovarianceUpdate, TGenoPheno>;
template <class TGenoPheno>
using PARAMETER = libcmaes::CMAParameters<TGenoPheno>;
using GENO_PHENO = libcmaes::GenoPheno<libcmaes::pwqBoundStrategy>;
using COVARIANCE_UPDATE = libcmaes::ACovarianceUpdate;

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

using OPTIMIZER = libcmaes::ESOptimizer<Custom_Strategy<COVARIANCE_UPDATE, GENO_PHENO>, PARAMETER<GENO_PHENO>>;
// #define OPTIMIZER libcmaes::ESOptimizer<Custom_Strategy<COVARIANCE_UPDATE, GENO_PHENO>, PARAMETER<GENO_PHENO>>
// #define OPTIMIZER libcmaes::ESOptimizer<Custom_Strategy<libcmaes::ACovarianceUpdate, libcmaes::GenoPheno<libcmaes::pwqBoundStrategy>>, libcmaes::CMAParameters<libcmaes::GenoPheno<libcmaes::pwqBoundStrategy>>>
extern OPTIMIZER *optim_ptr;

#endif