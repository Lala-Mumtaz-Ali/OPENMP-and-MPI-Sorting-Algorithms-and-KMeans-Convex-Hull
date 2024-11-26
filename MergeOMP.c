#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// Global variable for array size
int SIZE;

// Function to merge two sorted subarrays into a single sorted array
void merge(int *arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    // Create temporary arrays
    int *L = (int *)malloc(n1 * sizeof(int));
    int *R = (int *)malloc(n2 * sizeof(int));

    #pragma omp parallel for
    for (int i = 0; i < n1; i++)
        L[i] = arr[left + i];

    #pragma omp parallel for
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

// Function to perform merge sort
void mergeSort(int *arr, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;

        #pragma omp parallel
        {
            #pragma omp single // Single thread spawns tasks
            {
                #pragma omp task // Left half
                mergeSort(arr, left, mid);

                #pragma omp task // Right half
                mergeSort(arr, mid + 1, right);
            }
        }

        merge(arr, left, mid, right);
    }
}

int main() {
    int num_threads;

    // Input the size of the array and the number of threads
    printf("Enter the size of the array: ");
    scanf("%d", &SIZE);

    printf("Enter the number of threads: ");
    scanf("%d", &num_threads);

    // Set the number of threads
    omp_set_num_threads(num_threads);

    // Allocate memory for the array
    int *arr = (int *)malloc(SIZE * sizeof(int));

    // Initialize the array with random numbers
    #pragma omp parallel for
    for (int i = 0; i < SIZE; i++) {
        arr[i] = rand() % 100; // Random number between 0 and 99
    }

    // Print the unsorted array (optional)
    //printf("Unsorted array:\n");
    //for (int i = 0; i < SIZE; i++) {
    //    printf("%d ", arr[i]);
    //}
    //printf("\n");

    // Measure the time for merge sort
    double start_time = omp_get_wtime();

    #pragma omp parallel
    {
        #pragma omp single
        mergeSort(arr, 0, SIZE - 1);
    }

    double end_time = omp_get_wtime();

    // Print the sorted array
    //printf("Sorted array:\n");
    //for (int i = 0; i < SIZE; i++) {
     //   printf("%d ", arr[i]);
    //}
    //printf("\n");

    // Print the execution time
    printf("Time taken for merge sort: %.6f seconds\n", end_time - start_time);

    // Free allocated memory
    free(arr);

    return 0;
}

