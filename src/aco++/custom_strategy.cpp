#include <chrono>
#include <iostream>
#include <assert.h>

#include <libcmaes/cmaes.h>

#include "custom_strategy.hpp"
#include "es_ant.hpp"

#define BIPOP_FIRST_OPTIMIZE 1
#define BIPOP_SECOND_OPTIMIZE 2
#define BIPOP_STOPPED 3

template <class TGenoPheno>
using eostrat = libcmaes::ESOStrategy<libcmaes::CMAParameters<TGenoPheno>, libcmaes::CMASolutions, libcmaes::CMAStopCriteria<TGenoPheno>>;

template <class TCovarianceUpdate, class TGenoPheno>
libcmaes::Custom_Strategy<TCovarianceUpdate, TGenoPheno>::Custom_Strategy(libcmaes::FitFunc &func,
                                                                          PARAMETER<TGenoPheno> &parameters) : STRATEGY<TCovarianceUpdate, TGenoPheno>(func, parameters)
{
    this->bipop_for_loop();
    this->cma_in();
};
template <class TCovarianceUpdate, class TGenoPheno>
libcmaes::Custom_Strategy<TCovarianceUpdate, TGenoPheno>::~Custom_Strategy(){};

template <class TCovarianceUpdate, class TGenoPheno>
uint_fast64_t libcmaes::Custom_Strategy<TCovarianceUpdate, TGenoPheno>::get_lambda(void)
{
    return this->STRATEGY<TCovarianceUpdate, TGenoPheno>::_parameters.lambda();
}

template <class TCovarianceUpdate, class TGenoPheno>
void libcmaes::Custom_Strategy<TCovarianceUpdate, TGenoPheno>::bipop_for_loop(void)
{
    while (this->bipop_r < this->libcmaes::CMAStrategy<TCovarianceUpdate, TGenoPheno>::_parameters._nrestarts)
    {
        while (this->bipop_budgets[0] > this->bipop_budgets[1])
        {
            this->STRATEGY<TCovarianceUpdate, TGenoPheno>::r2();
            this->libcmaes::CMAStrategy<TCovarianceUpdate, TGenoPheno>::_parameters.set_max_fevals(0.5 * this->bipop_budgets[0]);
            this->libcmaes::IPOPCMAStrategy<TCovarianceUpdate, TGenoPheno>::reset_search_state();
            this->bipop_case = BIPOP_FIRST_OPTIMIZE;
            return;
        }
        if (this->bipop_r > 0) // use lambda_def on first call.
        {
            this->STRATEGY<TCovarianceUpdate, TGenoPheno>::r1();
            this->libcmaes::IPOPCMAStrategy<TCovarianceUpdate, TGenoPheno>::reset_search_state();
        }
        this->libcmaes::CMAStrategy<TCovarianceUpdate, TGenoPheno>::_parameters.set_max_fevals(this->STRATEGY<TCovarianceUpdate, TGenoPheno>::_max_fevals); // resets the budget
        this->bipop_case = BIPOP_SECOND_OPTIMIZE;
        return;
    }
    this->bipop_case = BIPOP_STOPPED;
    return;
}

template <class TCovarianceUpdate, class TGenoPheno>
void libcmaes::Custom_Strategy<TCovarianceUpdate, TGenoPheno>::cma_in(void)
{
    if (this->eostrat<TGenoPheno>::_initial_elitist || this->eostrat<TGenoPheno>::_parameters._initial_elitist || this->eostrat<TGenoPheno>::_parameters._elitist || this->eostrat<TGenoPheno>::_parameters._initial_fvalue)
    {
        this->eostrat<TGenoPheno>::_solutions._initial_candidate = Candidate(this->eostrat<TGenoPheno>::_func(this->eostrat<TGenoPheno>::_parameters._gp.pheno(this->eostrat<TGenoPheno>::_solutions._xmean).data(), this->eostrat<TGenoPheno>::_parameters._dim),
                                                                             this->eostrat<TGenoPheno>::_solutions._xmean);
        this->eostrat<TGenoPheno>::_solutions._best_seen_candidate = this->eostrat<TGenoPheno>::_solutions._initial_candidate;
        this->update_fevals(1);
    }
}

template <class TCovarianceUpdate, class TGenoPheno>
void libcmaes::Custom_Strategy<TCovarianceUpdate, TGenoPheno>::cma_out(void)
{
    if (this->eostrat<TGenoPheno>::_parameters._with_edm)
        this->eostrat<TGenoPheno>::edm();

    // test on final value wrt. to best candidate value and number of iterations in between.
    if (this->eostrat<TGenoPheno>::_parameters._initial_elitist_on_restart)
    {
        if (this->eostrat<TGenoPheno>::_parameters._initial_elitist_on_restart && this->eostrat<TGenoPheno>::_solutions._best_seen_candidate.get_fvalue() < this->eostrat<TGenoPheno>::_solutions.best_candidate().get_fvalue() && this->eostrat<TGenoPheno>::_niter - this->eostrat<TGenoPheno>::_solutions._best_seen_iter >= 3) // elitist
        {
            LOG_IF(libcmaes::INFO, !this->eostrat<TGenoPheno>::_parameters._quiet) << "Starting elitist on restart: bfvalue=" << this->eostrat<TGenoPheno>::_solutions._best_seen_candidate.get_fvalue() << " / biter=" << this->eostrat<TGenoPheno>::_solutions._best_seen_iter << std::endl;
            this->set_initial_elitist(true);

            // reinit solution and re-optimize.
            this->eostrat<TGenoPheno>::_parameters.set_x0(this->eostrat<TGenoPheno>::_solutions._best_seen_candidate.get_x_dvec_ref());
            this->eostrat<TGenoPheno>::_solutions = CMASolutions(this->eostrat<TGenoPheno>::_parameters);
            this->eostrat<TGenoPheno>::_solutions._nevals = this->eostrat<TGenoPheno>::_nevals;
            this->eostrat<TGenoPheno>::_niter = 0;
            this->cma_in();
        }
    }
}

template <class TCovarianceUpdate, class TGenoPheno>
void libcmaes::Custom_Strategy<TCovarianceUpdate, TGenoPheno>::prepare(void)
{
    while (this->stop())
    {
        switch (bipop_case)
        {
        case BIPOP_FIRST_OPTIMIZE:
            this->cma_out();
            this->bipop_budgets[1] += libcmaes::CMAStrategy<TCovarianceUpdate, TGenoPheno>::_solutions._niter * libcmaes::CMAStrategy<TCovarianceUpdate, TGenoPheno>::_parameters._lambda;
            this->libcmaes::IPOPCMAStrategy<TCovarianceUpdate, TGenoPheno>::capture_best_solution(this->bipop_best_run);
            this->bipop_for_loop();
            this->cma_in();
            break;
        case BIPOP_SECOND_OPTIMIZE:
            this->cma_out();
            this->bipop_budgets[0] += libcmaes::CMAStrategy<TCovarianceUpdate, TGenoPheno>::_solutions._niter * libcmaes::CMAStrategy<TCovarianceUpdate, TGenoPheno>::_parameters._lambda;
            this->libcmaes::IPOPCMAStrategy<TCovarianceUpdate, TGenoPheno>::capture_best_solution(this->bipop_best_run);
            this->bipop_r++;
            this->bipop_for_loop();
            this->cma_in();
            break;
        default:
            assert(false);
        }
    }
}

template <class TCovarianceUpdate, class TGenoPheno>
void libcmaes::Custom_Strategy<TCovarianceUpdate, TGenoPheno>::generation_run(void)
{
    const std::chrono::time_point<std::chrono::system_clock> tstart = std::chrono::system_clock::now();

    auto candidates = this->ask();
    repair_candidates(candidates);
    const auto phenocandidates = this->eostrat<TGenoPheno>::_parameters.get_gp().pheno(candidates);

    this->eval(candidates, phenocandidates);
    if (termination_condition())
        return;

    this->tell();
    this->eostrat<TGenoPheno>::inc_iter();

    const std::chrono::time_point<std::chrono::system_clock> tstop = std::chrono::system_clock::now();
    this->eostrat<TGenoPheno>::_solutions._elapsed_last_iter = std::chrono::duration_cast<std::chrono::milliseconds>(tstop - tstart).count();
    this->prepare();
}