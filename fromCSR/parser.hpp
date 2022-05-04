#pragma once


#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "graph.hpp"
#include <boost/algorithm/string/predicate.hpp>
#include <mmio.h>

enum Format {
   // csr = 0,
    edgelist_txt = 0,
    edgelist_binary = 1,
    adj_txt = 2,
    mtx_txt = 3,
    bel_pushpull = 4
};

void parseGraph(std::string filename, Graph& graph) {

    std::cout << "loading " << filename << '\n';

    if( boost::algorithm::ends_with(filename, "csr") )
        assert(loadCSR(filename)==true);
    else if( boost::algorithm::ends_with(filename, "mix") )
        assert(loadMix(filename)==true);
    else
        std::cout << "unsupported graph format" << '\n';

    std::cout << "num vertices: " << num_vertices << " intV: " << sizeof(VerxId) << " Byte\n";
    std::cout << "num edges: " << num_edges << " intE: " << sizeof(VerxId) << " Byte\n";
 }


bool loadCSR(std::string filename, Graph& graph) {
    std::ifstream csr_file;
    csr_file.open(filename, std::ios::binary);
    if(!csr_file.is_open()) {
        std::cout << "cannot open csr file!" << std::endl;
        return false;
    }

    csr_file.read(reinterpret_cast<char*>(&graph.num_vertex), sizeof(unsigned));
    csr_file.read(reinterpret_cast<char*>(&graph.num_edges), sizeof(unsigned));

    std::cout << "num vertex: " << graph.num_vertex << std::endl;
    std::cout << "num edges: " << graph.num_edges <<std::endl;

    std::vector<unsigned> local_row(graph.num_vertex);
    std::vector<unsigned> local_col(graph.num_edges);

    csr_file.read(reinterpret_cast<char*>(local_row.data()), graph.num_vertex * sizeof(unsigned));
    csr_file.read(reinterpret_cast<char*>(local_col.data()), graph.num_edges * sizeof(unsigned));

    local_row.push_back(graph.num_edges);
    graph.csr_offset = std::move(local_row);
    graph.csr_index = std::move(local_col);

    if(is_weight) {
        std::vector<unsigned> local_wei(graph.num_edges);
        csr_file.read(reinterpret_cast<char*>(local_wei.data()), graph.num_edges * sizeof(unsigned));
        graph.edge_weight = std::move(local_wei);
    }

    csr_file.close();
    return true;
};

bool Graph::loadMix(std::string filename) {
    std::ifstream input_file (filename, std::ios::binary);
    if(!input_file.is_open()) {
        std::cout << "cannot open the input mix file!" << '\n';
        return false;
    }


    input_file.read(reinterpret_cast<char*>(&num_vertices), sizeof(EdgeId));
    input_file.read(reinterpret_cast<char*>(&num_edges), sizeof(VerxId));
    
    csr_offset.resize(num_vertices);
    csr_index.resize(num_edges); 
    csc_offset.resize(num_vertices);
    csc_index.resize(num_edges);

    input_file.read(reinterpret_cast<char*>(csr_offset.data()), num_vertices * sizeof(EdgeId));
    input_file.read(reinterpret_cast<char*>(csr_index.data()), num_edges * sizeof(VerxId));

    input_file.read(reinterpret_cast<char*>(csc_offset.data()), num_vertices * sizeof(EdgeId));
    input_file.read(reinterpret_cast<char*>(csc_index.data()), num_edges * sizeof(VerxId));

    input_file.close();

    out_degree.resize(num_vertices);
    in_degree.resize(num_vertices);

    return true;
}

void writeEdgelist(std::string filename, Graph& graph) {
    std::ofstream output(filename);

    if(!output.is_open()) {
        std::cout << "cannot open the output file!" << std::endl;
        return;
    }
    for(unsigned i = 0; i < graph.num_vertex; i++) {
        for(unsigned j = graph.csr_offset[i]; j < graph.csr_offset[i+1]; j++) {
            output << i << " " << graph.csr_index[j];
            if(is_weight)
                output << " " << graph.edge_weight[j];
            output << '\n';
        }
    }
        output.close();
    std::cout << "the format of graph is been converted from CSR to Edgelist-text and stored in file: " << filename << std::endl;
}

void writeEdgelistBin(std::string filename, Graph& graph) {
    FILE* fp = fopen(filename.c_str(), "wb");
    if (fp == NULL)
    {
        fputs("file error", stderr);
        return;
    }
    fwrite(&graph.num_vertex, sizeof(unsigned), 1, fp); 
    fwrite(&graph.num_edges, sizeof(unsigned), 1, fp); 

    for(unsigned i = 0; i < graph.num_vertex; i++) {
        for(unsigned j = graph.csr_offset[i]; j < graph.csr_offset[i+1]; j++) {
            fwrite(&i, sizeof(unsigned), 1, fp); 
            fwrite(graph.csr_index.data()+j, sizeof(unsigned), 1, fp); 
            if(is_weight)
                fwrite(graph.edge_weight.data()+j, sizeof(unsigned), 1, fp); 
        }
    }
    fclose(fp);
    std::cout << "the format of graph is been converted from CSR to Edgelist-bin and stored in file: " << filename << std::endl;

}

void writeAdj(std::string filename, Graph& graph) {
    std::ofstream output;
    output.open(filename, std::ios::app);

    if(!output.is_open()) {
        std::cout << "cannot open the output file!" << std::endl;
        return;
    }
    if(is_weight)
        std::cout << "weighted graph" << '\n';
    if(is_weight)
        output << "WeightedAdjacencyGraph" << '\n';
    else
        output << "AdjacencyGraph" << '\n';
    output << graph.num_vertex << '\n';
    output << graph.num_edges << '\n';
    for(unsigned i = 0; i < graph.num_vertex; i++)
        output << graph.csr_offset[i] << '\n';
    for(auto e: graph.csr_index)
        output << e << '\n';
    if(is_weight)
        for(auto w: graph.edge_weight)
            output << w << '\n';
    output.close();
    std::cout << "the format of graph is been converted from CSR to (Ligra) Adjacency List and stored in file: " << filename << std::endl;
}

void writeMtx(std::string filename, Graph& graph) {
    std::ofstream output(filename);

    if(!output.is_open()) {
        std::cout << "cannot open the output file!" << std::endl;
        return;
    }
    output << graph.num_vertex << " " << graph.num_vertex << " " << graph.num_edges << '\n';
    for(unsigned i = 0; i < graph.num_vertex; i++) {
        for(unsigned j = graph.csr_offset[i]; j < graph.csr_offset[i+1]; j++) {
            output << i+1 << " " << graph.csr_index[j]+1;
            if(is_weight)
                output << " " << graph.edge_weight[j];
            output << '\n';
        }
    }
        output.close();
    std::cout << "the format of graph is been converted from CSR to (GraphMat) MTX and stored in file: " << filename << std::endl;
}

void writeBelPP(std::string filename, Graph& graph) {


}


void writeGraph(std::string filename, Graph& graph, Format format) {
    switch(format) {
        case Format::edgelist_txt:
            writeEdgelist(filename, graph);
            break;
        case Format::edgelist_binary:
            writeEdgelistBin(filename, graph);
            break;
        case Format::adj_txt:
            writeAdj(filename, graph);
            break;
        case Format::mtx_txt:
            writeMtx(filename, graph);
            break;
        case Format::bel_pushpull:
            writeBelPP(filename, graph);
            break;
        default:
            std::cout << "choose your output format!" << std::endl;
    }
}
