#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int num_thread;

void odd_even_sort(int *arr, int n) {
    int sorted = 0;

    while (!sorted) {
        sorted = 1;

        // Single parallel region, threads are created once
        #pragma omp parallel shared(arr, sorted)
        {
            // First phase: Odd indexed elements
            #pragma omp for
            for (int i = 1; i < n - 1; i += 2) {
                if (arr[i] > arr[i + 1]) {
                    int temp = arr[i];
                    arr[i] = arr[i + 1];
                    arr[i + 1] = temp;
                   
                    sorted = 0;  // Set sorted flag to 0
                }
            }

            // Second phase: Even indexed elements
            #pragma omp for
            for (int i = 0; i < n - 1; i += 2) {
                if (arr[i] > arr[i + 1]) {
                    int temp = arr[i];
                    arr[i] = arr[i + 1];
                    arr[i + 1] = temp;
                                       sorted = 0;  // Set sorted flag to 0
                }
            }
        }

        // Barrier ensures synchronization before starting the next iteration
        #pragma omp barrier
    }
}

int main() {
    int n;

    // Input array size and number of threads
    printf("Enter the size of the array: ");
    scanf("%d", &n);

    printf("Enter the number of threads: ");
    scanf("%d", &num_thread);

    // Set the number of threads globally for all parallel regions
    omp_set_num_threads(num_thread);

    // Allocate memory for the array
    int *arr = (int *)malloc(n * sizeof(int));

    // Initialize array with random values
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % 100;
    }

    // Measure the time taken for sorting
    double start_time = omp_get_wtime();

    odd_even_sort(arr, n);

    double end_time = omp_get_wtime();
    double time_taken = end_time - start_time;
    
    // Uncomment to display the sorted array
     //for (int i = 0; i < n; i++) {
     //    printf("%d ", arr[i]);
     //}
     //printf("\n");

    // Output the time taken
    printf("\nTime taken to sort the array: %f seconds\n", time_taken);

    // Free the allocated memory
    free(arr);

    return 0;
}
