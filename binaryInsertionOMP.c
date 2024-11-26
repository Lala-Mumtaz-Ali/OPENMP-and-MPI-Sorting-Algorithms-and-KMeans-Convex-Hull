#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

#define SIZE 1000000

int n1[SIZE];
int n2[SIZE];

typedef struct dim {
    int start;
    int end;
} limit;

// Binary search function for binary insertion sort
int binary_search(int move, int start, int end) {
    while (start <= end) {
        int mid = (start + end) / 2;
        if (move == n1[mid])
            return mid + 1;
        else if (move > n1[mid])
            start = mid + 1;
        else
            end = mid - 1;
    }
    return start;
}

// Binary insertion sort for a specific range
void insertion(limit l) {
    for (int i = l.start + 1; i <= l.end; i++) {
        int move = n1[i];
        int pos = binary_search(move, l.start, i - 1);

        // Shift elements to make space for 'move'
        for (int j = i; j > pos; j--) {
            n1[j] = n1[j - 1];
        }
        n1[pos] = move;
    }
}

// Merge two sorted sections
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

// Fill the array with random numbers
void fill_array(int size) {
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        n1[i] = rand() % 1000;
    }
}

// Print the array
void print_array(int *list, int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", list[i]);
    }
    printf("\n");
}

int main() {
    fill_array(SIZE);
    printf("Original Array: ");
    print_array(n1, SIZE);

    int num_threads;
    printf("Enter number of threads: ");
    scanf("%d", &num_threads);

    if (num_threads > SIZE) {
        printf("Number of threads cannot exceed the array size.\n");
        return -1;
    }

    omp_set_num_threads(num_threads);
    int chunk_size = SIZE / num_threads;
    limit limits[num_threads];

    for (int i = 0; i < num_threads; i++) {
        limits[i].start = i * chunk_size;
        if (i == num_threads - 1)
            limits[i].end = SIZE - 1; // Handle the remainder
        else
            limits[i].end = (i + 1) * chunk_size - 1;
    }

    double start_time = omp_get_wtime();

    // Parallel binary insertion sort
#pragma omp parallel for
    for (int i = 0; i < num_threads; i++) {
        insertion(limits[i]);
    }
 // Merging sorted sections
    int step = 1;
    while (step < num_threads) {
#pragma omp parallel for
        for (int i = 0; i < num_threads; i += 2 * step) {
            if (i + step < num_threads) {
                merge_sections(
                    limits[i].start, limits[i].end,
                    limits[i + step].start, limits[i + step].end
                );
                limits[i].end = limits[i + step].end;
            }
        }
        step *= 2;
    }

    double end_time = omp_get_wtime();

    printf("\nSorted Array: ");
    print_array(n1, SIZE);
    printf("Time Taken: %lf seconds\n", end_time - start_time);

    return 0;
}
