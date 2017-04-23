#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <math.h>
#include <deque>

using namespace std;

int threshold;
vector<int> state, result, edgesList;

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

void printGraph(vector<vector<bool> >& graph, int nodes) {
    for (int i = 0; i < nodes; ++i) {
        for (int j = 0; j < nodes; ++j) {
            cout << graph[i][j] << " ";
        }
        cout << endl;
    }
}

int price(vector<vector<bool> >& graph, vector<int>& state, int stateSize) {
    int sameEdges = 0;
    int edges = 0;

    for (int i = 0; i < stateSize; ++i) {
        for (int j = i + 1; j < stateSize; ++j) {
            if ( graph[state[i]][state[j]] == 1 ) sameEdges++;
        }
        edges += edgesList[state[i]];
    }

    return edges - sameEdges * 2;
}

bool BBDFSSec(uint &a, uint &n, vector<vector<bool> >& graph, vector<int> state, uint stateSize, uint &minPrice, uint depth, vector<int> &result) {

    int firstNode;
    int tmpPrice;

    if ( depth < a ) {
        if ( depth == 0 ) {
            firstNode = 0;
        } else {
            firstNode = state[stateSize - 1] + 1;
        }
        for (int i = firstNode; i < n; ++i) {
            state.push_back(i);
            //tmpPrice = price(graph, state, stateSize + 1);
            if ( (stateSize + 1) == a ) {
                tmpPrice = price(graph, state, stateSize + 1);
                if (tmpPrice < minPrice) {
                    minPrice = tmpPrice;
                    result = state;
                }
                //cout << tmpPrice << ": " << state;
            }
            //if ( tmpPrice <= (minPrice + stateSize)) {
            BBDFSSec(a, n, graph, state, stateSize + 1, minPrice, depth + 1, result);
            //}
            state.pop_back();
        }
    } else {
        return false;
    }


    return true;
}

struct DFSState {
    uint depth;
    vector<int> state;
    uint stateSize;
    uint minPrice;
};

bool BBDFSPar(uint &a, uint &n, vector<vector<bool> >& graph, vector<int> state, uint stateSize, uint &minPrice, uint depth, vector<int> &result) {

    deque<DFSState> q;

    DFSState init;
    init.depth = depth;
    init.state = state;
    init.stateSize = stateSize;
    init.minPrice = minPrice;

    int firstNode;
    uint tmpPrice = minPrice;
    vector<int> tmpResult;

    q.push_back(init);

    while(q.size() < n*2) {
        DFSState current = q.front();
        q.pop_front();

        if ( current.depth == 0 ) {
            firstNode = 0;
        } else {
            firstNode = current.state[current.stateSize - 1] + 1;
        }

        //cout << "current " << current.state;

        for (int i = firstNode; i < n; ++i) {
            DFSState next;
            next.state = current.state;
            next.state.push_back(i);
            next.stateSize = current.stateSize + 1;
            next.depth = current.depth + 1;
            next.minPrice = minPrice;
            q.push_back(next);
            //cout << next.state;
        }

    }

    cout << "queue size = " << q.size() << endl;

    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < q.size(); ++i) {
        DFSState current = q[i];
        BBDFSSec(a, n, graph, current.state, current.stateSize, current.minPrice, current.depth, result);
            #pragma omp critical
            {
                if (current.minPrice < tmpPrice) {
                    tmpPrice = current.minPrice;
                    tmpResult = result;
                    cout << tmpPrice << ": " << tmpResult;
                }
            }
    }

    minPrice = tmpPrice;
    result = tmpResult;

    return 1;
}

bool BBDFSParTask(uint &a, uint &n, vector<vector<bool> >& graph, vector<int> state, uint stateSize, uint &minPrice, uint depth, vector<int> &result) {

    int firstNode;
    int tmpPrice;

    if ( depth < a ) {
        if ( depth == 0 ) {
            firstNode = 0;
        } else {
            firstNode = state[stateSize - 1] + 1;
        }
        for (int i = firstNode; i < n; ++i) {
            state.push_back(i);
            //tmpPrice = price(graph, state, stateSize + 1);
            if ( (stateSize + 1) == a ) {
                tmpPrice = price(graph, state, stateSize + 1);
                if (tmpPrice < minPrice) {
                #pragma omp critical
                    {
                        minPrice = tmpPrice;
                        result = state;
                    }
                }
                //cout << tmpPrice << ": " << state;
            }
            //if ( tmpPrice <= (minPrice + stateSize)) {
            #pragma omp task shared(a,n,graph,minPrice,result) if ( (a - depth) > threshold )
            BBDFSParTask(a, n, graph, state, stateSize + 1, minPrice, depth + 1, result);
            //}
            state.pop_back();
        }
    } else {
        return false;
    }

    return true;
}

int main(int argc, char const* argv[]) {

    if (argc < 5) {
        help();
        return 1;
    }

    uint n = strtoul(argv[1], NULL, 0);
    uint k = strtoul(argv[2], NULL, 0);
    uint a = strtoul(argv[4], NULL, 0);
    vector<vector<bool> > graph;
    vector<bool> graphLine;
    int nodes;
    string node;

    ifstream inputFile;
    inputFile.open(argv[3]);
    if (inputFile.is_open()) {
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
            if (graph[i][j] == 1) edges++;
        }
    }

    int tmpEdges = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (graph[i][j] == 1) tmpEdges++;
        }
        edgesList.push_back(tmpEdges);
        tmpEdges = 0;
    }

    threshold = a - floor(a/3);

    //cout << "threshold = " << threshold << endl;

    clock_t timeStart = clock();

    //BBDFSSec(a, n, graph, state, 0, edges, 0, result);
    BBDFSPar(a, n, graph, state, 0, edges, 0, result);

    double duration = (clock() - timeStart) / (double) CLOCKS_PER_SEC;

    cout << "n = " << result;
    cout << "edges = " << edges << endl;
    //cout << "time = " << duration << endl;

    inputFile.close();


    return 0;
}
