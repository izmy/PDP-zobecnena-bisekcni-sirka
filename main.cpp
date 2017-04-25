#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <math.h>
#include <deque>
#include <mpi.h>

static const int tag_done = 0;
static const int tag_donee = 3;
static const int tag_work = 1;
static const int tag_finished = 2;

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

bool BBDFSPar(uint &a, uint &n, vector<vector<bool> >& graph, vector<int> state, uint stateSize, uint &minPrice, uint depth, vector<int> &result, const int num_procs, const int proc_num) {

    deque<int> paralelArray;
    int paralelResult;
    int msg;
    vector<int> resultVector(a);

    MPI_Status status;
    if(proc_num == 0) {

        cout << proc_num << ": We have " << num_procs << " processors" << endl;

        for (int i = 0; i < n; ++i) {
            paralelArray.push_back(i);
        }

        for (int dest = 1; dest < num_procs; dest++) {
            MPI_Send(&paralelArray.front(), 1, MPI_INT, dest, tag_work, MPI_COMM_WORLD);
            paralelArray.pop_front();
        }

        int working_slaves = num_procs - 1;
        while (working_slaves > 0) {
            MPI_Recv(&paralelResult, 1, MPI_INT, MPI_ANY_SOURCE, tag_done, MPI_COMM_WORLD, &status);
            MPI_Recv(&resultVector[0], a, MPI_INT, MPI_ANY_SOURCE, tag_donee, MPI_COMM_WORLD, &status);
            if (paralelResult < minPrice ) {
                minPrice = paralelResult;
                result = resultVector;
            }
            if(paralelArray.size() > 0) {
                MPI_Send(&paralelArray.front(), 1, MPI_INT, status.MPI_SOURCE, tag_work, MPI_COMM_WORLD);
                paralelArray.pop_front();
            } else {
                MPI_Send(&paralelArray.front(), 1, MPI_INT, status.MPI_SOURCE, tag_finished, MPI_COMM_WORLD);
                working_slaves--;
            }
        }

    } else {
        while(true) {
            MPI_Recv(&msg, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            //cout << "slave msg: " << msg << ", mpi_tag " << status.MPI_TAG << endl;
            if (status.MPI_TAG == tag_finished) break;
            else if (status.MPI_TAG == tag_work) {

                int firstNode;
                uint tmpPrice = minPrice;
                vector<int> tmpResult(a);

                deque<DFSState> q;

                DFSState init;
                init.depth = 1;
                init.state.push_back(msg);
                init.stateSize = 1;
                init.minPrice = minPrice;

                q.push_back(init);

                //cout << "co mam ve fronte " << init.state;

                while (q.size() < n) {
                    DFSState current = q.front();

                    firstNode = current.state[current.stateSize - 1] + 1;
                    //cout << "|" << proc_num << "| firstNode " << firstNode << ", " << current.state;
                    if (firstNode >= n) break;

                    for (int i = firstNode; i < n; ++i) {
                        DFSState next;
                        next.state = current.state;
                        next.state.push_back(i);
                        next.stateSize = current.stateSize + 1;
                        next.depth = current.depth + 1;
                        next.minPrice = minPrice;
                        q.push_back(next);

                    }
                    q.pop_front();
                }

//                for (int i = 0; i < q.size(); ++i) {
//                    DFSState current = q[i];
//                    cout << "|" << proc_num << "| " << current.state;
//                }

                #pragma omp parallel for schedule(dynamic)
                for (int i = 0; i < q.size(); ++i) {
                    DFSState current = q[i];
                    BBDFSSec(a, n, graph, current.state, current.stateSize, current.minPrice, current.depth, result);
                    #pragma omp critical
                    {
                        if (current.minPrice < tmpPrice) {
                            tmpPrice = current.minPrice;
                            tmpResult = result;
                            //cout << "|" << proc_num << "| size: " << q.size() << ", price: " << tmpPrice << ": " << tmpResult;
                        }
                    }
                }

                //cout << "|" << proc_num << "| " << tmpPrice << ": " << tmpResult;

                MPI_Send(&tmpPrice, 1, MPI_UNSIGNED, 0, tag_done, MPI_COMM_WORLD);
                MPI_Send(&tmpResult[0], a, MPI_INT, 0, tag_donee, MPI_COMM_WORLD);
            }
        }
    }

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

int main(int argc, char * argv[]) {

    if (argc < 5) {
        help();
        return 1;
    }

    int provided;
    int required = MPI_THREAD_FUNNELED;
    MPI_Init_thread(&argc, &argv, required, &provided);

    if (provided < required) {
        throw runtime_error("MPI library does not provide required threading support");
    }

    int num_procs;
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    int proc_num;
    MPI_Comm_rank(MPI_COMM_WORLD, &proc_num);

    uint n = strtoul(argv[1], NULL, 0);
    uint k = strtoul(argv[2], NULL, 0);
    uint a = strtoul(argv[4], NULL, 0);
    vector<vector<bool> > graph;
    vector<bool> graphLine;
    int nodes;
    string node;
    double t1, t2;

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

    if (proc_num == 0) {
        cout << "n = " << n << ", k = " << k << ", a = " << a << endl;
        //printGraph(graph, nodes);
    }

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

    t1 = MPI_Wtime();
    //BBDFSSec(a, n, graph, state, 0, edges, 0, result);
    BBDFSPar(a, n, graph, state, 0, edges, 0, result, num_procs, proc_num);
    t2 = MPI_Wtime();

    if (proc_num == 0) {
        cout << "n = " << result;
        cout << "edges = " << edges << endl;
    }

    printf ("%d: Elapsed time is %f.\n",proc_num,t2-t1);

    inputFile.close();

    MPI_Finalize();
}
