#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <fstream>

using namespace std;

void help() {
    cout << "spouštění: ./main n k graphFile a" << endl << endl;
    cout << "n = přirozené číslo představující počet uzlů grafu G, n >= 10" << endl;
    cout << "k = přirozené číslo řádu jednotek představující průměrný stupeň uzlu grafu G, n >= k >= 3" << endl;
    cout << "graphFile = G(V,E) = jednoduchý souvislý neorientovaný neohodnocený graf o n uzlech a průměrném stupni k" << endl;
    cout << "a = přirozené číslo, 5 =< a =< n/2" << endl;
}

void printGraph(vector<vector<bool>> graph, int nodes) {
    for (int i = 0; i < nodes; ++i) {
        for (int j = 0; j < nodes; ++j) {
            cout << graph[i][j] << ", ";
        }
        cout << endl;
    }
}



int main(int argc, char const* argv[]) {

    if ( argc < 5 ) {
        help();
        return 1;
    }

    uint n = strtoul (argv[1], NULL, 0);
    uint k = strtoul (argv[2], NULL, 0);
    uint a = strtoul (argv[4], NULL, 0);
    vector<vector<bool> > graph;
    vector<bool> graphLine;
    int nodes;
    string node;

    ifstream inputFile;
    inputFile.open(argv[3]);
    if ( inputFile.is_open() ) {
        inputFile >> dec >> nodes;
        for (int i = 0; i < nodes; ++i) {
            inputFile >> node;
            for (int j = 0; j < nodes; ++j) {
                if (node[j] == '0') {
                    graphLine.push_back(0);
                } else {
                    graphLine.push_back(1);
                }
            }
            graph.push_back(graphLine);
            graphLine.clear();
        }
    } else {
        cerr << "chyba při otevírání souboru" << endl;
    }

    cout << "n = " << n << ", k = " << k << ", a = " << a << endl;
    printGraph(graph, nodes);

    inputFile.close();


    return 0;
}