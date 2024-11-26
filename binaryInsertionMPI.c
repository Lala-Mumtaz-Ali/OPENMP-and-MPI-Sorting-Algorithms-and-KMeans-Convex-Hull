#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

#define SIZE 1000000

int n1[SIZE];
int n2[SIZE]; 

// Binary search function that works on the global n1 array
int binary_search(int move, int start, int end) {
    if (end <= start) {
        return (move > n1[end]) ? (end + 1) : end;
    }

    int mid = (start + end) / 2;
    if (move == n1[mid]) return mid + 1;
    if (move > n1[mid]) return binary_search(move, mid + 1, end);
    return binary_search(move, start, mid - 1);
}

// Binary insertion sort on a portion of the global array
void binary_insertion_sort(int start, int end) {
    int i = start + 1;
    int j, k, move;
    while (i <= end) {
        j = i - 1;
        move = n1[i];
        k = binary_search(move, start, j);
        while (j >= k) {
            n1[j + 1] = n1[j];
            j--;
        }
        n1[j + 1] = move;
        i++;
    }
}

// Merge two sorted sections of the global array
void merge_sections(int start1, int end1, int start2, int end2) {
    int i = start1, j = start2, tpos = start1;

    while (i <= end1 && j <= end2) {
        if (n1[i] < n1[j])
            n2[tpos++] = n1[i++];
        else
            n2[tpos++] = n1[j++];
    }

    while (i <= end1) n2[tpos++] = n1[i++];
    while (j <= end2) n2[tpos++] = n1[j++];

    for (int k = start1; k <= end2; k++) {
        n1[k] = n2[k];
    }
}

// Fill the global array n1 with random values
void fill_array(int size) {
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        n1[i] = rand() % 1000;
    }
}

// Print the global array n1
void print_array(int *list, int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", list[i]);
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    int rank, num_processes;
    double start_time, end_time;

    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

    // Start the timer
    start_time = MPI_Wtime();

    if (num_processes > SIZE) {
        if (rank == 0) {
            printf("Number of processes cannot exceed the array size.\n");
        }
        MPI_Finalize();
        return -1;
    }

    int chunk_size = SIZE / num_processes;
    int start = rank * chunk_size;
    int end = (rank == num_processes - 1) ? SIZE - 1 : (rank + 1) * chunk_size - 1;

    if (rank == 0) {
        fill_array(SIZE);
        //printf("Original Array: ");
        //print_array(n1, SIZE);
    }

    // Perform binary insertion sort on the global array n1 (using start and end for each process)
    binary_insertion_sort(start, end);

    // Wait for all processes to finish before proceeding
    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0) {
        int step = 1;
        while (step < num_processes) {
            #pragma omp parallel for
            for (int i = 0; i < num_processes; i += 2 * step) {
                if (i + step < num_processes) {
                    int start1 = i * chunk_size;
                    int end1 = (i + step) * chunk_size - 1;
                    int start2 = (i + step) * chunk_size;
                    int end2 = (i + 2 * step) * chunk_size - 1;
                    if (end2 >= SIZE) end2 = SIZE - 1;  // Handle last merge range
                    merge_sections(start1, end1, start2, end2);
                }
            }
            step *= 2;
        }
        //printf("\nSorted Array: ");
        //print_array(n1, SIZE);
    }

    // End the timer
    end_time = MPI_Wtime();

    // Print the execution time
    if (rank == 0) {
        printf("Execution Time: %f seconds\n", end_time - start_time);
    }

    // Finalize MPI
    MPI_Finalize();
    return 0;
}

