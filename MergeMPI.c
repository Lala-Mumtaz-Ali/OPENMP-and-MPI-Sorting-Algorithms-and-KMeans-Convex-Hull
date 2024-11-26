#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

void merge(int *arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    int *L = (int *)malloc(n1 * sizeof(int));
    int *R = (int *)malloc(n2 * sizeof(int));

    for (int i = 0; i < n1; i++)
        L[i] = arr[left + i];

    for (int j = 0; j < n2; j++)
        R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;

    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }

    free(L);
    free(R);
}

void mergeSort(int *arr, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(arr, left, mid);   
        mergeSort(arr, mid + 1, right); 
        merge(arr, left, mid, right);  
    }
}

void mpiMergeSort(int *arr, int size, int rank, int num_procs) {
    int local_size = size / num_procs;

    int *local_array = (int *)malloc(local_size * sizeof(int));

    MPI_Scatter(arr, local_size, MPI_INT, local_array, local_size, MPI_INT, 0, MPI_COMM_WORLD);

    mergeSort(local_array, 0, local_size - 1);

    MPI_Gather(local_array, local_size, MPI_INT, arr, local_size, MPI_INT, 0, MPI_COMM_WORLD);

    free(local_array);
}

void mergeAll(int *arr, int size, int num_procs) {
    int step = size / num_procs;
    while (step < size) {
        for (int i = 0; i < num_procs; i++) {
            int left = i * step * 2;
            int mid = left + step - 1;
            int right = (i + 1) * step * 2 - 1;
            if (right >= size)
                right = size - 1;
            if (mid < size)
                merge(arr, left, mid, right);
        }
        step *= 2;
    }
}

int main(int argc, char *argv[]) {
    int rank, num_procs;
    int *array;
    int size = 1000;    
    srand(time(NULL));

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    double start_time, end_time;

    if (rank == 0) {
        array = (int *)malloc(size * sizeof(int));
        for (int i = 0; i < size; i++) {
            array[i] = rand() % 1000000;  
        }
    }

    MPI_Barrier(MPI_COMM_WORLD); // Synchronize before timing
    start_time = MPI_Wtime();

    mpiMergeSort(array, size, rank, num_procs);

    if (rank == 0) {
        mergeAll(array, size, num_procs);
    }

    MPI_Barrier(MPI_COMM_WORLD); // Synchronize after timing
    end_time = MPI_Wtime();

    if (rank == 0) {
        //printf("Sorted array:\n");
       // for (int i = 0; i < size; i++) {
        //    printf("%d ", array[i]);
       // }
        printf("\n");
        printf("Execution Time: %f seconds\n", end_time - start_time);
        free(array);
    }

    MPI_Finalize();
    return 0;
}

