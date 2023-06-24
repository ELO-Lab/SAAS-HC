#include <functional>

#include "utilities.h"
#include "tree_map.hpp"

Tree_Map::Tree_Map(const std::size_t &num_city, long int **distance_matrix)
{
    size_t i;

    _tree_edge_ptrs.resize(num_city - 1);
    for (i = 0; i < num_city - 1; i++) // Do not go from the end city
        _tree_edge_ptrs[i] = new Tree_Edge(num_city, i, distance_matrix);

    _wont_visit_tree_ptr = new Wont_Visit_Tree(num_city);

    _global_wont_visit_restart_times = 0;
    _global_restart_times = 0;
    _global_evap_times = 0;
}

Tree_Map::~Tree_Map()
{
    delete _wont_visit_tree_ptr;
    for (auto &_ptr : _tree_edge_ptrs)
        delete _ptr;
}

void Tree_Map::_set_wont_visit(const std::size_t &city_index)
{
    if (city_index != 0)
        _wont_visit_tree_ptr->set_wont_visit(city_index, _global_wont_visit_restart_times);
}

void Tree_Map::reinforce(
    ant_struct &an_ant,
    const double &rho)
{
    std::size_t i;
    const double invert_fitness = 1.0 / an_ant.fitness;
    const double one_minus_rho = 1 - rho;

    for (i = 0; i <= an_ant.tour_size - 4; i++)
    {
        _tree_edge_ptrs[an_ant.tour[i]]->reinforce(
            an_ant.tour[i + 1],
            invert_fitness,
            one_minus_rho,
            _trail_restart,
            _trail_min,
            _trail_max,
            _global_restart_times,
            _global_evap_times);
    }
}

void Tree_Map::_reset_wont_visit()
{
    _global_wont_visit_restart_times += 1;
}

void Tree_Map::evaporate(const double &trail_min, const double &trail_max)
{
    _global_evap_times += 1;
    _trail_min = trail_min;
    _trail_max = trail_max;
}

void Tree_Map::restart_pheromone(const double &trail_restart)
{
    _global_restart_times += 1;
    _trail_restart = trail_restart;
}

void Tree_Map::find_route(
    ant_struct &an_ant,
    const size_t &num_city,
    std::function<double()> rand01,
    const double &q_0,
    const double &alpha,
    const double &beta,
    const double &rho,
    std::function<void(ant_struct *a)> ant_empty_memory,
    std::function<long int(long int *t, char *visited, long int t_size, char *p)> compute_fitness,
    long int &n_tours)
{
    size_t i;      /* counter variable */
    long int step; /* counter of the number of construction steps */
    const double &one_minus_q_0 = 1 - q_0;
    const double &one_minus_rho = 1 - rho;
    size_t current_city, next_city;

    _reset_wont_visit();
    ant_empty_memory(&an_ant);

    /* Place the ants at initial city 0 and set the final city as n-1 */
    an_ant.tour_size = 1;
    an_ant.tour[0] = 0;
    an_ant.visited[0] = TRUE;
    an_ant.visited[num_city + 1 - 1] = TRUE;

    step = 0;
    while (step < num_city + 1 - 2)
    {
        step++;
        current_city = an_ant.tour[an_ant.tour_size - 1];
        if (current_city == num_city + 1 - 2)
        { /* previous city is the last one */
            continue;
        }

        // neighbour_choose_and_move_to_next(&an_ant, step);
        // if (acs_flag)
        //     local_acs_pheromone_update(&an_ant, step);
        next_city = _tree_edge_ptrs[current_city]->find_next_city(
            _wont_visit_tree_ptr->get_root_ptr(),
            rand01,
            one_minus_q_0,
            alpha,
            beta,
            one_minus_rho,
            _trail_restart,
            _trail_min,
            _global_restart_times,
            _global_evap_times);
        an_ant.tour[an_ant.tour_size] = next_city;
        an_ant.visited[next_city] = TRUE;

        an_ant.tour_size++;
    }

    an_ant.tour[an_ant.tour_size++] = num_city + 1 - 1;
    an_ant.tour[an_ant.tour_size++] = an_ant.tour[0];
    for (i = an_ant.tour_size; i < num_city + 1; i++)
        an_ant.tour[i] = 0;
    an_ant.fitness = compute_fitness(an_ant.tour, an_ant.visited, an_ant.tour_size, an_ant.packing_plan);
    // if (acs_flag)
    //     local_acs_pheromone_update(&an_ant, an_ant.tour_size - 1);

    n_tours += 1;
}

// hyperparameters
bool tree_map_flag = true;

Tree_Map *tree_map;
double trail_restart;

void tree_map_init(Tree_Map *&tree_map, const std::size_t &num_city, long int **distance_matrix)
{
    tree_map = new Tree_Map(num_city, distance_matrix);
}
void tree_map_fixed_parameters(long int &mmas_flag)
{
    mmas_flag = true;
}