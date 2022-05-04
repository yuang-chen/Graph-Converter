#pragma once


#include <iostream>
#include <stdio.h>
#include <fstream>

#include <stdlib.h>
#include <omp.h>
#include <vector>
#include <cmath>
#include <stdint.h>
#include <assert.h>
#include <boost/algorithm/string/predicate.hpp>

/// input graph format type
enum Type {
    csr = 0,
    mix = 1
};
/// output graph format type
enum Format {
    edgelist_txt = 0,
    edgelist_binary = 1,
    adj_txt = 2,
    mtx_txt = 3,
    b64 = 4
};


unsigned NUM_THREADS = omp_get_max_threads();

class Graph{
public:
    unsigned num_vertex;
    unsigned num_edges;
    std::vector<unsigned> csr_offset;
    std::vector<unsigned> csr_index;
    std::vector<unsigned> out_degree;

    std::vector<unsigned> csc_offset; // 
    std::vector<unsigned> csc_index;   // 
    std::vector<unsigned> in_degree;

    std::vector<unsigned> edge_weight;

    bool is_weighted = false;

    Graph(bool is_w): num_vertex(0), num_edges(0), is_weighted(is_w){}
    ~Graph(){}
    
    void computeOutDegree() {
        out_degree = std::vector<unsigned>(num_vertex, 0);
        uint64_t total = 0;
        float avr = num_edges / num_vertex;
        
        #pragma omp parallel for schedule(static, 1024 * 256) num_threads(NUM_THREADS)
        for(unsigned i = 0; i < num_vertex; i++) {
            out_degree[i] = csr_offset[i + 1] - csr_offset[i];
           // if(out_degree[i] > avr)
           //     total += out_degree[i];
        }
     //   std::cout << (float) total/num_edges << std::endl;

    }
 


    void convert(std::string in, std::string out) {

        if( boost::algorithm::ends_with(in, "csr") )
            assert(loadCSR(in)==true);
        else if( boost::algorithm::ends_with(in, "mix") )
            assert(loadMix(in)==true);
        else
            std::cout << "unsupported input graph format" << '\n';
        
        std::cout << "num vertices: " << num_vertex <<  " num edges: " << num_edges << '\n';          
    
        if( boost::algorithm::ends_with(out, "el") )
            assert(writeEdgelist(out)==true);
        else if( boost::algorithm::ends_with(out, "bel") )
            assert(writeEdgelistBin(out)==true);
        else if( boost::algorithm::ends_with(out, "adj") )
            assert(writeAdj(out)==true);
        else if( boost::algorithm::ends_with(out, "mtx") )
            assert(writeMtx(out)==true);
        else if( boost::algorithm::ends_with(out, "b64") )
            assert(writeB64(out)==true);
        else
            std::cout << "unsupported output graph format" << '\n';
    }


    bool loadCSR(std::string input) {
        std::ifstream csr_file;
        csr_file.open(input, std::ios::binary);
        if(!csr_file.is_open()) {
            std::cout << "cannot open csr file!" << std::endl;
            return false;
        }

        csr_file.read(reinterpret_cast<char*>(&num_vertex), sizeof(unsigned));
        csr_file.read(reinterpret_cast<char*>(&num_edges), sizeof(unsigned));

        std::cout << "num vertex: " << num_vertex << std::endl;
        std::cout << "num edges: " << num_edges <<std::endl;

        csr_offset.resize(num_vertex);
        csr_index.resize(num_edges);

        csr_file.read(reinterpret_cast<char*>(csr_offset.data()), num_vertex * sizeof(unsigned));
        csr_file.read(reinterpret_cast<char*>(csr_index.data()), num_edges * sizeof(unsigned));

        csr_offset.push_back(num_edges);

        if(is_weighted) {
            std::vector<unsigned> local_wei(num_edges);
            csr_file.read(reinterpret_cast<char*>(local_wei.data()), num_edges * sizeof(unsigned));
            edge_weight = std::move(local_wei);
        }
    
        #pragma omp parallel for    
        for(unsigned v = 0; v < num_vertex; v++) {
            std::sort( csr_index.begin() + csr_offset[v], csr_index.begin() + csr_offset[v+1] - 1);
        }


        csr_file.close();
        return true;
    };

    bool loadMix(std::string input) {
        std::ifstream input_file (input, std::ios::binary);
        if(!input_file.is_open()) {
            std::cout << "cannot open the input mix file!" << '\n';
            return false;
        }


        input_file.read(reinterpret_cast<char*>(&num_vertex), sizeof(unsigned));
        input_file.read(reinterpret_cast<char*>(&num_edges), sizeof(unsigned));
        
        csr_offset.resize(num_vertex);
        csr_index.resize(num_edges); 
        csc_offset.resize(num_vertex);
        csc_index.resize(num_edges);

        input_file.read(reinterpret_cast<char*>(csr_offset.data()), num_vertex * sizeof(unsigned));
        input_file.read(reinterpret_cast<char*>(csr_index.data()), num_edges * sizeof(unsigned));

        input_file.read(reinterpret_cast<char*>(csc_offset.data()), num_vertex * sizeof(unsigned));
        input_file.read(reinterpret_cast<char*>(csc_index.data()), num_edges * sizeof(unsigned));

        input_file.close();

   //     out_degree.resize(num_vertex);
    //    in_degree.resize(num_vertex);
        #pragma omp parallel for    
        for(unsigned v = 0; v < num_vertex; v++) {
            std::sort( csr_index.begin() + csr_offset[v], csr_index.begin() + csr_offset[v+1] - 1);
            std::sort( csc_index.begin() + csc_offset[v], csc_index.begin() + csc_offset[v+1] - 1);
        }
        return true;
    }

    bool writeEdgelist(std::string out) {
        std::ofstream output(out);

        if(!output.is_open()) {
            std::cout << "cannot open the output file!" << std::endl;
            return false;
        }
        for(unsigned i = 0; i < num_vertex; i++) {
            for(unsigned j = csr_offset[i]; j < csr_offset[i+1]; j++) {
                output << i << " " << csr_index[j];
                if(is_weighted)
                    output << " " << edge_weight[j];
                output << '\n';
            }
        }
            output.close();
        std::cout << "the format of graph is been converted from CSR to Edgelist-text and stored in file: " << out << std::endl;
        return true;
    }

    bool writeEdgelistBin(std::string out) {
        FILE* fp = fopen(out.c_str(), "wb");
        if (fp == NULL)
        {
            fputs("file error", stderr);
            return false;
        }
        fwrite(&num_vertex, sizeof(unsigned), 1, fp); 
        fwrite(&num_edges, sizeof(unsigned), 1, fp); 

        for(unsigned i = 0; i < num_vertex; i++) {
            for(unsigned j = csr_offset[i]; j < csr_offset[i+1]; j++) {
                fwrite(&i, sizeof(unsigned), 1, fp); 
                fwrite(csr_index.data()+j, sizeof(unsigned), 1, fp); 
                if(is_weighted)
                    fwrite(edge_weight.data()+j, sizeof(unsigned), 1, fp); 
            }
        }
        fclose(fp);
        std::cout << "the format of graph is been converted from CSR to Edgelist-bin and stored in file: " << out << std::endl;
        return true;
    }

    bool writeAdj(std::string out) {
        std::ofstream output;
        output.open(out, std::ios::app);

        if(!output.is_open()) {
            std::cout << "cannot open the output file!" << std::endl;
            return false;
        }
        if(is_weighted)
            std::cout << "weighted graph" << '\n';
        if(is_weighted)
            output << "WeightedAdjacencyGraph" << '\n';
        else
            output << "AdjacencyGraph" << '\n';
        output << num_vertex << '\n';
        output << num_edges << '\n';
        for(unsigned i = 0; i < num_vertex; i++)
            output << csr_offset[i] << '\n';
        for(auto e: csr_index)
            output << e << '\n';
        if(is_weighted)
            for(auto w: edge_weight)
                output << w << '\n';
        output.close();
        std::cout << "the format of graph is been converted from CSR to (Ligra) Adjacency List and stored in file: " << out << std::endl;
        return true;
    }

    bool writeMtx(std::string out) {
        std::ofstream output(out);

        if(!output.is_open()) {
            std::cout << "cannot open the output file!" << std::endl;
            return false;
        }
        output << num_vertex << " " << num_vertex << " " << num_edges << '\n';
        for(unsigned i = 0; i < num_vertex; i++) {
            for(unsigned j = csr_offset[i]; j < csr_offset[i+1]; j++) {
                output << i+1 << " " << csr_index[j]+1;
                if(is_weighted)
                    output << " " << edge_weight[j];
                output << '\n';
            }
        }
            output.close();
        std::cout << "the format of graph is been converted from CSR to (GraphMat) MTX and stored in file: " << out << std::endl;
        return true;
    }

    bool writeB64(std::string out) {
        if(csc_index.size() == 0) {
            std::cout << "please convert from *.mix file" << '\n';
            return false;
        }
        std::string outpush = out + "-push";

        std::cout << "MIX->B64: csr->push " << outpush << std::endl;

        uint64_t nv = (uint64_t) num_vertex;
        uint64_t ne = (uint64_t) num_edges;

        std::vector<uint64_t> csr_offset64(csr_offset.begin(), csr_offset.end());
        std::vector<uint64_t> csr_index64(csr_index.begin(), csr_index.end());
        std::vector<uint64_t> csc_offset64(csc_offset.begin(), csc_offset.end());
        std::vector<uint64_t> csc_index64(csc_index.begin(), csc_index.end());

        FILE* fp = fopen(outpush.c_str(), "wb");
        if (fp == NULL)
        {
            fputs("push file error", stderr);
            return false;
        }
        fwrite(&nv, sizeof(uint64_t), 1, fp); 
        fwrite(&ne, sizeof(uint64_t), 1, fp); 

        for(uint64_t i = 0; i < num_vertex; i++) {
            for(uint64_t j = csr_offset64[i]; j < csr_offset64[i+1]; j++) {
                fwrite(&i, sizeof(uint64_t), 1, fp); 
                fwrite(csr_index64.data()+j, sizeof(uint64_t), 1, fp); 
                if(is_weighted)
                    fwrite(edge_weight.data()+j, sizeof(uint64_t), 1, fp); 
            }
        }
        fclose(fp);

        std::string outpull = out + "-pull";
        std::cout << "MIX->B64: csc->pull " << outpull << std::endl;

        fp = fopen(outpull.c_str(), "wb");
        if (fp == NULL)
        {
            fputs("pull file error", stderr);
            return false;
        }
        fwrite(&nv, sizeof(uint64_t), 1, fp); 
        fwrite(&ne, sizeof(uint64_t), 1, fp); 

        for(uint64_t i = 0; i < nv; i++) {
            for(uint64_t j = csc_offset64[i]; j < csc_offset64[i+1]; j++) {
                fwrite(csc_index64.data()+j, sizeof(uint64_t), 1, fp); 
                fwrite(&i, sizeof(uint64_t), 1, fp); 
                if(is_weighted)
                    fwrite(edge_weight.data()+j, sizeof(uint64_t), 1, fp); 
            }
        }
        fclose(fp);

        std::cout << "the format of graph is been converted from MIX to B64-push-pull: " << out << std::endl;

        return true;
    }

};
