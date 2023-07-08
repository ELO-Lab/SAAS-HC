#include <assert.h>

#include <mlpack.hpp>

#include "utilities.h"
#include "ants.h"
#include "thop.h"
#include "trees.h"

Tree_Edge::Tree_Edge(const std::size_t &num_city, const std::size_t &current_city, Building_Node *building_root_ptr)
{
    // Top-down

    // _current_city = current_city;
    _root_ptr = new Node();
    _leaf_ptrs.resize(num_city);
    _leaf_ptrs[0] = nullptr; // Not allowed to revisit the starting city
    _build_childs(_root_ptr, building_root_ptr, current_city);
}

void Tree_Edge::_build_childs(Node *&parent_ptr, Building_Node *building_parent_ptr, const std::size_t &current_city)
{
    std::size_t i;
    bool is_leaf;
    Building_Leaf *building_leaf_ptr;
    Building_Node *building_child_ptr;
    double heuristic;

    for (i = 0; i < 2; i++)
    {
        building_child_ptr = building_parent_ptr->child_ptrs[i];
        heuristic = building_child_ptr->make_heuristic(current_city);
        is_leaf = building_child_ptr->child_ptrs[0] == nullptr;

        if (is_leaf)
        {
            building_leaf_ptr = (Building_Leaf *)(building_child_ptr);
            const std::size_t city_index = building_leaf_ptr->get_city_index();

            parent_ptr->child_ptrs[i] = new Leaf(parent_ptr, heuristic, city_index);
            _leaf_ptrs[city_index] = (Leaf *)(parent_ptr->child_ptrs[i]);
            continue;
        }

        parent_ptr->child_ptrs[i] = new Node(parent_ptr, heuristic, building_child_ptr->get_n_child_leaf());
        _build_childs(parent_ptr->child_ptrs[i], building_child_ptr, current_city);
    }
}

Tree_Edge::Tree_Edge(const std::size_t &num_city, const std::size_t &current_city)
{
    // Bottom-up

    std::size_t i;
    std::vector<Node *> node_ptrs;

    // _current_city = current_city;
    _leaf_ptrs.resize(num_city);
    _leaf_ptrs[0] = nullptr; // Not allowed to revisit the starting city
    // _leaf_ptrs[current_city] = nullptr; // Not allowed to stand still
    for (i = 1; i < num_city; i++)
    {
        _leaf_ptrs[i] = new Leaf(i, HEURISTIC(current_city, i));
        node_ptrs.push_back(_leaf_ptrs[i]);
    }
    _bottom_up_build_tree(node_ptrs);
}

void Tree_Edge::_bottom_up_build_tree(std::vector<Node *> &node_ptrs)
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
    _bottom_up_build_tree(parent_ptrs);
}

std::size_t Tree_Edge::choose_next_city(
    Wont_Visit_Tree *wont_visit_tree_ptr,
    const double &neighbour_prob,
    const double &alpha,
    const double &beta,
    const double &one_minus_rho,
    const double &past_trail_restart,
    const double &past_trail_min,
    const std::size_t &global_restart_times,
    const std::size_t &global_evap_times,
    const std::size_t &global_wont_visit_restart_times,
    const std::size_t &nn_ants,
    long *nn_list,
    const std::size_t &num_city,
    const double &elite_prob)
{
    if (new_rand01() < neighbour_prob)
        return _choose_neighbour_leaf(
            wont_visit_tree_ptr,
            alpha,
            beta,
            one_minus_rho,
            past_trail_restart,
            past_trail_min,
            global_restart_times,
            global_evap_times,
            global_wont_visit_restart_times,
            nn_ants,
            nn_list,
            num_city,
            elite_prob);
    // return _choose_best_leaf(
    //     wont_visit_tree_ptr,
    //     alpha, beta, one_minus_rho,
    //     past_trail_restart, past_trail_min,
    //     global_restart_times, global_evap_times,
    //     global_wont_visit_restart_times,
    //     num_city);
    else
        return _walk_from_root(
            wont_visit_tree_ptr->get_root_ptr(),
            alpha,
            beta,
            one_minus_rho,
            past_trail_restart,
            past_trail_min,
            global_restart_times,
            global_evap_times,
            global_wont_visit_restart_times,
            elite_prob);
}

std::size_t Tree_Edge::_walk_from_root(
    Wont_Visit_Node *wont_visit_root_ptr,
    const double &alpha,
    const double &beta,
    const double &one_minus_rho,
    const double &past_trail_restart,
    const double &past_trail_min,
    const std::size_t &global_restart_times,
    const std::size_t &global_evap_times,
    const std::size_t &global_wont_visit_restart_times,
    const double &elite_prob)
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
                alpha, beta, one_minus_rho, past_trail_restart, past_trail_min,
                global_restart_times, global_evap_times, elite_prob);
        else
            assert(false);

        current_ptr = current_ptr->child_ptrs[next_child_index];
        current_wont_visit_ptr = current_wont_visit_ptr->child_ptrs[next_child_index];
    }

    return ((Leaf *)current_ptr)->get_city_index();
}

std::size_t Tree_Edge::_choose_neighbour_leaf(
    Wont_Visit_Tree *wont_visit_tree_ptr,
    const double &alpha,
    const double &beta,
    const double &one_minus_rho,
    const double &past_trail_restart,
    const double &past_trail_min,
    const std::size_t &global_restart_times,
    const std::size_t &global_evap_times,
    const std::size_t &global_wont_visit_restart_times,
    const std::size_t &nn_ants,
    long *nn_list,
    const std::size_t &num_city,
    const double &elite_prob)
{
    std::size_t i, city_index;
    double rand_num, total_weights, partial_sum;
    std::vector<double> prob_weights(nn_ants);

    total_weights = 0.0;
    for (i = 0; i < nn_ants; i++)
    {
        city_index = nn_list[i];
        assert(0 < city_index && city_index < num_city);
        if (city_index == num_city || wont_visit_tree_ptr->check_city_visited(city_index, global_wont_visit_restart_times))
            prob_weights[i] = 0.0;
        else
        {
            prob_weights[i] = _leaf_ptrs[city_index]->prob_weight(
                alpha,
                beta,
                one_minus_rho,
                past_trail_restart,
                past_trail_min,
                global_restart_times,
                global_evap_times);
            total_weights += prob_weights[i];
        }
    }

    if (total_weights <= 0.0)
    {
        // All neighbours are visited
        return _walk_from_root(
            wont_visit_tree_ptr->get_root_ptr(),
            alpha,
            beta,
            one_minus_rho,
            past_trail_restart,
            past_trail_min,
            global_restart_times,
            global_evap_times,
            global_wont_visit_restart_times,
            elite_prob);
    }

    rand_num = new_rand01() * total_weights;
    i = 0;
    partial_sum = prob_weights[i];
    while (partial_sum <= rand_num)
    {
        i++;
        partial_sum += prob_weights[i];
    }
    city_index = nn_list[i];

    assert(0 <= i && i < nn_ants);
    assert(prob_weights[i] >= 0.0);
    return city_index;
}

std::size_t Tree_Edge::_choose_best_leaf(
    Wont_Visit_Tree *wont_visit_tree_ptr,
    const double &alpha, const double &beta, const double &one_minus_rho,
    const double &past_trail_restart, const double &past_trail_min,
    const std::size_t &global_restart_times, const std::size_t &global_evap_times,
    const std::size_t &global_wont_visit_restart_times,
    const std::size_t &num_city)
{

    std::size_t i, best_city;
    double max_prob_weight, temp_prob_weight;

    best_city = 0;
    max_prob_weight = 0;
    for (i = 1; i < num_city; i++)
    {
        if (wont_visit_tree_ptr->check_city_visited(i, global_wont_visit_restart_times))
            continue;

        temp_prob_weight = _leaf_ptrs[i]->prob_weight_without_child_leaf(
            alpha, beta, one_minus_rho,
            past_trail_restart, past_trail_min,
            global_restart_times, global_evap_times);
        if (temp_prob_weight > max_prob_weight)
        {
            max_prob_weight = temp_prob_weight;
            best_city = i;
        }
    }

    assert(best_city > 0);
    return best_city;
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

Wont_Visit_Tree::Wont_Visit_Tree(const std::size_t &num_city, Building_Node *building_root_ptr)
{
    // Top-down
    _root_ptr = new Wont_Visit_Node(nullptr);
    _leaf_ptrs.resize(num_city);
    _leaf_ptrs[0] = nullptr; // Not allowed to revisit the starting city
    _build_childs(_root_ptr, building_root_ptr);
}

void Wont_Visit_Tree::_build_childs(Wont_Visit_Node *&parent_ptr, Building_Node *building_parent_ptr)
{
    std::size_t i;
    bool is_leaf;
    Building_Leaf *building_leaf_ptr;
    Building_Node *building_child_ptr;

    for (i = 0; i < 2; i++)
    {
        building_child_ptr = building_parent_ptr->child_ptrs[i];
        is_leaf = building_child_ptr->child_ptrs[0] == nullptr;

        if (is_leaf)
        {
            building_leaf_ptr = (Building_Leaf *)(building_child_ptr);
            parent_ptr->child_ptrs[i] = new Wont_Visit_Node(parent_ptr, true);
            _leaf_ptrs[building_leaf_ptr->get_city_index()] = parent_ptr->child_ptrs[i];
            continue;
        }

        parent_ptr->child_ptrs[i] = new Wont_Visit_Node(parent_ptr);
        _build_childs(parent_ptr->child_ptrs[i], building_child_ptr);
    }
}

Wont_Visit_Tree::Wont_Visit_Tree(const std::size_t &num_city)
{
    // Bottom-up
    std::size_t i;
    std::vector<Wont_Visit_Node *> node_ptrs;

    _leaf_ptrs.resize(num_city);
    _leaf_ptrs[0] = nullptr; // Not allowed to revisit the starting city
    for (i = 1; i < num_city; i++)
    {
        _leaf_ptrs[i] = new Wont_Visit_Node(nullptr, nullptr);
        node_ptrs.push_back(_leaf_ptrs[i]);
    }
    _bottom_up_build_tree(node_ptrs);
}

void Wont_Visit_Tree::_bottom_up_build_tree(std::vector<Wont_Visit_Node *> &node_ptrs)
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
    _bottom_up_build_tree(parent_ptrs);
}

void Wont_Visit_Tree::set_wont_visit(const std::size_t &city_index, const std::size_t &num_city, const std::size_t &global_wont_visit_restart_times)
{
    if (city_index != 0 and city_index != num_city - 1)
        _leaf_ptrs[city_index]->set_wont_visit(global_wont_visit_restart_times);
    assert(_root_ptr->get_wont_visit(global_wont_visit_restart_times) == false);
}

bool Wont_Visit_Tree::check_city_visited(const std::size_t &city_index, const std::size_t &global_wont_visit_restart_times)
{
    return _leaf_ptrs[city_index]->get_wont_visit(global_wont_visit_restart_times);
}

Building_Tree::Building_Tree(const struct problem &instance)
{
    cluster_struct cluster;
    const std::size_t num_city = instance.n - 1;

    _make_first_cluster(instance, cluster);

    _root_ptr = new Building_Node();
    _root_ptr->child_ptrs[0] = new Building_Node(_root_ptr, cluster.centroid_x, cluster.centroid_y, cluster.indexes.size());
    _root_ptr->child_ptrs[1] = new Building_Leaf(
        _root_ptr,
        instance.nodeptr[num_city - 1].x,
        instance.nodeptr[num_city - 1].y,
        num_city - 1);

    _build_childs(_root_ptr->child_ptrs[0], cluster.indexes, cluster.features);
}

void Building_Tree::_build_childs(Building_Node *parent_ptr, const std::vector<std::size_t> &city_indexes, const arma::mat &city_features)
{
    std::vector<cluster_struct> clusters;
    std::size_t i;

    _cluster_cities(
        city_indexes,
        city_features,
        clusters);

    for (i = 0; i < 2; i++)
    {
        if (clusters[i].indexes.size() == 1)
        {
            parent_ptr->child_ptrs[i] = new Building_Leaf(parent_ptr, clusters[i].centroid_x, clusters[i].centroid_y, clusters[i].indexes[0]);
            continue;
        }

        parent_ptr->child_ptrs[i] = new Building_Node(parent_ptr, clusters[i].centroid_x, clusters[i].centroid_y, clusters[i].indexes.size());
        _build_childs(parent_ptr->child_ptrs[i], clusters[i].indexes, clusters[i].features);
    }
}

void Building_Tree::_make_first_cluster(const struct problem &instance, cluster_struct &cluster)
{
    std::vector<std::vector<double>> weights, profits, profit_over_weight_ratios;
    std::size_t i, cluster_index;
    const std::size_t num_city = instance.n - 1;
    const std::size_t cluster_size = num_city - 2; // No starting or ending cites
    const std::size_t n_features = 8;
    // const std::size_t n_features = 2;
    double p_w_ratio, mean_value, std_value;

    cluster.centroid_x = 0;
    cluster.centroid_y = 0;
    cluster.indexes.resize(cluster_size);
    cluster.features = arma::mat(n_features, cluster_size);
    weights.resize(cluster_size);
    profits.resize(cluster_size);
    profit_over_weight_ratios.resize(cluster_size);

    for (i = 0; i < instance.m; i++)
    {
        cluster_index = instance.itemptr[i].id_city - 1;
        p_w_ratio = double(instance.itemptr[i].profit) / instance.itemptr[i].weight;

        weights[cluster_index].push_back(instance.itemptr[i].weight);
        profits[cluster_index].push_back(instance.itemptr[i].profit);
        profit_over_weight_ratios[cluster_index].push_back(p_w_ratio);
    }

    for (i = 1; i < cluster_size + 1; i++)
    {
        cluster_index = i - 1;
        cluster.indexes[cluster_index] = i;
        cluster.centroid_x += instance.nodeptr[i].x / cluster_size;
        cluster.centroid_y += instance.nodeptr[i].y / cluster_size;
        cluster.features(0, cluster_index) = instance.nodeptr[i].x;
        cluster.features(1, cluster_index) = instance.nodeptr[i].y;

        mean_and_std(weights[cluster_index], mean_value, std_value);
        cluster.features(2, cluster_index) = mean_value - 2 * std_value;
        cluster.features(3, cluster_index) = mean_value + 2 * std_value;

        mean_and_std(profits[cluster_index], mean_value, std_value);
        cluster.features(4, cluster_index) = mean_value - 2 * std_value;
        cluster.features(5, cluster_index) = mean_value + 2 * std_value;

        mean_and_std(profit_over_weight_ratios[cluster_index], mean_value, std_value);
        cluster.features(6, cluster_index) = mean_value - 2 * std_value;
        cluster.features(7, cluster_index) = mean_value + 2 * std_value;
    }
}

void Building_Tree::_cluster_cities(const std::vector<std::size_t> &indexes, const arma::mat &features, std::vector<cluster_struct> &clusters)
{
    if (indexes.size() <= 1)
        return;
    mlpack::Log::Info.ignoreInput = true;

    const int n_clusters = 2;
    const std::size_t n_cities = features.n_cols;
    const std::size_t n_features = features.n_rows;
    std::size_t i, cluster_index;

    arma::Row<std::size_t> assignments;
    arma::mat centroid;

    mlpack::tree::KMeans<> kmeans;

    kmeans.Cluster(features, n_clusters, assignments, centroid);

    clusters = std::vector<cluster_struct>(n_clusters);

    for (i = 0; i < n_clusters; i++)
    {
        clusters[i].features = arma::mat(n_features, 0);
        clusters[i].centroid_x = centroid(0, i);
        clusters[i].centroid_y = centroid(1, i);
    }

    for (i = 0; i < indexes.size(); i++)
    {
        cluster_index = assignments[i];
        clusters[cluster_index].indexes.push_back(indexes[i]);
        clusters[cluster_index].features = arma::join_rows(clusters[cluster_index].features, arma::mat(features.col(i)));
    }

    int index = -1;
    if (clusters[0].indexes.size() == 0)
        index = 0;
    if (clusters[1].indexes.size() == 0)
        index = 1;

    if (index != -1)
    {
        int len = clusters[1 - index].indexes.size();
        clusters[index].indexes.push_back(clusters[1 - index].indexes[len - 1]);
        clusters[index].features = arma::mat(clusters[1 - index].features.col(len - 1));

        clusters[1 - index].indexes.pop_back();
        clusters[1 - index].features.shed_col(len - 1);
    }
}