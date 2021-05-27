/**
 *

 */
#include <pthread.h>
#include <time.h>
#include "graph.hpp"
#include "parser.hpp"

using namespace std;

//////////////////////////////////////////
//main function
//////////////////////////////////////////
int main(int argc, char** argv)
{
    if (argc != 4)
    {
        cout << "./convertor <binary_csr_file> <text_output_file> <format_option>" << '\n';
        cout << "format_option: 0--text edgelist, 1--binary edgelist, 2--text adjacency, 3--binary adjacency" << '\n';
        exit(1);
    }
    string data_file = argv[1];
    string output = argv[2];
    // format = Format::csr;
    Format format = static_cast<Format>((unsigned int)atoi(argv[3]));
    Graph graph;

    if(parseGraph(data_file, graph)) {
        graph.printGraph();
    } else {
        cout << "failed to load the graph" << '\n';
    }
    
    //////////////////////////////////////////
    // write csr file
    //////////////////////////////////////////
    writeGraph(output, graph, format);
    
}
