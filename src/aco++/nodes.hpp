#ifndef _NODES_HPP_
#define _NODES_HPP_

#include <stdint.h>
#include <array>

// #include "node_tree_template.hpp"

template <class NodeT>
class Node_Base
{
public:
    NodeT *parent_ptr;
    std::array<NodeT *, 2> child_ptrs;

    Node_Base(){};
    Node_Base(NodeT *left_ptr, NodeT *right_ptr);
    ~Node_Base();
};

class Node : public Node_Base<Node>
{
public:
    Node(){};
    Node(Node *left_ptr, Node *right_ptr);
    ~Node(){};

    void reinforce(
        const double &invert_fitness,
        const double &one_minus_rho,
        const double &trail_restart,
        const double &trail_min,
        const double &trail_max,
        const std::size_t &global_restart_times,
        const std::size_t &global_evap_times);
    std::size_t choose_child_with_prob(
        const double &one_minus_q_0,
        const double &rand_num_01,
        const double &alpha,
        const double &beta,
        const double &one_minus_rho,
        const double &trail_restart,
        const double &trail_min,
        const std::size_t &global_restart_times,
        const std::size_t &global_evap_times);

protected:
    double _pheromone; // pheromone of the edge from the parent node to itself
    double _heuristic; // heuristic of the edge from the parent node to itself
    std::size_t _local_restart_times;
    std::size_t _local_evap_times;

    void _evaporate(
        const double &one_minus_rho,
        const double &trail_restart,
        const double &trail_min,
        const std::size_t &global_restart_times,
        const std::size_t &global_evap_times);
    double _prob_weight(
        const double &alpha,
        const double &beta,
        const double &one_minus_rho,
        const double &trail_restart,
        const double &trail_min,
        const std::size_t &global_restart_times,
        const std::size_t &global_evap_times);
    void _restart_if_needed(
        const std::size_t &global_restart_times,
        const double &trail_restart);
    double _temp_pheromone(
        const double &one_minus_rho,
        const double &trail_restart,
        const double &trail_min,
        const std::size_t &global_restart_times,
        const std::size_t &global_evap_times);
};

class Root : public Node
{
public:
    Root(Node *left_ptr, Node *right_ptr);
    ~Root(){};
};

class Leaf : public Node
{
public:
    Leaf(
        const std::size_t &city_index,
        const std::size_t &current_city,
        long int **distance_matrix);
    std::size_t city_index();
    ~Leaf(){};

protected:
    std::size_t _city_index;
};

class Wont_Visit_Node : public Node_Base<Wont_Visit_Node>
{
public:
    Wont_Visit_Node()
        : _wont_visit(false), _local_wont_visit_restart_times(0){};
    Wont_Visit_Node(Wont_Visit_Node *left_ptr, Wont_Visit_Node *right_ptr)
        : Node_Base<Wont_Visit_Node>(left_ptr, right_ptr),
          _wont_visit(false), _local_wont_visit_restart_times(0){};
    ~Wont_Visit_Node(){};

    void set_wont_visit(const std::size_t &global_wont_visit_restart_times);
    bool get_wont_visit();

protected:
    bool _wont_visit;
    std::size_t _local_wont_visit_restart_times;

    void _check_wont_visit(const std::size_t &global_wont_visit_restart_times);
    void _restart_if_needed(const std::size_t &global_wont_visit_restart_times);
};

#include "nodes.cpp"
#include <math.h>
#include <assert.h>

#include "nodes.hpp"

template <class NodeT>
Node_Base<NodeT>::Node_Base(NodeT *left_ptr, NodeT *right_ptr)
{
    left_ptr->parent_ptr = (NodeT *)(this);
    right_ptr->parent_ptr = (NodeT *)(this);
    child_ptrs[0] = left_ptr;
    child_ptrs[1] = right_ptr;
}

template <class NodeT>
Node_Base<NodeT>::~Node_Base()
{
    for (auto &_ptr : child_ptrs)
        delete _ptr;
}

Node::Node(Node *left_ptr, Node *right_ptr)
    : Node_Base<Node>(left_ptr, right_ptr)
{
    _local_evap_times = 0;
    _local_restart_times = 0;
    _heuristic = (left_ptr->_heuristic + right_ptr->_heuristic) / 2.0;
}

Root::Root(Node *left_ptr, Node *right_ptr)
    : Node(left_ptr, right_ptr)
{
    parent_ptr = nullptr;
};

Leaf::Leaf(
    const std::size_t &city_index,
    const std::size_t &current_city,
    long int **distance_matrix)
{
    _local_evap_times = 0;
    _local_restart_times = 0;
    _heuristic = 1.0 / ((double)distance_matrix[current_city][city_index] + 0.1);
    child_ptrs[0] = nullptr;
    child_ptrs[1] = nullptr;
    this->_city_index = city_index;
}

std::size_t Leaf::city_index() { return _city_index; };

void Node::_restart_if_needed(const uint_fast64_t &global_restart_times, const double &trail_restart)
{
    if (_local_restart_times < global_restart_times)
    {
        _local_restart_times = global_restart_times;
        _local_evap_times = 0;
        _pheromone = trail_restart;
    }
}

double Node::_temp_pheromone(const double &one_minus_rho,
                             const double &trail_restart,
                             const double &trail_min,
                             const uint_fast64_t &global_restart_times, const uint_fast64_t &global_evap_times)
{
    _restart_if_needed(global_restart_times, trail_restart);
    double pheromone = _pheromone * pow(one_minus_rho, global_evap_times - _local_evap_times);
    if (pheromone < trail_min)
        pheromone = trail_min;

    return pheromone;
}

void Node::_evaporate(const double &one_minus_rho,
                      const double &trail_restart,
                      const double &trail_min,
                      const uint_fast64_t &global_restart_times, const uint_fast64_t &global_evap_times)
{

    _pheromone = _temp_pheromone(one_minus_rho, trail_restart, trail_min, global_restart_times, global_evap_times);
    _local_evap_times = global_evap_times;
}

void Node::reinforce(const double &invert_fitness, const double &one_minus_rho,
                     const double &trail_restart,
                     const double &trail_min,
                     const double &trail_max,
                     const uint_fast64_t &global_restart_times, const uint_fast64_t &global_evap_times)
{
    _evaporate(one_minus_rho, trail_restart, trail_min, global_restart_times, global_evap_times);
    _pheromone += invert_fitness;
    if (_pheromone > trail_max)
        _pheromone = trail_max;
}

double Node::_prob_weight(const double &alpha, const double &beta, const double &one_minus_rho,
                          const double &trail_restart,
                          const double &trail_min,
                          const uint_fast64_t &global_restart_times, const uint_fast64_t &global_evap_times)
{
    const double pheromone = this->_temp_pheromone(one_minus_rho, trail_restart, trail_min, global_restart_times, global_evap_times);
    return pow(pheromone, alpha) * pow(_heuristic, beta);
}

std::size_t Node::choose_child_with_prob(const double &one_minus_q_0, const double &rand_num_01,
                                         const double &alpha, const double &beta, const double &one_minus_rho,
                                         const double &trail_restart,
                                         const double &trail_min,
                                         const uint_fast64_t &global_restart_times, const uint_fast64_t &global_evap_times)
{
    // IMPORTANCE NOTE: Remember to check won't visit before go to this function

    std::array<double, 2> weights;
    std::size_t index_of_min;
    double prob_of_min;

    weights[0] = child_ptrs[0]->_prob_weight(alpha, beta, one_minus_rho, trail_restart, trail_min, global_restart_times, global_evap_times);
    weights[1] = child_ptrs[1]->_prob_weight(alpha, beta, one_minus_rho, trail_restart, trail_min, global_restart_times, global_evap_times);
    if (weights[0] < weights[1])
        index_of_min = 0;
    else
        index_of_min = 1;

    prob_of_min = weights[index_of_min] / (weights[0] + weights[1]);
    prob_of_min *= one_minus_q_0;
    if (rand_num_01 < prob_of_min)
        return index_of_min;
    else
        return 1 - index_of_min;
}

void Wont_Visit_Node::_restart_if_needed(const uint_fast64_t &global_wont_visit_restart_times)
{
    if (_local_wont_visit_restart_times < global_wont_visit_restart_times)
    {
        _wont_visit = false;
        _local_wont_visit_restart_times = global_wont_visit_restart_times;
    }
}

void Wont_Visit_Node::set_wont_visit(const uint_fast64_t &global_wont_visit_restart_times)
{
    _local_wont_visit_restart_times = global_wont_visit_restart_times;
    _wont_visit = true;
    if (parent_ptr != nullptr)
        parent_ptr->_check_wont_visit(global_wont_visit_restart_times);
}

void Wont_Visit_Node::_check_wont_visit(const uint_fast64_t &global_wont_visit_restart_times)
{
    child_ptrs[0]->_restart_if_needed(global_wont_visit_restart_times);
    child_ptrs[1]->_restart_if_needed(global_wont_visit_restart_times);
    if (child_ptrs[0]->_wont_visit and child_ptrs[1]->_wont_visit)
        set_wont_visit(global_wont_visit_restart_times);
}

bool Wont_Visit_Node::get_wont_visit() { return _wont_visit; }

#endif