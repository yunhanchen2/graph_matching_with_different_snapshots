#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <iterator>
#include <set>
#include <pthread.h>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <random>
#include <unordered_set>

using namespace std;

class CSRGraph {
public:
    int node;
    int edge;
    int *col_indices;
    int *row_offsets;
    int *true_index;
    int **array;
    static int *query_list;
    int **array_origin;
    static int max_index;

    static int ** ss_neighbor_number;
    static vector <int> ss_col_indices;
    static int * ss_row_offsets;


    CSRGraph();
    static bool compare(const int* a, const int* b);
    void ReadTheGraph(char *pathname);
    void removeDuplicates();
    void GetFourArray();
    void Clear();
    void Generate_Snapshots();
    vector<int> generateUniqueRandomInts(int n, int m);
    int check_neighbor(int a,int b,int index_of_snapshot);
    static vector <int> GetTheNeighbor(int index_of_snapshot,int a);
};

