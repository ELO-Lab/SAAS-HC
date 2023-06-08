#include <stdlib.h>
#include <cstddef>

#include "ants.h"
#include "thop.h"

Ant_Swarm::Ant_Swarm()
{
}

Ant_Swarm::~Ant_Swarm()
{
    for (auto ant_i : this->ant_vec)
    {
        free_ant(ant_i);
    }
}

size_t Ant_Swarm::size()
{
    return this->ant_vec.size();
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
    const size_t &old_size = this->ant_vec.size();

    for (i = new_size; i < old_size; i++)
    {
        this->free_ant(ant_vec[i]);
    }

    this->ant_vec.resize(new_size);

    for (i = old_size; i < new_size; i++)
    {
        this->allocate_ant(ant_vec[i]);
    }
}

ant_struct &Ant_Swarm::operator[](size_t pos)
{
    return this->ant_vec[pos];
}

void Ant_Swarm::allocate_ant(ant_struct &ant_i)
{
    ant_i.tour = (long int *)calloc(instance.n + 1, sizeof(long int));
    ant_i.packing_plan = (char *)calloc(instance.m, sizeof(char));
    ant_i.visited = (char *)calloc(instance.n, sizeof(char));
}

Ant_Swarm ant, prev_ls_ant;
