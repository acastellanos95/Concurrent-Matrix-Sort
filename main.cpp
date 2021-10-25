#include <iostream>
#include <vector>
#include <fstream>
#include <random>
#include <functional>
#include <iterator>
#include <algorithm>

int main(int argc, char const *argv[])
{
  int N;
  char *endptr;
  if (argc == 2)
  {
    N = strtol(argv[1], &endptr, 10);
  }
  else
  {
    std::cout << "Indique el número de hilos a ejecutar" << std::endl;
    exit(EXIT_FAILURE);
  }

  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_real_distribution<> dis(-9, 9);

  // Crear matriz de flotantes (no es necesario más precisión)
  std::vector<std::vector<float>> matrix(N, std::vector<float>(N));

  for (size_t i = 0; i < matrix.size(); ++i)
  {
    std::generate(matrix[i].begin(), matrix[i].end(), std::bind(dis, std::ref(mt)));
  }

  std::ofstream outputFile("matriz_" + std::to_string(N) + ".txt", std::ofstream::out);

  for (int i = 0; i < matrix.size(); ++i)
  {
    std::copy(matrix[i].begin(), matrix[i].end(), std::ostream_iterator<float>(outputFile, " "));
    outputFile << '\n';
  }

  outputFile.close();

  return 0;
}
