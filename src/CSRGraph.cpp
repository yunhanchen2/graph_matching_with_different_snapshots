#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <iterator>
#include <set>
#include <time.h>
#include <pthread.h>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <random>
#include <unordered_set>
#include "../include/CSRGraph.h"

using namespace std;

CSRGraph::CSRGraph() {
    node=0;
    edge=0;
}



void CSRGraph::ReadTheGraph(char *pathname){
    ifstream fin;
    fin.open(pathname);//n edges and m nodes
    fin >> edge >> node;
    edge = edge * 2;

    array = new int *[edge];  // 动态分配二维数组的行
    for (int i = 0; i < edge; ++i) {
        array[i] = new int[2];  // 动态分配二维数组的列
    }

    array_origin = new int *[edge];  // 动态分配二维数组的行
    for (int i = 0; i < edge; ++i) {
        array_origin[i] = new int[2];  // 动态分配二维数组的列
    }

    col_indices = new int[edge]();
    row_offsets = new int[node + 1]();
    ss_row_offsets=new int[node+1]();

    ss_neighbor_number=new int*[16]();
    for(int i=0;i<16;i++){
        ss_neighbor_number[i]=new int[node]();
    }
    true_index = new int[edge]();

    for (int i = 0; i < edge; ++i) {
        int m1, n1;
        fin >> m1 >> n1;

        array[i][0] = m1;
        array[i][1] = n1;
        array_origin[i][0] = m1;
        array_origin[i][1] = n1;
        i++;
        array[i][0] = n1;
        array[i][1] = m1;
        array_origin[i][0] = n1;
        array_origin[i][1] = m1;
    }

    fin.close();

    sort(array, array + edge, compare);

    query_list = new int[array[edge - 1][0] + 1]();
    max_index=array[edge - 1][0];

}

bool CSRGraph::compare(const int* a, const int* b) {
    if (abs(a[0]) != abs(b[0])) {
        return abs(a[0]) < abs(b[0]);  // 按照第一个数从小到大排列
    }
    return abs(a[1]) < abs(b[1]);  // 对于相同的第一个数，按照第二个数从小到大排列
}


void CSRGraph::removeDuplicates() {
    int newSize = 0;
    for (int i = 0; i < edge; i++) {
        if (i == 0 || !(array[i][0] == array[i - 1][0] && array[i][1] == array[i - 1][1])) {  // 如果当前元素不等于前一个元素，则将其保留
            array[newSize][0] = array[i][0];
            array[newSize][1] = array[i][1];
            newSize++;
        }
    }

    edge = newSize;  // 更新数组大小
}

int CSRGraph::check_neighbor(int a,int b,int index_of_snapshot){
    int ret =0;
    vector <int> neighbor= GetTheNeighbor(index_of_snapshot,a);
    for(int i=0;i<neighbor.size();i++){
        if(neighbor[i]==b){
            ret=1;
        }
    }
    return ret;
}

void CSRGraph::GetFourArray(){
    for (int i = 0; i < edge; i++) {
        true_index[i] = array[i][0];
    }

    //get the true index
    int slow = 0; // 慢指针
    for (int fast = 1; fast < edge; fast++) { // 快指针
        if (true_index[fast] != true_index[slow]) {
            slow++;
            true_index[slow] = true_index[fast]; // 将非重复元素移到慢指针位置
        }
    }

    for (int i = 1; i < node + 1; i++) {
        query_list[true_index[i - 1]] = i;//已知真正的编号 要找到对应是第几个（对应1~m)
    }

    for (int i = 0; i < edge; i++) {
        row_offsets[query_list[array[i][0]] - 1]++;
    }

    int temp = 0;
    int accumulate = 0;

    for (int i = 0; i < node; i++) {
        temp = row_offsets[i];
        row_offsets[i] = accumulate;
        accumulate += temp;
    }
    row_offsets[node] = accumulate;

    for (int i = 0; i < edge; i++) {
        col_indices[i] = array[i][1];
    }
}

void CSRGraph::Clear(){
    delete[] col_indices;
    delete[] row_offsets;
    delete[] true_index;

    for (int i = 0; i < node; i++) {
        delete [] array[i];
        delete [] array_origin[i];
    }
    delete [] array;
    delete [] array_origin;


    for(int i=0;i<16;i++){
        delete [] ss_neighbor_number[i];
    }
    delete [] ss_neighbor_number;

    delete [] ss_row_offsets;

    delete[] query_list;
}

vector<int> CSRGraph::generateUniqueRandomInts(int n, int m) {
    // 创建一个包含 0 到 n 范围内所有数字的向量
    vector<int> nums(n + 1);
    for (int i = 0; i < nums.size(); i++) {
        nums[i] = i;
    }

    // 使用 std::random_device 和 std::mt19937 随机打乱向量
    random_device rd;
    mt19937 gen(rd());
    shuffle(nums.begin(), nums.end(), gen);

    // 取出前 m 个元素
    vector<int> result(nums.begin(), nums.begin() + m);
    return result;
}

void CSRGraph::Generate_Snapshots(){
    //generate snapshot#0
    int percent=10;
    vector <int> random_0= generateUniqueRandomInts((edge/2)-1,(edge/2)/percent);  //****
    int size=(edge/2)/percent;  //****

    int **array_0=new int *[size*2];
    for (int i = 0; i < size*2; ++i) {
        array_0[i] = new int[2];
    }

    for(int i=0;i<size;i++){
        array_0[i*2][0]=array_origin[random_0[i]*2][0];
        array_0[i*2][1]=array_origin[random_0[i]*2][1];
        array_0[i*2+1][0]=array_origin[random_0[i]*2][1];
        array_0[i*2+1][1]=array_origin[random_0[i]*2][0];
    }

    sort(array_0, array_0 + size*2, compare);

        //get the ss_neighbor_number
    for(int i=0;i<size*2;i++){
        ss_neighbor_number[0][query_list[array_0[i][0]]-1]++; //&&&&
    }

    int temp = 0;
    int accumulate = 0;
    for (int i = 0; i < node; i++) {
        temp = ss_neighbor_number[0][i];      //&&&&
        ss_row_offsets[i] = accumulate;      //&&&&
        accumulate += temp;
    }
    ss_row_offsets[node] = accumulate;      //&&&&

        //get the ss_col_indices
    ss_col_indices.resize(size*2,0);
    for (int i = 0; i < size*2; i++) {
        ss_col_indices[i] = array_0[i][1];
    }

    //deleting
    for (int i = 0; i < size*2; i++) {
        delete [] array_0[i];
    }
    delete [] array_0;



    //generate snapshot #i --- totally 16 snapshots
    int add_delete_percent=10;
    int number_of_add=size/add_delete_percent;  //****
    int number_of_delete=size/add_delete_percent; //number是边的条数，没有做无向处理  ****


    for(int r=1;r<16;r++){
        vector <int> random_change=generateUniqueRandomInts((edge/2)-1,edge/2);

        int **array_change=new int*[number_of_add*4];
        for(int i=0;i<number_of_add*4;i++){
            array_change[i]=new int[2];
        }


        //generate the changing number
        int add_record=0;
        int delete_record=0;
        int ptr_of_random=0;
        int array_ptr=0;

        while(add_record<number_of_add&&delete_record<number_of_delete){

            if(check_neighbor(array_origin[random_change[ptr_of_random]*2][0],array_origin[random_change[ptr_of_random]*2][1],r-1)){ //delete
                array_change[array_ptr][0]=-array_origin[random_change[ptr_of_random]*2][0];
                array_change[array_ptr][1]=-array_origin[random_change[ptr_of_random]*2][1];
                array_ptr++;
                array_change[array_ptr][0]=-array_origin[random_change[ptr_of_random]*2][1];
                array_change[array_ptr][1]=-array_origin[random_change[ptr_of_random]*2][0];
                delete_record++;
                array_ptr++;
            } else {
                array_change[array_ptr][0]=array_origin[random_change[ptr_of_random]*2][0];
                array_change[array_ptr][1]=array_origin[random_change[ptr_of_random]*2][1];
                array_ptr++;
                array_change[array_ptr][0]=array_origin[random_change[ptr_of_random]*2][1];
                array_change[array_ptr][1]=array_origin[random_change[ptr_of_random]*2][0];
                add_record++;
                array_ptr++;
            }
            ptr_of_random++;
        }
        //处理未分配完的
        while(add_record<number_of_add){
            if(!check_neighbor(array_origin[random_change[ptr_of_random]*2][0],array_origin[random_change[ptr_of_random]*2][1],r-1)) { //delete
                array_change[array_ptr][0] = array_origin[random_change[ptr_of_random] * 2][0];
                array_change[array_ptr][1] = array_origin[random_change[ptr_of_random] * 2][1];
                array_ptr++;
                array_change[array_ptr][0] = array_origin[random_change[ptr_of_random] * 2][1];
                array_change[array_ptr][1] = array_origin[random_change[ptr_of_random] * 2][0];
                array_ptr++;
                add_record++;
            }
            ptr_of_random++;
        }

        while (delete_record<number_of_delete){
            if(check_neighbor(array_origin[random_change[ptr_of_random]*2][0],array_origin[random_change[ptr_of_random]*2][1],r-1)){ //delete
                array_change[array_ptr][0]=-array_origin[random_change[ptr_of_random]*2][0];
                array_change[array_ptr][1]=-array_origin[random_change[ptr_of_random]*2][1];
                array_ptr++;
                array_change[array_ptr][0]=-array_origin[random_change[ptr_of_random]*2][1];
                array_change[array_ptr][1]=-array_origin[random_change[ptr_of_random]*2][0];
                array_ptr++;
                delete_record++;
            }
            ptr_of_random++;
        }


        //record the change and get a new row_offset
        sort(array_change, array_change + array_ptr, compare);


        int * ttemp=new int[abs(array_change[number_of_add*4-1][0])+1]();
        int ssize=0;
        for(int i=0;i<number_of_add*4;i++){
            if(ttemp[abs(array_change[i][0])]==0){
                ssize++;
            }
            ttemp[abs(array_change[i][0])]++;
        }

        int * edge_change=new int[ssize]();
        int * numFrequency=new int[ssize]();
        int ptr=0;
        for(int i=0;i<abs(array_change[number_of_add*4-1][0])+1;i++){
            if(ttemp[i]!=0){
                edge_change[ptr]=i;
                numFrequency[ptr]=ttemp[i];
                ptr++;
            }
        }

        int * changing_row_offset=new int[ssize+1]();
        int acc=0;
        for(int i=0;i<ssize;i++){
            changing_row_offset[i]=acc;
            acc+=numFrequency[i];
        }
        changing_row_offset[ssize]=acc;


        //deleting
        delete [] ttemp;
        delete [] numFrequency;


        //insert and create a new row_offset
        int change_ptr=0;   //point to the number of changing
        int add_parameter=0;  //当有插入，后面每一个row_offset的值，不论有没有新的插入都先加上一个增加变量因子
        ss_col_indices.resize(size*2+number_of_add*4*r);

        for(int i=0;i<node;i++){
            ss_row_offsets[i]=ss_row_offsets[i]+add_parameter;
            int adder=0;

            if(true_index[i]==edge_change[change_ptr]) {

                //将变化的插入到col_indices中
                int position = ss_row_offsets[i + 1]+add_parameter;
                for (int j = changing_row_offset[change_ptr]; j < changing_row_offset[change_ptr + 1]; j++) {
                    int insert_num = array_change[j][1];
                    ss_col_indices.insert(ss_col_indices.begin() + position, insert_num);
                    position++;
                    add_parameter++;
                    adder++;
                }
                change_ptr++;
            }

            ss_neighbor_number[r][i]=ss_neighbor_number[r-1][i]+adder;
        }

        ss_row_offsets[node]=ss_row_offsets[node]+add_parameter;

        ss_col_indices.resize(size*2+number_of_add*4*r);


        //deleting
//    random_change.clear();
        for (int i = 0; i < number_of_add*4; i++) {
            delete [] array_change[i];
        }
        delete [] array_change;

        delete [] edge_change;
        delete [] changing_row_offset;

    }


}

vector <int> CSRGraph::GetTheNeighbor(int index_of_snapshot,int a){
    vector <int> neighbor;
    int * temp=new int[max_index+1]();
    for(int i=ss_row_offsets[query_list[a]-1];i<ss_row_offsets[query_list[a]-1]+ss_neighbor_number[index_of_snapshot][query_list[a]-1];i++){
        if(ss_col_indices[i]>0){
            temp[abs(ss_col_indices[i])]++;
        } else {
            temp[abs(ss_col_indices[i])]--;
        }
    }
    int size=0;
    for(int i=0;i<max_index+1;i++){

        if(temp[i]>0){
            size++;
            neighbor.push_back(i);
        }
    }
    neighbor.resize(size);


    delete [] temp;
    return neighbor;
}

int *CSRGraph::query_list = nullptr;
int CSRGraph::max_index = 0;
int *CSRGraph::ss_row_offsets = nullptr;
int **CSRGraph::ss_neighbor_number = nullptr;
std::vector<int> CSRGraph::ss_col_indices;