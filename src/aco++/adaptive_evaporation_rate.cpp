#include "thop.h"
#include "ants.h"

#include <vector>
#include <math.h>
#include <iostream>

double rho_min = 0.01;
double rho_max = 0.99;

int adaptive_evaporation_flag;

void calculate_edges_occurence(std::vector< std::vector<long int> > &edges_occurence, long int &number_of_edge, long int &min_edge){
    for (long int i = 0; i < n_ants; i++){
        long edges = 0;
        for (long int j = 0; j < ant[i].tour_size - 1; j++){
            long int u = ant[i].tour[j];
            long int v = ant[i].tour[j+1];
            edges_occurence[u][v] += 1;
            edges_occurence[v][u] += 1;
            number_of_edge += 1;
            edges += 1;
        }
        if (edges < min_edge){
            min_edge = edges;
        }
    }
}

double calculate_entropy(const std::vector< std::vector<long int> > &edges_occurence, const long int &number_of_edge) {
    double entropy = 0;
    for (int i = 1; i <= instance.n; i++){
        for (int j = 0; j < i; j++){
            if (edges_occurence[i][j] != 0){
                double pij = double(edges_occurence[i][j]) / number_of_edge;
                entropy -= pij * log2(pij);
            }
        }
    }

    return entropy;
}

void update_evaporation_rate(void){
    std::vector< std::vector<long int> > edges_occurence(instance.n + 2, std::vector<long int>(instance.n + 2, 0));
    long int number_of_edge = 0;
    long int min_edge = __INT_MAX__;

    calculate_edges_occurence(edges_occurence, number_of_edge, min_edge);

    double entropy = calculate_entropy(edges_occurence, number_of_edge);

    double min_entropy = -log2(n_ants * 1.0 / number_of_edge);
    double max_entropy = -log2(1.0 / number_of_edge);
    // double min_entropy = -log2(1.0/number_of_edge);
    // double max_entropy = -double(number_of_edge)/n_ants * log2(1.0/n_ants);

    rho = rho_max - (rho_max - rho_min) * ( (entropy - min_entropy) / (max_entropy - min_entropy) );
    // rho = rho_min + (rho_max - rho_min) * ( (entropy - min_entropy) / (max_entropy - min_entropy) );
    std::cout << "min_ent " << min_entropy <<" entropy " << entropy << " max_ent " <<  max_entropy << " rho " << rho << std::endl;
}