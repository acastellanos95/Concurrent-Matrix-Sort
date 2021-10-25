#include <iostream>
#include "Utils.h"

int main(int argc, char *argv[]) {
    int MAX_THREAD;
    char *endptr;
    if (argc == 2){
        MAX_THREAD = strtol(argv[1], &endptr, 10);
    }else {
        std::cout<< "Indique el número de hilos a ejecutar"<<std::endl;
        exit(EXIT_FAILURE);
    }
//    std::cout << "Inicio de merge sort a filas" << std::endl;
    auto [size] = readFileAndMergeSortRowsDiff<float>("matriz_matrix_size.txt", "matrix_a_mergesort_rows.dat", MAX_THREAD);
//    std::cout << "Término de merge sort a filas" << std::endl;
//    std::cout << "Inicio de k ways merge a filas para matriz" << std::endl;
    readFileAndMergeRows<float>("matrix_a_mergesort_rows.dat", "matriz_matrix_size-sort.txt", size, MAX_THREAD);
//    std::cout << "Término de k ways merge a filas para matriz" << std::endl;
    return 0;
}
