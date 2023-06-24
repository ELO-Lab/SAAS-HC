#ifndef _TREE_MAP_HPP
#define _TREE_MAP_HPP

extern bool tree_map_flag;

class Base_Node
{
    Base_Node *parent_ptr, *left, *right;
    double heuristic;

    Base_Node();
    ~Base_Node();
};

#endif