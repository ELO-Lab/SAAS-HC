#include <assert.h>

#include "trees.h"

Tree_Edge::Tree_Edge(const std::size_t &num_city, const std::size_t &current_city, long int **&distance_matrix)
{
    std::size_t i;
    std::vector<Node *> node_ptrs;

    _leaf_ptrs.resize(num_city);
    _leaf_ptrs[0] = nullptr; // Do not go to the start city
    for (i = 1; i < num_city; i++)
    {
        _leaf_ptrs[i] = new Leaf(i, current_city, distance_matrix);
        node_ptrs.push_back(_leaf_ptrs[i]);
    }
    _build_tree(node_ptrs);
}

void Tree_Edge::_build_tree(std::vector<Node *> &node_ptrs)
{
    if (node_ptrs.size() == 2)
    {
        _root_ptr = new Node(node_ptrs[0], node_ptrs[1], true);
        return;
    }

    std::size_t i;
    std::vector<Node *> parent_ptrs;

    for (i = 0; i + 1 < node_ptrs.size(); i += 2)
        parent_ptrs.push_back(new Node(node_ptrs[i], node_ptrs[i + 1]));
    if (i == node_ptrs.size() - 1)
        parent_ptrs.push_back(node_ptrs[i]);

    node_ptrs.resize(0);
    node_ptrs.shrink_to_fit();
    _build_tree(parent_ptrs);
}

std::size_t Tree_Edge::choose_next_city(
    Wont_Visit_Node *wont_visit_root_ptr,
    const std::function<double()> &rand01,
    const double &one_minus_q_0,
    const double &alpha,
    const double &beta,
    const double &one_minus_rho,
    const double &past_trail_restart,
    const double &past_trail_min,
    const std::size_t &global_restart_times,
    const std::size_t &global_evap_times,
    const std::size_t &global_wont_visit_restart_times)
{
    Node *current_ptr = _root_ptr;
    Wont_Visit_Node *current_wont_visit_ptr = wont_visit_root_ptr;
    std::size_t next_child_index;
    bool left_wont_visit, right_wont_visit;

    while (current_ptr->child_ptrs[0] != nullptr)
    {
        left_wont_visit = current_wont_visit_ptr->child_ptrs[0]->get_wont_visit(global_wont_visit_restart_times);
        right_wont_visit = current_wont_visit_ptr->child_ptrs[1]->get_wont_visit(global_wont_visit_restart_times);

        if (left_wont_visit && !right_wont_visit)
            next_child_index = 1;
        else if (!left_wont_visit && right_wont_visit)
            next_child_index = 0;
        else if (!left_wont_visit && !right_wont_visit)
            next_child_index = current_ptr->choose_child_with_prob(
                one_minus_q_0, rand01(),
                alpha, beta, one_minus_rho, past_trail_restart, past_trail_min,
                global_restart_times, global_evap_times);
        else
            assert(false);

        current_ptr = current_ptr->child_ptrs[next_child_index];
        current_wont_visit_ptr = current_wont_visit_ptr->child_ptrs[next_child_index];
    }

    return ((Leaf *)current_ptr)->city_index();
}

void Tree_Edge::reinforce(
    const std::size_t &city_index,
    const double &invert_fitness,
    const double &one_minus_rho,
    const double &past_trail_restart,
    const double &past_trail_min,
    const double &trail_max,
    const std::size_t &global_restart_times,
    const std::size_t &global_evap_times)
{
    Node *node_ptr = _leaf_ptrs[city_index];
    while (node_ptr->parent_ptr != nullptr)
    {
        node_ptr->reinforce(
            invert_fitness,
            one_minus_rho,
            past_trail_restart,
            past_trail_min,
            trail_max,
            global_restart_times,
            global_evap_times);
        node_ptr = node_ptr->parent_ptr;
    }
}

double Tree_Edge::leaf_pheromone(
    const std::size_t &city_index,
    const double &one_minus_rho,
    const double &past_trail_restart,
    const double &past_trail_min,
    const std::size_t &global_restart_times,
    const std::size_t &global_evap_times)
{
    return _leaf_ptrs[city_index]->get_pheromone(
        one_minus_rho,
        past_trail_restart,
        past_trail_min,
        global_restart_times,
        global_evap_times);
}

Wont_Visit_Tree::Wont_Visit_Tree(const std::size_t &num_city)
{
    std::size_t i;
    std::vector<Wont_Visit_Node *> node_ptrs;

    _leaf_ptrs.resize(num_city);
    _leaf_ptrs[0] = nullptr; // Do not go to the start city
    for (i = 1; i < num_city; i++)
    {
        _leaf_ptrs[i] = new Wont_Visit_Node();
        node_ptrs.push_back(_leaf_ptrs[i]);
    }
    _build_tree(node_ptrs);
}

void Wont_Visit_Tree::_build_tree(std::vector<Wont_Visit_Node *> &node_ptrs)
{
    if (node_ptrs.size() == 2)
    {
        _root_ptr = new Wont_Visit_Node(node_ptrs[0], node_ptrs[1], true);
        return;
    }

    std::size_t i;
    std::vector<Wont_Visit_Node *> parent_ptrs;

    for (i = 0; i + 1 < node_ptrs.size(); i += 2)
        parent_ptrs.push_back(new Wont_Visit_Node(node_ptrs[i], node_ptrs[i + 1]));
    if (i == node_ptrs.size() - 1)
        parent_ptrs.push_back(node_ptrs[i]);

    node_ptrs.resize(0);
    node_ptrs.shrink_to_fit();
    _build_tree(parent_ptrs);
}

void Wont_Visit_Tree::set_wont_visit(const std::size_t &city_index, const std::size_t &num_city, const std::size_t &global_wont_visit_restart_times)
{
    if (city_index != 0 and city_index != num_city - 1)
        _leaf_ptrs[city_index]->set_wont_visit(global_wont_visit_restart_times);
    assert(_root_ptr->get_wont_visit(global_wont_visit_restart_times) == false);
}

Wont_Visit_Node *Wont_Visit_Tree::get_root_ptr() { return _root_ptr; }
