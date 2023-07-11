#include "algo_config.h"
#if TREE_MAP_MACRO

#include <math.h>
#include <assert.h>

#include "utilities.h"
#include "nodes.h"
#include "thop.h"
#include "ants.h"

Node::Node(const bool &is_leaf)
    : Node_Base<Node>(nullptr, is_leaf) {}

Node::Node(Node *parent_ptr, const double &heuristic, const std::size_t &n_child_leaf, const bool &is_leaf)
    : Node_Base<Node>(parent_ptr, is_leaf)
{
    _local_evap_times = 0;
    _local_restart_times = 0;
    _heuristic = heuristic;
    _n_child_leaf = n_child_leaf;
}

Node::Node(Node *left_ptr, Node *right_ptr, const bool &is_root)
    : Node_Base<Node>(left_ptr, right_ptr, is_root)
{
    const bool is_leaf = (left_ptr == nullptr) and (right_ptr == nullptr);

    _n_child_leaf = 1;
    _local_evap_times = 0;
    _local_restart_times = 0;
    if (!is_leaf)
        _heuristic = (left_ptr->_heuristic + right_ptr->_heuristic) / 2.0;
}

Leaf::Leaf(Node *parent_ptr, const double &heuristic, const std::size_t &city_index)
    : Node(parent_ptr, heuristic, 1, true), Leaf_Base(city_index) {}

Leaf::Leaf(
    const std::size_t &city_index,
    const double &heuristic)
    : Node(nullptr, nullptr, false), Leaf_Base(city_index)
{
    _heuristic = heuristic;
}

Wont_Visit_Node::Wont_Visit_Node(Wont_Visit_Node *parent_ptr, const bool &is_leaf)
    : Node_Base<Wont_Visit_Node>(parent_ptr, is_leaf)
{
    _wont_visit = false;
    _local_wont_visit_restart_times = 0;
}

Wont_Visit_Node::Wont_Visit_Node(Wont_Visit_Node *left_ptr, Wont_Visit_Node *right_ptr, const bool &is_root)
    : Node_Base<Wont_Visit_Node>(left_ptr, right_ptr, is_root)
{
    _wont_visit = false;
    _local_wont_visit_restart_times = 0;
}

Building_Node::Building_Node(const bool &is_leaf)
    : Node_Base<Building_Node>(nullptr, is_leaf) {}

Building_Node::Building_Node(Building_Node *parent_ptr, const double &centroid_x, const double &centroid_y, const std::size_t &n_child_leaf, const bool &is_leaf)
    : Node_Base<Building_Node>(parent_ptr, is_leaf)
{
    this->_centroid_x = centroid_x;
    this->_centroid_y = centroid_y;
    _n_child_leaf = n_child_leaf;
}

Building_Leaf::Building_Leaf(Building_Node *parent_ptr, const double &centroid_x, const double &centroid_y, const std::size_t &city_index)
    : Building_Node(parent_ptr, centroid_x, centroid_y, 1, true), Leaf_Base(city_index) {}

void Node::restart_if_needed(const std::size_t &global_restart_times, const double &past_trail_restart)
{
    if (_local_restart_times < global_restart_times)
    {
        _local_restart_times = global_restart_times;
        _local_evap_times = 0;
        _past_pheromone = past_trail_restart;
    }
}

double Node::get_pheromone(const double &one_minus_rho,
                           const double &past_trail_restart,
                           const double &past_trail_min,
                           const std::size_t &global_restart_times, const std::size_t &global_evap_times)
{
    restart_if_needed(global_restart_times, past_trail_restart);
    double pheromone = _past_pheromone * pow(one_minus_rho, global_evap_times - _local_evap_times);
    if (pheromone < past_trail_min)
        pheromone = past_trail_min;

    return pheromone;
}

void Node::pay_evaporation_debt(const double &one_minus_rho,
                                const double &past_trail_restart,
                                const double &past_trail_min,
                                const std::size_t &global_restart_times, const std::size_t &global_evap_times)
{

    _past_pheromone = get_pheromone(one_minus_rho, past_trail_restart, past_trail_min, global_restart_times, global_evap_times);
    _local_evap_times = global_evap_times;
}

void Node::reinforce(const double &invert_fitness, const double &one_minus_rho,
                     const double &past_trail_restart,
                     const double &past_trail_min,
                     const double &trail_max,
                     const std::size_t &global_restart_times, const std::size_t &global_evap_times)
{
    pay_evaporation_debt(one_minus_rho, past_trail_restart, past_trail_min, global_restart_times, global_evap_times);
    _past_pheromone += invert_fitness;
    // _past_pheromone += invert_fitness / _n_child_leaf;
    if (_past_pheromone > trail_max)
        _past_pheromone = trail_max;
}

double Node::prob_weight(const double &alpha, const double &beta, const double &one_minus_rho,
                         const double &past_trail_restart,
                         const double &past_trail_min,
                         const std::size_t &global_restart_times, const std::size_t &global_evap_times)
{
    double weight;

    weight = prob_weight_without_child_leaf(
        alpha, beta, one_minus_rho,
        past_trail_restart,
        past_trail_min,
        global_restart_times, global_evap_times);
    weight *= _n_child_leaf;

    return weight;
}

double Node::prob_weight_without_child_leaf(
    const double &alpha, const double &beta, const double &one_minus_rho,
    const double &past_trail_restart,
    const double &past_trail_min,
    const std::size_t &global_restart_times, const std::size_t &global_evap_times)
{
    const double pheromone = this->get_pheromone(one_minus_rho, past_trail_restart, past_trail_min, global_restart_times, global_evap_times);

    return pow(pheromone, alpha) * pow(_heuristic, beta);
}

std::size_t Node::choose_child_with_prob(
    const double &alpha, const double &beta, const double &one_minus_rho,
    const double &past_trail_restart,
    const double &past_trail_min,
    const std::size_t &global_restart_times,
    const std::size_t &global_evap_times,
    const double &elite_prob)
{
    // IMPORTANCE NOTE: Remember to check won't visit before go to this function

    std::array<double, 2> weights;
    std::size_t i;
    double move_prob;

    for (i = 0; i < 2; i++)
        weights[i] = child_ptrs[i]->prob_weight_without_child_leaf(
            alpha, beta, one_minus_rho,
            past_trail_restart, past_trail_min,
            global_restart_times, global_evap_times);

    if (elite_prob != 0.0 && new_rand01() < elite_prob)
    {
        if (weights[1] > weights[0])
            return 1;
        else
            return 0;
    }

    for (i = 0; i < 2; i++)
        // weights[i] = child_ptrs[i]->prob_weight(alpha, beta, one_minus_rho, past_trail_restart, past_trail_min, global_restart_times, global_evap_times);
        weights[i] *= child_ptrs[i]->get_n_child_leaf();

    move_prob = weights[1] / (weights[0] + weights[1]);
    if (new_rand01() < move_prob)
        return 1;
    else
        return 0;
}

std::size_t Node::get_n_child_leaf() { return _n_child_leaf; }

void Wont_Visit_Node::_restart_if_needed(const std::size_t &global_wont_visit_restart_times)
{
    if (_local_wont_visit_restart_times < global_wont_visit_restart_times)
    {
        _wont_visit = false;
        _local_wont_visit_restart_times = global_wont_visit_restart_times;
    }
}

void Wont_Visit_Node::set_wont_visit(const std::size_t &global_wont_visit_restart_times)
{
    _wont_visit = true;
    _local_wont_visit_restart_times = global_wont_visit_restart_times;
    if (parent_ptr != nullptr)
        parent_ptr->_check_wont_visit(global_wont_visit_restart_times);
}

void Wont_Visit_Node::_check_wont_visit(const std::size_t &global_wont_visit_restart_times)
{
    if (child_ptrs[0]->get_wont_visit(global_wont_visit_restart_times) and child_ptrs[1]->get_wont_visit(global_wont_visit_restart_times))
        set_wont_visit(global_wont_visit_restart_times);
}

bool Wont_Visit_Node::get_wont_visit(const std::size_t &global_wont_visit_restart_times)
{
    _restart_if_needed(global_wont_visit_restart_times);
    return _wont_visit;
}

double Building_Node::make_heuristic(const std::size_t &city_index)
{
    return compute_heuristic(distance_with_coordinate(city_index, _centroid_x, _centroid_y));
}

std::size_t Building_Node::get_n_child_leaf()
{
    return _n_child_leaf;
}

#endif