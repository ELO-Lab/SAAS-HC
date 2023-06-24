#include "tree_map.hpp"

// hyperparameters
bool tree_map_flag = true;

Base_Node::Base_Node()
{
}

Base_Node::destructor()
{
}

Base_Node::~Base_Node()
{
    left->destructor();
    right->destructor();
}