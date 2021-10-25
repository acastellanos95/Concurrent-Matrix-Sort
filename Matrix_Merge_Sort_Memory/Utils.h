//
// Created by andre on 10/20/21.
//

#ifndef MATRIX_MERGE_SORT_MEMORY_UTILS_H
#define MATRIX_MERGE_SORT_MEMORY_UTILS_H

#include <vector>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <ios>
#include <fstream>
#include <sstream>
#include <thread>
#include <atomic>
#include <mutex>
#include <filesystem>
#include <queue>

/*Plantilla de algoritmo merge-sort usando iteradores.*/
template<class Iter>
void merge_sort(Iter first, Iter last)
{
    if (std::distance(first, last) > 1) {
        Iter middle = first + (last - first) / 2;
        merge_sort(first, middle);
        merge_sort(middle, last);
        std::inplace_merge(first, middle, last);
    }
}

/*K ways merge*/
template<typename T>
void mergeKArrays(std::vector<std::vector<T>> &v,long i,long j, std::vector<T> &output)
{
    //si queda solo uno
    if(i==j)
    {
        for(size_t p=0; p < v[0].size(); p++)
            output.push_back(v[i][p]);
        return;
    }

    //si son dos filas
    if(j-i==1)
    {
//        mergeArrays(arr[i],arr[j],n,n,output);
        std::merge(v[i].begin(), v[i].end(), v[j].begin(), v[j].end(), std::back_inserter(output));
        return;
    }

    //vectores auxiliares
    std::vector<T> out1,out2;

    //divide en dos
    mergeKArrays(v,i,(i+j)/2,out1);
    mergeKArrays(v,(i+j)/2+1,j,out2);

    //mezclar los vectores auxiliares al vector de salida
    std::merge(out1.begin(), out1.end(), out2.begin(), out2.end(), std::back_inserter(output));
}

/*K ways merge*/
template<typename T>
void mergeKArraysHeap(std::vector<std::vector<T>> &v, int i, int j, std::vector<T> &output)
{
    auto cmp = [](std::vector<T> a, std::vector<T> b)
    {
        return a[0] > b[0];
    };

    std::priority_queue<std::vector<T>, std::vector<std::vector<T>>, decltype(cmp)> pq(cmp);

    for (int rowIndex = i; rowIndex <= j; ++rowIndex) {
        pq.push(v[rowIndex]);
    }

    while (!pq.empty())
    {
        std::vector<T> auxRow(pq.top());
        pq.pop();
        output.push_back(auxRow[0]);
        auxRow.erase(auxRow.begin());
        if(!auxRow.empty())
            pq.push(auxRow);
    }
}

template<typename T>
int readFileAndMergeSortRowsDiff(const std::string &input_filename, std::vector<std::vector<T>> &v, const int &num_threads){
    // Tamaño de la matriz
    int matrixSize = 0;
    // vector auxiliar para fila
    std::vector<T> auxRow;
    // Leer el archivo
    std::ifstream inputFile(input_filename, std::ifstream::in);
    std::string line;
    if(inputFile.is_open()){
        while(std::getline(inputFile, line)){
            std::istringstream lineStream(line);
            std::string cell;
            auxRow.clear();
            matrixSize = 0;
            while(lineStream >> cell)
            {
                char *ptr;
                if(std::is_same<T, int>::value || std::is_same<T, long>::value || std::is_same<T, long long>::value){
                    auxRow.push_back((T) strtol(cell.c_str(), &ptr, 10));
                }else if (std::is_same<T, double>::value || std::is_same<T, float>::value || std::is_same<T, long double>::value) {
                    auxRow.push_back((T) strtod(cell.c_str(), &ptr));
                } else throw std::runtime_error("El tipo T no es soportado");
                matrixSize++;
            }
            // Hilos y pedazos de vectores para hilos
            std::vector<std::thread> threads(num_threads);
            std::vector<std::vector<T>> auxIRow(num_threads);
            // Crear hilos
            for (int i = 0; i < num_threads; ++i) {
                // inicializamos los hilos con una función anónima que le pasamos el tamaño de la matriz para actualizar
                threads[i] = std::thread([i, matrixSize, num_threads, &auxRow, &auxIRow](){
                    int lengthPart = auxRow.size()/num_threads;
                    int begin = (i*lengthPart);
                    int end = (i*lengthPart) + lengthPart;
                    typename std::vector<T>::iterator beginIt = auxRow.begin() + begin;
                    typename std::vector<T>::iterator endIt;
                    if(i == num_threads - 1){
                        endIt = auxRow.end();
                    } else{
                        endIt = auxRow.begin() + end;
                    }
                    auxIRow[i]=std::vector<T>(beginIt, endIt);
                    // Ordenar el pedazo de arreglo de una fila en la fila i  de la matriz auxiliar para el hilo i
                    merge_sort<typename std::vector<T>::iterator>(auxIRow[i].begin(), auxIRow[i].end());
                });
            }
            // Join hilos hasta num_threads
            for (int i = 0; i < num_threads; ++i) {
                threads[i].join();
            }
            auxRow.clear();
            // mezclar la matriz auxiliar de pedazos del arreglo de la fila
            mergeKArrays(auxIRow, 0, auxIRow.size() - 1, auxRow);
            // Guardar en la matriz final
            v.push_back(auxRow);
        }
    }
    inputFile.close();
    return matrixSize;
}

template<typename T>
int readFromCSVAndPushToSquareMatrix(const std::string &input_filename, std::vector<std::vector<T>> &v)
{
    std::ifstream file(input_filename, std::ifstream::in);
    std::string line;
    std::vector<T> row;
    int size = 0;
    if(file.is_open()){
        while(std::getline(file, line)){
            size++;
            row.clear();
            std::istringstream lineStream(line);
            std::string cell;
            while(lineStream >> cell)
            {
                char *ptr;
                if(std::is_same<T, int>::value || std::is_same<T, long>::value){
                    row.push_back((T) strtol(cell.c_str(), &ptr, 10));
                }else if (std::is_same<T, double>::value || std::is_same<T, float>::value) {
                    row.push_back((T) strtod(cell.c_str(), &ptr));
                } else throw std::runtime_error("El tipo T no es soportado");
            }
            v.push_back(row);
        }
    }
    file.close();
    return size;
};

template<typename T>
void MergeSortRows(std::vector<std::vector<T>> &v, const int num_threads){
    // Hilos
    std::vector<std::thread> threads(num_threads);
    // Crear hilos
    for (int i = 0; i < num_threads; ++i) {
        // inicializamos los hilos con una función anónima que le pasamos el tamaño de la matriz para actualizar
        threads[i] = std::thread([i, &v, &num_threads]() {
            // Tamaño de pedazo
            int lengthPart = v.size() / num_threads;
            // Empieza en 0 de lo contrario en i*tamaño de pedazo + 1
            int begin = (i == 0) ? 0 : (i * lengthPart) + 1;
            // Donde termina??
            int end = (i == num_threads - 1) ? v.size() - 1 : (i * lengthPart) + lengthPart;
//            std::cout << "begin: " << begin << " end: " << end << '\n';
            // Iteremos y hagamos mergesort a cada fila que toca al hilo
            for(int row = begin; row <= end; ++row){
                merge_sort<typename std::vector<T>::iterator>(v[row].begin(), v[row].end());
            }
        });
    }
    // Join hilos hasta num_threads
    for (int i = 0; i < num_threads; ++i) {
        threads[i].join();
    }
}

template<typename T>
void MergeRows(std::vector<std::vector<T>> &v, const int num_threads, std::vector<T> &finalMatrix){
    // Hilos
    std::vector<std::thread> threads(num_threads);
    // matriz auxiliar
    std::vector<std::vector<T>> auxMatrix(num_threads);

    // Crear hilos
    for (int i = 0; i < num_threads; ++i) {
        // inicializamos los hilos con una función anónima que le pasamos el tamaño de la matriz para actualizar
        threads[i] = std::thread([i, &v, &num_threads, &auxMatrix]() {
            // Tamaño de pedazo
            int lengthPart = v.size() / num_threads;
            // Empieza en 0 de lo contrario en i*tamaño de pedazo + 1
            int begin = (i == 0) ? 0 : (i * lengthPart) + 1;
            // Donde termina??
            int end = (i == num_threads - 1) ? (v.size() - 1) : (i * lengthPart) + lengthPart;

//            std::cout << "begin: " << begin << " end: " << end << " size: " << v.size()*((end + 1) - begin) << '\n';

            // hagamos mergesort a cada fila que toca al hilo
            mergeKArrays<T>(v, begin, end, auxMatrix[i]);
//            mergeKArraysHeap<T>(v, begin, end, auxMatrix[i]);
        });
    }
    // Join hilos hasta num_threads
    for (int i = 0; i < num_threads; ++i) {
        threads[i].join();
    }

        mergeKArrays<T>(auxMatrix, 0, (num_threads - 1), finalMatrix);
//    mergeKArraysHeap<T>(auxMatrix, 0, (num_threads - 1), finalMatrix);
}

template<typename T>
void saveToFile(const std::string output_filename, std::vector<T> &flat_matrix, int size_to_fit, int num_threads_file_iterator){
    std::ofstream outputFile(output_filename, std::ofstream::out | std::ofstream::trunc);

    // Hilos para escribir a archivos y después solo hacer append
    std::vector<std::thread> fileThreads(num_threads_file_iterator);
    //Iniciamos hilos para escribir en num_threads_file_iterator archivos
    for (int file_index = 0; file_index < num_threads_file_iterator; ++file_index) {
        long lengthFilePart = (size_to_fit*size_to_fit)/num_threads_file_iterator;
        long begin = file_index == 0 ? 0 : (file_index*lengthFilePart + 1);
        long end = file_index == (num_threads_file_iterator - 1) ? size_to_fit*size_to_fit - 1 : (file_index*lengthFilePart) + lengthFilePart;
        // Crear hilos
        // inicializamos los hilos con una función anónima que le pasamos el tamaño de la matriz para actualizar
        fileThreads[file_index] = std::thread([begin, end, &flat_matrix, size_to_fit, file_index, output_filename](){
            std::ofstream outputFileSorted(output_filename + "sorted_" + std::to_string(file_index), std::ofstream::out | std::ofstream::trunc);
            for (auto col = begin; col <= end; ++col) {
                if(col != 0 && col%size_to_fit == 0){
                    outputFileSorted << '\n';
                }
                if(col >= size_to_fit*size_to_fit) break;
                outputFileSorted << " " << flat_matrix[col] << " ";
            }
            outputFileSorted.close();
        });
    }
    // Join hilos de archivo
    for (int file_index = 0; file_index < num_threads_file_iterator; ++file_index) {
        fileThreads[file_index].join();
    }

    // Guardar en archivos finales
    for (int file_index = 0; file_index < num_threads_file_iterator; ++file_index) {
        std::ifstream outputFileSorted(output_filename + "sorted_" + std::to_string(file_index), std::ifstream::in);
        std::stringstream contents;
        contents << outputFileSorted.rdbuf();
        outputFile << contents.str();
        outputFileSorted.close();
    }

    // Borrar archivos temporales
    for (int file_index = 0; file_index < num_threads_file_iterator; ++file_index){
        std::filesystem::remove(output_filename + "_" + std::to_string(file_index));
        std::filesystem::remove(output_filename + "sorted_" + std::to_string(file_index));
    }
    outputFile.close();

//    if(outputFile.is_open()){
//        for(int rowIndex = 0; rowIndex < size_to_fit; ++rowIndex){
//            // Tamaño de pedazo
//            int lengthPart = size_to_fit;
//            // Empieza en 0 de lo contrario en i*tamaño de pedazo + 1
//            int begin = (rowIndex == 0) ? 0 : (rowIndex * lengthPart);
//            // Donde termina??
//            int end = (rowIndex == size_to_fit - 1) ? flat_matrix.size() - 1 : (rowIndex * lengthPart) + lengthPart;
//
//            typename std::vector<T>::iterator beginIt = flat_matrix.begin() + begin;
//            typename std::vector<T>::iterator endIt = flat_matrix.begin() + end;
//
//            std::copy(beginIt, endIt, std::ostream_iterator<T>(outputFile, " "));
//            outputFile << '\n';
//        }
//    }
//
//    outputFile.close();
}

template<typename T>
void printVector(std::string rem, std::vector<T> const& v)
{
    std::cout << rem;
    std::copy(v.begin(), v.end(), std::ostream_iterator<T>(std::cout, " "));
    std::cout << '\n';
};

#endif //MATRIX_MERGE_SORT_MEMORY_UTILS_H
