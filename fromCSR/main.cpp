/**
 *

 */
#include <pthread.h>
#include <time.h>
#include "graph.hpp"

using namespace std;


//////////////////////////////////////////
//main function
//////////////////////////////////////////
int main(int argc, char** argv)
{
    if (argc != 4 )
    {
        cout << "./convert <input_file> <output_file> <is_weight?>" << '\n';
        cout << "input format: [1] *.csr (GPOP); [2] *.mix (Mixen)" << '\n'; 
        cout <<  "output format: [1] *.el (text edgelist - Snap) ; [2] *.bel (binary edgelist) ; [3] *.adj (Ligra) ; [4] *.mtx  (GraphMat) ; [5] *.b64 push+pull (Grazelle *must be used with *.mix)" << '\n';
        exit(1);
    }

    string input_file = argv[1];
    string output_file = argv[2];
    // format = Format::csr;
    bool is_w = false;
    if( 1 == (unsigned int)atoi(argv[3]))
        is_w = true;
    Graph graph(is_w);

    graph.convert(input_file, output_file);
    
}
