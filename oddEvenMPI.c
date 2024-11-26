#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int merge(int *ina, int lena, int *inb, int lenb, int *out) {
    int i, j, outcount = 0;

    for (i = 0, j = 0; i < lena; i++) {
        while ((inb[j] < ina[i]) && j < lenb) {
            out[outcount++] = inb[j++];
        }
        out[outcount++] = ina[i];
    }
    while (j < lenb) {
        out[outcount++] = inb[j++];
    }

    return 0;
}

int domerge_sort(int *a, int start, int end, int *b) {
    if ((end - start) <= 1) return 0;

    int mid = (end + start) / 2;
    domerge_sort(a, start, mid, b);
    domerge_sort(a, mid, end, b);
    merge(&(a[start]), mid - start, &(a[mid]), end - mid, &(b[start]));
    for (int i = start; i < end; i++) {
        a[i] = b[i];
    }

    return 0;
}

int merge_sort(int n, int *a) {
    int b[n];
    domerge_sort(a, 0, n, b);
    return 0;
}

int is_sorted(int n, int *a) {
    for (int i = 1; i < n; i++) {
        if (a[i] < a[i - 1]) {
            return 0; // Not sorted
        }
    }
    return 1; // Sorted
}

void MPI_Pairwise_Exchange(int localn, int *locala, int sendrank, int recvrank, MPI_Comm comm) {
    int rank;
    int remote[localn];
    int all[2 * localn];
    const int mergetag = 1;
    const int sortedtag = 2;

    MPI_Comm_rank(comm, &rank);
    if (rank == sendrank) {
        MPI_Send(locala, localn, MPI_INT, recvrank, mergetag, MPI_COMM_WORLD);
        MPI_Recv(locala, localn, MPI_INT, recvrank, sortedtag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    } else {
        MPI_Recv(remote, localn, MPI_INT, sendrank, mergetag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        merge(locala, localn, remote, localn, all);

        int theirstart = 0, mystart = localn;
        if (sendrank > rank) {
            theirstart = localn;
            mystart = 0;
        }
        MPI_Send(&(all[theirstart]), localn, MPI_INT, sendrank, sortedtag, MPI_COMM_WORLD);
        for (int i = mystart; i < mystart + localn; i++) {
            locala[i - mystart] = all[i];
        }
    }
}

int MPI_OddEven_Sort(int n, int *a, int root, MPI_Comm comm) {
    int rank, size, i;
    int *local_a;
    int sorted_result = 0;

    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);

    local_a = (int *)calloc(n / size, sizeof(int));

    // Scatter the array a to local_a
    MPI_Scatter(a, n / size, MPI_INT, local_a, n / size, MPI_INT, root, comm);

    // Sort local_a
    merge_sort(n / size, local_a);

    // Odd-even part
    for (i = 0; i < size; i++) {
        if ((i + rank) % 2 == 0) {
            if (rank < size - 1) {
                MPI_Pairwise_Exchange(n / size, local_a, rank, rank + 1, comm);
            }
        } else if (rank > 0) {
            MPI_Pairwise_Exchange(n / size, local_a, rank - 1, rank, comm);
        }

        MPI_Barrier(comm);

        // Test if the array is sorted
        sorted_result = is_sorted(n / size, local_a);
        if (sorted_result == 1) {
            break;
        }
    }

    // Gather local_a to a
    MPI_Gather(local_a, n / size, MPI_INT, a, n / size, MPI_INT, root, comm);

    // Print result
    /*if (rank == root) {
        printf("Sorted Array: \n");
        for (int j = 0; j < n; j++) {
            printf("%d ", a[j]);
        }
        printf("\n");
    }*/

    free(local_a);
    return MPI_SUCCESS;
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int n = 100000; // Hardcoded array size of 100
    int a[n];

    // Initialize the array with random values
    if (argc == 1) {
        for (int i = 0; i < n; i++) {
            a[i] = rand() % 1000; // Random integers between 0 and 999
        }
    } else {
        // If arguments are provided, use them as the input
        for (int i = 0; i < n; i++) {
            a[i] = atoi(argv[i + 1]);
        }
    }

    double start_time, end_time;

    MPI_Barrier(MPI_COMM_WORLD); // Synchronize all processes before starting the timer

    start_time = MPI_Wtime(); // Start timing

    MPI_OddEven_Sort(n, a, 0, MPI_COMM_WORLD);

    end_time = MPI_Wtime(); // End timing

    if (argc == 1) {
        // Only print execution time if no arguments are passed (i.e., random array)
        printf("Execution Time: %f seconds\n", end_time - start_time);
    }

    MPI_Finalize();

    return 0;
}

