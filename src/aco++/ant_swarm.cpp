#include <stdlib.h>
#include <cstddef>

#include "ants.h"
#include "thop.h"

Ant_Swarm::Ant_Swarm()
{
}

Ant_Swarm::~Ant_Swarm()
{
    for (auto ant_i : this->_ant_vec)
    {
        free_ant(ant_i);
    }
}

size_t Ant_Swarm::size()
{
    return this->_size;
}

void Ant_Swarm::free_ant(ant_struct &ant_i)
{
    free(ant_i.tour);
    free(ant_i.visited);
    free(ant_i.packing_plan);
}

void Ant_Swarm::resize(const size_t &new_size)
{
    size_t i;
    const size_t &capacity = this->_ant_vec.size();

    if (new_size > capacity)
    {
        this->_ant_vec.resize(new_size);

        for (i = capacity; i < new_size; i++)
        {
            this->allocate_ant(_ant_vec[i]);
        }
    }
    this->_size = new_size;
}

ant_struct &Ant_Swarm::operator[](size_t pos)
{
    return this->_ant_vec[pos];
}

void Ant_Swarm::allocate_ant(ant_struct &ant_i)
{
    ant_i.tour = (long int *)calloc(instance.n + 1, sizeof(long int));
    ant_i.packing_plan = (char *)calloc(instance.m, sizeof(char));
    ant_i.visited = (char *)calloc(instance.n, sizeof(char));
}

Ant_Swarm ant, prev_ls_ant;
