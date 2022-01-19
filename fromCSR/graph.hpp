#pragma once


#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <vector>
#include <cmath>

bool is_weight = false;

unsigned NUM_THREADS = omp_get_max_threads();

class Graph{
public:
    unsigned num_vertex;
    unsigned num_edges;
    std::vector<unsigned> row_index;
    std::vector<unsigned> col_index;
    std::vector<unsigned> out_degree;
    std::vector<float> attr;
    std::vector<unsigned> edge_weight;

    Graph(unsigned num_vertex = 0, unsigned num_edges = 0):
            num_vertex(num_vertex), num_edges(num_edges){
                row_index.reserve(num_vertex + 1);
                col_index.reserve(num_edges);
                attr.reserve(num_vertex);
                if(is_weight)
                    edge_weight.reserve(num_edges);
            }
    ~Graph(){}
    
    void computeOutDegree() {
        out_degree = std::vector<unsigned>(num_vertex, 0);
        uint64_t total = 0;
        float avr = num_edges / num_vertex;
        
        #pragma omp parallel for schedule(static, 1024 * 256) num_threads(NUM_THREADS)
        for(unsigned i = 0; i < num_vertex; i++) {
            out_degree[i] = row_index[i + 1] - row_index[i];
           // if(out_degree[i] > avr)
           //     total += out_degree[i];
        }
     //   std::cout << (float) total/num_edges << std::endl;

    }
   
    void printGraph(bool all = false) {
        std::cout << "num vertex: " << num_vertex << std::endl;
        std::cout << "num edges: " << num_edges <<std::endl;
       if(is_weight)
          std::cout << "weighted graph" <<std::endl;
  
        if(all) {
            for(auto it = row_index.begin(); it != row_index.end(); ++it) 
                std::cout << *it << " ";
            std::cout << std::endl;
            for(auto it = col_index.begin(); it != col_index.end(); ++it) 
                std::cout << *it << " ";
            std::cout << std::endl;
            for(auto it = attr.begin(); it != attr.end(); ++it) 
                std::cout << *it << " ";
            std::cout << std::endl;
        }
    }

    void initAttribute()
    {
        attr = std::vector<float>(num_vertex, 1.0);
        #pragma omp parallel for schedule(static) num_threads(NUM_THREADS)
        for (unsigned int i=0; i<num_vertex; i++)
        {
            attr[i] = 1;
            if (out_degree[i] > 0)
                attr[i] = (1.0)/out_degree[i];  
        }
        return;
    }
    /*
    Graph& operator= (Graph&& other) {
        if(&other == this)
            return *this;

        this->num_vertex = other.num_vertex;
        this->num_edges = other.num_edges;

        this->row_index.clear();
        this->col_index.clear();
        this->out_degree.clear();
        this->attr.clear();

        this->row_index = std::move(other.row_index);
        this->col_index = std::move(other.col_index);
        this->out_degree = std::move(other.out_degree);
        this->attr = std::move(other.attr);

        return *this;
    }*/
};
