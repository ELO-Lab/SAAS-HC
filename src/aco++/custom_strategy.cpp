#include "custom_strategy.hpp"

#define BIPOP_FIRST_OPTIMIZE 1
#define BIPOP_SECOND_OPTIMIZE 2

template <class TGenoPheno>
using eostrat = libcmaes::ESOStrategy<libcmaes::CMAParameters<TGenoPheno>, libcmaes::CMASolutions, libcmaes::CMAStopCriteria<TGenoPheno>>;

template <class TCovarianceUpdate, class TGenoPheno>
Custom_Strategy<TCovarianceUpdate, TGenoPheno>::Custom_Strategy(libcmaes::FitFunc &func,
                                                                PARAMETER<TGenoPheno> &parameters) : STRATEGY<TCovarianceUpdate, TGenoPheno>(func, parameters)
{
    this->bipop_for_loop();
    this->cma_in();
};
template <class TCovarianceUpdate, class TGenoPheno>
Custom_Strategy<TCovarianceUpdate, TGenoPheno>::~Custom_Strategy(){};

template <class TCovarianceUpdate, class TGenoPheno>
uint_fast64_t Custom_Strategy<TCovarianceUpdate, TGenoPheno>::get_lambda(void)
{
    return STRATEGY<TCovarianceUpdate, TGenoPheno>::_parameters._lambda;
}

template <class TCovarianceUpdate, class TGenoPheno>
void Custom_Strategy<TCovarianceUpdate, TGenoPheno>::bipop_for_loop(void)
{
    while (this->bipop_r < libcmaes::CMAStrategy<TCovarianceUpdate, TGenoPheno>::_parameters._nrestarts)
    {
        while (this->bipop_budgets[0] > this->bipop_budgets[1])
        {
            STRATEGY<TCovarianceUpdate, TGenoPheno>::r2();
            libcmaes::CMAStrategy<TCovarianceUpdate, TGenoPheno>::_parameters.set_max_fevals(0.5 * this->bipop_budgets[0]);
            libcmaes::IPOPCMAStrategy<TCovarianceUpdate, TGenoPheno>::reset_search_state();
            this->bipop_case = BIPOP_FIRST_OPTIMIZE;
            return;
        }
        if (this->bipop_r > 0) // use lambda_def on first call.
        {
            STRATEGY<TCovarianceUpdate, TGenoPheno>::r1();
            libcmaes::IPOPCMAStrategy<TCovarianceUpdate, TGenoPheno>::reset_search_state();
        }
        libcmaes::CMAStrategy<TCovarianceUpdate, TGenoPheno>::_parameters.set_max_fevals(STRATEGY<TCovarianceUpdate, TGenoPheno>::_max_fevals); // resets the budget
        this->bipop_case = BIPOP_SECOND_OPTIMIZE;
        return;
    }
    assert(false);
}

template <class TCovarianceUpdate, class TGenoPheno>
void Custom_Strategy<TCovarianceUpdate, TGenoPheno>::cma_in(void)
{
    if (eostrat<TGenoPheno>::_initial_elitist || eostrat<TGenoPheno>::_parameters._initial_elitist || eostrat<TGenoPheno>::_parameters._elitist || eostrat<TGenoPheno>::_parameters._initial_fvalue)
    {
        eostrat<TGenoPheno>::_solutions._initial_candidate = Candidate(eostrat<TGenoPheno>::_func(eostrat<TGenoPheno>::_parameters._gp.pheno(eostrat<TGenoPheno>::_solutions._xmean).data(), eostrat<TGenoPheno>::_parameters._dim),
                                                                       eostrat<TGenoPheno>::_solutions._xmean);
        eostrat<TGenoPheno>::_solutions._best_seen_candidate = eostrat<TGenoPheno>::_solutions._initial_candidate;
        this->update_fevals(1);
    }
}

template <class TCovarianceUpdate, class TGenoPheno>
void Custom_Strategy<TCovarianceUpdate, TGenoPheno>::cma_out(void)
{
    if (eostrat<TGenoPheno>::_parameters._with_edm)
        eostrat<TGenoPheno>::edm();

    // test on final value wrt. to best candidate value and number of iterations in between.
    if (eostrat<TGenoPheno>::_parameters._initial_elitist_on_restart)
    {
        if (eostrat<TGenoPheno>::_parameters._initial_elitist_on_restart && eostrat<TGenoPheno>::_solutions._best_seen_candidate.get_fvalue() < eostrat<TGenoPheno>::_solutions.best_candidate().get_fvalue() && eostrat<TGenoPheno>::_niter - eostrat<TGenoPheno>::_solutions._best_seen_iter >= 3) // elitist
        {
            LOG_IF(libcmaes::INFO, !eostrat<TGenoPheno>::_parameters._quiet) << "Starting elitist on restart: bfvalue=" << eostrat<TGenoPheno>::_solutions._best_seen_candidate.get_fvalue() << " / biter=" << eostrat<TGenoPheno>::_solutions._best_seen_iter << std::endl;
            this->set_initial_elitist(true);

            // reinit solution and re-optimize.
            eostrat<TGenoPheno>::_parameters.set_x0(eostrat<TGenoPheno>::_solutions._best_seen_candidate.get_x_dvec_ref());
            eostrat<TGenoPheno>::_solutions = CMASolutions(eostrat<TGenoPheno>::_parameters);
            eostrat<TGenoPheno>::_solutions._nevals = eostrat<TGenoPheno>::_nevals;
            eostrat<TGenoPheno>::_niter = 0;
            this->cma_in();
        }
    }

    assert(eostrat<TGenoPheno>::_solutions._run_status >= 0);
}

template <class TCovarianceUpdate, class TGenoPheno>
void Custom_Strategy<TCovarianceUpdate, TGenoPheno>::prepare(void)
{
    if (!this->stop())
    {
        return;
    }
    switch (bipop_case)
    {
    case BIPOP_FIRST_OPTIMIZE:
        this->cma_out();
        this->bipop_budgets[1] += libcmaes::CMAStrategy<TCovarianceUpdate, TGenoPheno>::_solutions._niter * libcmaes::CMAStrategy<TCovarianceUpdate, TGenoPheno>::_parameters._lambda;
        libcmaes::IPOPCMAStrategy<TCovarianceUpdate, TGenoPheno>::capture_best_solution(this->bipop_best_run);
        this->bipop_for_loop();
        this->cma_in();
        break;
    case BIPOP_SECOND_OPTIMIZE:
        this->cma_out();
        this->bipop_budgets[0] += libcmaes::CMAStrategy<TCovarianceUpdate, TGenoPheno>::_solutions._niter * libcmaes::CMAStrategy<TCovarianceUpdate, TGenoPheno>::_parameters._lambda;
        libcmaes::IPOPCMAStrategy<TCovarianceUpdate, TGenoPheno>::capture_best_solution(this->bipop_best_run);
        this->bipop_r++;
        this->bipop_for_loop();
        this->cma_in();
        break;
    default:
        assert(false);
    }
}

template <class TCovarianceUpdate, class TGenoPheno>
void Custom_Strategy<TCovarianceUpdate, TGenoPheno>::generation_run(void)
{
    const std::chrono::time_point<std::chrono::system_clock> tstart = std::chrono::system_clock::now();

    const auto candidates = this->ask();
    const auto phenocandidates = eostrat<TGenoPheno>::_parameters._gp.pheno(candidates);
    this->eval(candidates, phenocandidates);
    this->tell();
    eostrat<TGenoPheno>::inc_iter();

    const std::chrono::time_point<std::chrono::system_clock> tstop = std::chrono::system_clock::now();
    eostrat<TGenoPheno>::_solutions._elapsed_last_iter = std::chrono::duration_cast<std::chrono::milliseconds>(tstop - tstart).count();
    this->prepare();
}