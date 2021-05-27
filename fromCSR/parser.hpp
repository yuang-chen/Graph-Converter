#pragma once


#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "graph.hpp"

enum Format {
   // csr = 0,
    edgelist_txt = 0,
    edgelist_binary = 1,
    adjacency_txt = 2,
    adjacency_binary = 3
};

bool parseGraph(std::string filename, Graph& graph) {
    std::ifstream csr_file;
    csr_file.open(filename, std::ios::binary);
    if(!csr_file.is_open()) {
        std::cout << "cannot open csr file!" << std::endl;
        return false;
    }

    csr_file.read(reinterpret_cast<char*>(&graph.num_vertex), sizeof(unsigned));
    csr_file.read(reinterpret_cast<char*>(&graph.num_edges), sizeof(unsigned));

    std::vector<unsigned> local_row(graph.num_vertex);
    std::vector<unsigned> local_col(graph.num_edges);

    csr_file.read(reinterpret_cast<char*>(local_row.data()), graph.num_vertex * sizeof(unsigned));
    csr_file.read(reinterpret_cast<char*>(local_col.data()), graph.num_edges * sizeof(unsigned));

    local_row.push_back(graph.num_edges);
    graph.row_index = std::move(local_row);
    graph.col_index = std::move(local_col);

#ifdef WEIGHTED
    std::vector<unsigned> local_wei(graph.num_edges);
    csr_file.read(reinterpret_cast<char*>(local_wei.data()), graph.num_edges * sizeof(unsigned));
    graph.edge_weight = std::move(local_wei);
#endif
    csr_file.close();
    return true;
};

bool writeEdgelist(std::string filename, Graph& graph) {
    std::ofstream output(filename);

    if(!output.is_open()) {
        std::cout << "cannot open the output file!" << std::endl;
        return false;
    }
    for(unsigned i = 0; i < graph.num_vertex; i++) {
        for(unsigned j = graph.row_index[i]; j < graph.row_index[i+1]; j++) {
            output << i << " " << graph.col_index[j] << '\n';
        }
    }
        output.close();
    std::cout << "the format of graph is been converted from CSR to Edgelist and stored in file: " << filename << std::endl;
}

void writeEdgelistBin(std::string filename, Graph& graph) {
    FILE* fp = fopen(filename.c_str(), "wb");
    if (fp == NULL)
    {
        fputs("file error", stderr);
        return;
    }
    for(unsigned i = 0; i < graph.num_vertex; i++) {
        for(unsigned j = graph.row_index[i]; j < graph.row_index[i+1]; j++) {
            fwrite(&i, sizeof(unsigned), 1, fp); 
            fwrite(graph.col_index.data()+j, sizeof(unsigned), 1, fp); 
        }
    }

    fclose(fp);
}

bool writeAdj(std::string filename, Graph& graph) {
    std::ofstream output(filename);

    if(!output.is_open()) {
        std::cout << "cannot open the output file!" << std::endl;
        return false;
    }
    output << "AdjacencyGraph" << '\n';
    output << graph.num_vertex << '\n';
    output << graph.num_edges << '\n';
    for(unsigned i = 0; i < graph.num_vertex; i++)
        output << graph.row_index[i] << '\n';
    for(auto e: graph.col_index)
        output << e << '\n';

    output.close();
    std::cout << "the format of graph is been converted from CSR to Adjacency and stored in file: " << filename << std::endl;
}
/*
bool writeCSR(std::string filename, Graph& graph) {
    std::ofstream  output_file(filename);
    if(!output_file.is_open()) {
        std::cout << "cannot open txt file!" << std::endl;
        return false;
    }
    output_file.write(reinterpret_cast<char*>(&graph.num_vertex), sizeof(unsigned));
    output_file.write(reinterpret_cast<char*>(&graph.num_edges), sizeof(unsigned));

    output_file.write(reinterpret_cast<char*>(graph.row_index.data()), graph.num_vertex * sizeof(unsigned));
    output_file.write(reinterpret_cast<char*>(graph.col_index.data()), graph.num_edges * sizeof(unsigned));

#ifdef WEIGHTED
    output_file.write(reinterpret_cast<char*>(graph.edge_weight.data()), graph.num_edges * sizeof(unsigned));
#endif

    output_file.close();
    std::cout << "the graph in CSR format is stored in file: " << filename << std::endl;

};*/

void writeGraph(std::string filename, Graph& graph, Format format) {
    switch(format) {
        case Format::edgelist_txt:
            writeEdgelist(filename, graph);
            break;
        case Format::edgelist_binary:
            writeEdgelistBin(filename, graph);
            break;
        case Format::adjacency_txt:
            writeAdj(filename, graph);
            break;
        case Format::adjacency_binary:
            std::cout << "adj. format is NOT implemented!" << std::endl;
            break;
        default:
            std::cout << "choose your output format!" << std::endl;
    }
}
