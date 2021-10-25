cp -r ../datos/. cmake-build-debug/.
{
    read
    while IFS=" " read -r MATRIX_SIZE THREADS
    do
        echo "Matrix size: $MATRIX_SIZE, Threads: $THREADS"
        sed -i "s/matriz_matrix_size.txt/matriz_$MATRIX_SIZE.txt/" "main.cpp"
        sed -i "s/matriz_matrix_size-sort.txt/matriz_$MATRIX_SIZE-sort.txt/" "main.cpp"
        cd cmake-build-debug
        make
        (env time --format="%e" ./Matrix_Merge_Sort "$THREADS" ;) &>> test.out
        cd ..
        sed -i "s/matriz_$MATRIX_SIZE.txt/matriz_matrix_size.txt/" "main.cpp"
        sed -i "s/matriz_$MATRIX_SIZE-sort.txt/matriz_matrix_size-sort.txt/" "main.cpp"
    done
} < sizes.txt