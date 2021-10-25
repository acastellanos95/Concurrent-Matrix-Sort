#include <iostream>
#include <vector>
#include "Utils.h"

int main(int argc, char *argv[]) {
    int MAX_THREAD;
    char *endptr;
    if(argc != 2)
        std::cout<< "Indique el número de hilos a ejecutar"<<std::endl;
    else
        MAX_THREAD = (int) strtol(argv[1], &endptr, 10);

//    printf("Leyendo archivo a memoria\n");
    std::vector<std::vector<float>> matrix;
    int size = readFileAndMergeSortRowsDiff<float>("matriz_matrix_size.txt", matrix, MAX_THREAD);
//    printf("Terminamos de cargar a memoria\n");

//    printf("Merge con hilos a filas\n");
    std::vector<float> finalMatrix;
    MergeRows<float>(matrix, MAX_THREAD, finalMatrix);
//    printf("Termina Merge con hilos a filas\n");

//    std::cout << finalMatrix.size() << std::endl;

//    printf("Liberar carga de memoria y guardar a archivo final");
    matrix.clear();
    saveToFile<float>("matriz_matrix_size-sort.txt", finalMatrix, size, MAX_THREAD);
    return 0;
}
