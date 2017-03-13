#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <set>
#include <fstream>

using namespace std;

void help() {
    cout << "spouštění: ./main n k graphFile a" << endl << endl;
    cout << "n = přirozené číslo představující počet uzlů grafu G, n >= 10" << endl;
    cout << "k = přirozené číslo řádu jednotek představující průměrný stupeň uzlu grafu G, n >= k >= 3" << endl;
    cout << "graphFile = G(V,E) = jednoduchý souvislý neorientovaný neohodnocený graf o n uzlech a průměrném stupni k" << endl;
    cout << "a = přirozené číslo, 5 =< a =< n/2" << endl;
}

ostream& operator<<(ostream& os, vector<int>& graph)
{
    os << '{';
    for (int j = 0; j < graph.size(); ++j) {
        if (j != graph.size() - 1) {
            os << graph[j] << ", ";
        } else {
            os << graph[j];
        }
    }
    os << "}" << endl;

    return os;
}

void printGraph(vector<vector<bool>> graph, int nodes) {
    for (int i = 0; i < nodes; ++i) {
        for (int j = 0; j < nodes; ++j) {
            cout << graph[i][j] << " ";
        }
        cout << endl;
    }
}

int price(vector<vector<bool>>& graph, vector<int> state) {
    int sameEdges = 0;
    int edges = 0;

    for (int i = 0; i < state.size(); ++i) {
        for (int j = 0; j < graph.size(); ++j) {
            if ( graph[state[i]][j] == 1 ) edges++;
        }
        for (int j = i + 1; j < state.size(); ++j) {
            if ( graph[state[i]][state[j]] == 1 ) sameEdges++;
        }
    }

    return edges - sameEdges * 2;
}

bool BBDFS(uint a, vector<vector<bool>> graph, vector<int> state, uint &minPrice, uint depth, vector<int> &result) {

    int firstNode;
    int tmpPrice;

    if ( depth < a ) {
        if ( depth == 0 ) {
            firstNode = 0;
        } else {
            firstNode = state[state.size() - 1] + 1;
        }
        for (int i = firstNode; i < graph.size(); ++i) {
            state.push_back(i);
            tmpPrice = price(graph, state);
            if ( state.size() == a ) {
                tmpPrice = price(graph, state);
                if (tmpPrice < minPrice) {
                    minPrice = tmpPrice;
                    result = state;
                }
                //cout << tmpPrice << ": " << state;
            }
            if ( tmpPrice <= minPrice ) {
                BBDFS(a, graph, state, minPrice, depth + 1, result);
            }
            state.pop_back();
        }
    } else {
        return false;
    }


    return true;
}

int main(int argc, char const* argv[]) {

    if ( argc < 5 ) {
        help();
        return 1;
    }

    uint n = strtoul (argv[1], NULL, 0);
    uint k = strtoul (argv[2], NULL, 0);
    uint a = strtoul (argv[4], NULL, 0);
    vector<vector<bool>> graph;
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
        return 0;
    }

    cout << "n = " << n << ", k = " << k << ", a = " << a << endl;
    //printGraph(graph, nodes);

    uint edges = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < i; ++j) {
            if ( graph[i][j] == 1 ) edges++;
        }
    }
    vector<int> state, result;
    clock_t timeStart = clock();
    BBDFS(a, graph, state, edges, 0, result);
    double duration = ( clock() - timeStart ) / (double) CLOCKS_PER_SEC;

    cout << "n = " << result;
    cout << "edges = " << edges << endl;
    cout << "time = " << duration << endl;

    inputFile.close();


    return 0;
}
