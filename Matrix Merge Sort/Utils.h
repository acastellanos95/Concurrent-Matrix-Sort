//
// Created by andre on 10/15/21.
//

#ifndef MATRIX_MERGE_SORT_UTILS_H
#define MATRIX_MERGE_SORT_UTILS_H

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

std::ifstream& goToLine(std::ifstream& file, unsigned int num){
    file.seekg(std::ios::beg);
    for(int i=0; i < num - 1; ++i){
        file.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
    }
    return file;
}

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

template<typename T>
void mergeSort(typename std::vector<T>::iterator begin, typename std::vector<T>::iterator end)
{
    // Caso Base: si queda solo un elemento base.
    if (std::distance(begin, end) == 1)
        return ;
    // Computamos el punto medio con aritmetica de iteradores (apuntadores).
    long midSize = std::distance(begin, end)/2;
    typename std::vector<T>::iterator mid = begin;
    std::advance(mid, midSize);
    // Vectores copiados auxiliares para poder usar std::merge (son desechados cuando salen del scope)
    std::vector<T> low(begin, mid);
    std::vector<T> high(mid, end);
    // Recursivamente llamamos merge-sort para las dos mitades del vector
    mergeSort<T>(low.begin(), low.end());
    mergeSort<T>(high.begin(), high.end());
    // Combinamos la solución en el vector original de entrada
    std::merge(low.begin(), low.end(), high.begin(), high.end(), begin);
}

template<typename T>
void mergeKArrays(std::vector<std::vector<T>> &v,long i,long j, std::vector<T> &output)
{
    //if one array is in range
    if(i==j)
    {
        for(size_t p=0; p < v[0].size(); p++)
            output.push_back(v[i][p]);
        return;
    }

    //if only two arrays are left them merge them
    if(j-i==1)
    {
//        mergeArrays(arr[i],arr[j],n,n,output);
        std::merge(v[i].begin(), v[i].end(), v[j].begin(), v[j].end(), std::back_inserter(output));
        return;
    }

    //output arrays
    std::vector<T> out1,out2;

    //divide the array into halves
    mergeKArrays(v,i,(i+j)/2,out1);
    mergeKArrays(v,(i+j)/2+1,j,out2);

    //merge the output array
//    mergeArrays(out1,out2,*(((i+j)/2)-i+1),n*(j-((i+j)/2)),output);
    std::merge(out1.begin(), out1.end(), out2.begin(), out2.end(), std::back_inserter(output));

}

/*K-way Merge of square matrix*/
template<typename T>
void kWaysMerge(std::vector<std::vector<T>> &v)
{
//    Lamentablemente aunque afecte la complejidad del espacio por tiempo para realizar el proyecto usaremos merge en pares
    /*Creamos el vector solución*/
    std::vector<T> solution;

    mergeKArrays(v, 0, v[0].size() - 1, solution);

    /*Copiar vector solución a la matriz cuadrada O(n)*/
    size_t matrixRow = 0;
    for (size_t solutionIndex = 0; solutionIndex < solution.size(); ++solutionIndex) {
        if(solutionIndex%v[0].size() == 0 && solutionIndex != 0){
            ++matrixRow;
        }
        v[matrixRow][solutionIndex%v.size()] = solution[solutionIndex];
    }
};

/* Plantilla de lector de archivo que ordena las filas con mergesort y lo coloca en el nombre de archivo de salida
 * regresa de salida una tupla con el tamaño de filas y columnas
 * */
template<typename T>
std::tuple<long> readFileAndMergeSortRows(const std::string &input_filename, const std::string &output_filename, const int &num_threads){
    // Último hilo
    int lastThreadIndex = -1;
    // Tamaño de la matriz
    std::atomic<long> matrixSize = 0;
    // Leer el archivo
    std::ifstream inputFile(input_filename, std::ifstream::in);
    std::ofstream outputFile(output_filename, std::ofstream::out | std::ofstream::trunc);
    std::string line;
    if(inputFile.is_open()){
        while (lastThreadIndex == -1){
            std::vector<std::thread> threads(num_threads);
            std::mutex mtx;
            // Crear hilos
            for (int i = 0; i < num_threads; ++i) {
                // Si acabamos levantamos la bandera de la última linea y guardamos el índice de hilo para que al unirlos solo unamos los necesarios
                if(!std::getline(inputFile, line)){

                    lastThreadIndex = i;
                    break;
                }

                // inicializamos los hilos con una función anónima que le pasamos el tamaño de la matriz para actualizar
                threads[i] = std::thread([&matrixSize, i, line, &outputFile, num_threads, &mtx](){
                    // Vector auxiliar
                    std::vector<T> auxRow;
                    // Leemos y ordenamos la fila
                    std::istringstream lineStream(line);
                    std::string cell;
                    while(lineStream >> cell)
                    {
                        char *ptr;
                        if(std::is_same<T, int>::value || std::is_same<T, long>::value || std::is_same<T, long long>::value){
                            auxRow.push_back((T) strtol(cell.c_str(), &ptr, 10));
                        }else if (std::is_same<T, double>::value || std::is_same<T, float>::value || std::is_same<T, long double>::value) {
                            auxRow.push_back((T) strtod(cell.c_str(), &ptr));
                        } else throw std::runtime_error("El tipo T no es soportado");
                    }
                    mergeSort<T>(auxRow.begin(), auxRow.end());

                    // sincronización ordenada para poner la información en el archivo por orden de creación del hilo
                    bool flag = true;
                    while (flag){
                        mtx.lock();
                        // Sección crítica!!
                        // checamos si está en el orden que le toca, si es 0 trivialmente siempre estará en orden, pero si no está en orden entonces liberamos la llave
                        if(matrixSize%num_threads == i){
                            // Si estamos en orden de ejecución entonces podemos escribir al archivo y pasar la bandera a falso
                            if(outputFile.is_open()){
                                std::copy(auxRow.begin(), auxRow.end(), std::ostream_iterator<T>(outputFile, " "));
                                outputFile << '\n';
                            }
                            flag = false;
                        }
                        // Aumentamos el contador del tamaño de la matriz para el siguiente hilo
                        matrixSize++;
                        mtx.unlock();
                    }
                });
            }
            // Join hilos hasta num_threads o lastThreadIndex si no es -1
            for (int i = 0; i < (lastThreadIndex == -1?num_threads:lastThreadIndex); ++i) {
                threads[i].join();
            }
        }
    }
    inputFile.close();
    outputFile.close();
    return std::make_tuple(matrixSize.load());
}

template<typename T>
std::tuple<long> readFileAndMergeSortRowsDiff(const std::string &input_filename, const std::string &output_filename, const int &num_threads){
    // Tamaño de la matriz
    long matrixSize = 0;
    std::vector<T> auxRow;
    // Leer el archivo
    std::ifstream inputFile(input_filename, std::ifstream::in);
    std::ofstream outputFile(output_filename, std::ofstream::out | std::ofstream::trunc);
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
                    // Ordenamos el pedazo de arreglo de la fila en la matriz auxiliar
                    merge_sort<typename std::vector<T>::iterator>(auxIRow[i].begin(), auxIRow[i].end());
                });
            }
            // Join hilos hasta num_threads
            for (int i = 0; i < num_threads; ++i) {
                threads[i].join();
            }
            if(outputFile.is_open()){
                auxRow.clear();
                // Mezclamos la matriz auxiliar
                mergeKArrays(auxIRow, 0, auxIRow.size() - 1, auxRow);
                // Guardamos en el archivo
                std::copy(auxRow.begin(), auxRow.end(), std::ostream_iterator<T>(outputFile, " "));
                outputFile << '\n';
            }
        }
    }
    inputFile.close();
    outputFile.close();
    return std::make_tuple(matrixSize);
}

template<typename T>
void readFileAndMergeRows(const std::string &input_filename, const std::string &output_filename, long matrix_size,const int num_threads_file_iterator){
    // Inicializamos vectores de archivo de entrada para saltar a ciertas lineas y acomodar concurrentemente y el archivo de salida
    std::ofstream outputFile(output_filename, std::ofstream::out | std::ofstream::trunc);

    // Hilos
    std::vector<std::thread> threads(num_threads_file_iterator);
    //Iniciamos hilos para hacer k ways merge sobre la lineas de begin a end
    for (int file_index = 0; file_index < num_threads_file_iterator; ++file_index) {
        long lengthFilePart = matrix_size/num_threads_file_iterator;
        long begin;
        if(file_index == 0){
            // Las lineas comienzan en 1
            begin = 1;
        }else{
            // El que sigue del final
            begin = (file_index*lengthFilePart) + 1;
        }
        long end;
        if(file_index == num_threads_file_iterator - 1){
            end = matrix_size;
        } else{
            end = (file_index*lengthFilePart) + lengthFilePart;
        }
        // Crear hilos
        // inicializamos los hilos con una función anónima que le pasamos el tamaño de la matriz para actualizar
        threads[file_index] = std::thread([begin, file_index, input_filename, output_filename, end](){
//            std::cout << "begin: " << begin << " end: " << end << std::endl;
            std::ifstream inputFile(input_filename, std::ifstream::in);
            std::ofstream outputFiles(output_filename + "_" + std::to_string(file_index), std::ofstream::out | std::ofstream::trunc);
            goToLine(inputFile, begin);
            // Linea
            std::string line;
            // Matriz y vector auxiliar
            std::vector<std::vector<T>> auxMatrix;
            std::vector<T> auxRow;
            // Leemos de la linea en inputFile[file_index] hasta end para guardar en la matriz auxiliar
            for (long lineIndex = begin; lineIndex <= end; ++lineIndex) {
                std::getline(inputFile, line);
                std::istringstream lineStream(line);
                std::string cell;
                while(lineStream >> cell)
                {
                    char *ptr;
                    if(std::is_same<T, int>::value || std::is_same<T, long>::value || std::is_same<T, long long>::value){
                        auxRow.push_back((T) strtol(cell.c_str(), &ptr, 10));
                    }else if (std::is_same<T, double>::value || std::is_same<T, float>::value || std::is_same<T, long double>::value) {
                        auxRow.push_back((T) strtod(cell.c_str(), &ptr));
                    } else throw std::runtime_error("El tipo T no es soportado");
                }
                auxMatrix.push_back(auxRow);
                auxRow.clear();
            }
            // Mezclamos la matriz auxiliar
            mergeKArrays<T>(auxMatrix, 0, auxMatrix.size() - 1, auxRow);
            if(outputFiles.is_open()){
                // Guardamos en los num_hilos archivos
                std::copy(auxRow.begin(), auxRow.end(), std::ostream_iterator<T>(outputFiles, " "));
                outputFiles << '\n';
            }
            outputFiles.close();
            inputFile.close();
        });
    }
    // Join hilos de archivo
    for (int file_index = 0; file_index < num_threads_file_iterator; ++file_index) {
        threads[file_index].join();
    }

    // vector auxiliar
    std::vector<T> auxRow;
    std::vector<std::vector<T>> auxMatrix;
    // Combinar archivos
    for (int file_index = 0; file_index < num_threads_file_iterator; ++file_index) {
        std::ifstream outputFileRow(output_filename + "_" + std::to_string(file_index), std::ofstream::in);
        // Linea
        std::string line;

        std::getline(outputFileRow, line);
        std::istringstream lineStream(line);
        std::string cell;
        while(lineStream >> cell)
        {
            char *ptr;
            if(std::is_same<T, int>::value || std::is_same<T, long>::value || std::is_same<T, long long>::value){
                auxRow.push_back((T) strtol(cell.c_str(), &ptr, 10));
            }else if (std::is_same<T, double>::value || std::is_same<T, float>::value || std::is_same<T, long double>::value) {
                auxRow.push_back((T) strtod(cell.c_str(), &ptr));
            } else throw std::runtime_error("El tipo T no es soportado");
        }
        auxMatrix.push_back(auxRow);
        auxRow.clear();
        outputFileRow.close();
    }
    // Mezclamos la matriz de los num_hilos archivos a un vector
    mergeKArrays<T>(auxMatrix, 0, auxMatrix.size() - 1, auxRow);

    // Hilos para escribir a archivos y después solo hacer append
    std::vector<std::thread> fileThreads(num_threads_file_iterator);
    //Iniciamos hilos para escribir en num_threads_file_iterator archivos
    for (int file_index = 0; file_index < num_threads_file_iterator; ++file_index) {
        long lengthFilePart = (matrix_size*matrix_size)/num_threads_file_iterator;
        long begin = file_index == 0 ? 0 : (file_index*lengthFilePart + 1);
        long end = file_index == (num_threads_file_iterator - 1) ? matrix_size*matrix_size - 1 : (file_index*lengthFilePart) + lengthFilePart;
        // Crear hilos
        // inicializamos los hilos con una función anónima que le pasamos el tamaño de la matriz para actualizar
        fileThreads[file_index] = std::thread([begin, end, &auxRow, matrix_size, file_index, output_filename](){
            std::ofstream outputFileSorted(output_filename + "sorted_" + std::to_string(file_index), std::ofstream::out | std::ofstream::trunc);
            for (auto col = begin; col <= end; ++col) {
                if(col != 0 && col%matrix_size == 0){
                    outputFileSorted << '\n';
                }
                if(col >= matrix_size*matrix_size) break;
                outputFileSorted << " " << auxRow[col] << " ";
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
    // Borrar el archivo temporal de entrada
    std::filesystem::remove(input_filename);
}

template<typename T>
void printVector(std::string rem, std::vector<T> const& v)
{
    std::cout << rem;
    std::copy(v.begin(), v.end(), std::ostream_iterator<T>(std::cout, " "));
    std::cout << '\n';
};

#endif //MATRIX_MERGE_SORT_UTILS_H
