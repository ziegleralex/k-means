#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "distance.h"
#include "ioutils.h"
#include <math.h>

void initialize_cluster_assignment(int n, int y[n], int value) {
    for (int i = 0; i < n; i++) {
        y[i] = value;
    }
}

void copy_cluster_assignment(int n, const int from_y[n], int to_y[n]) {
    for (int i = 0; i < n; i++) {
        to_y[i] = from_y[i];
    }
}

bool is_identical_cluster_assignment(int n, const int a[n], const int b[n]) {
    for (int i = 0; i < n; i++) {
        if (a[i] != b[i])
            return false;
    }
    return true;
}

/* return a truly uniform random value in the range 0...n-1 inclusive */
int rand_in_range(int n) {
    int limit;
    int r;
    limit = RAND_MAX - (RAND_MAX % n);
    while((r = rand()) >= limit);
    return r % n;
}

void randomize_centroids(int n, int d, double x[n][d], int k, double centroids[k][d]) {

    // randomly place centroids somewhere near the data points

    for (int i = 0; i < k; i++) {
        for (int j = 0; j < d; j++) {
            centroids[i][j] = x[rand_in_range(n)][j];
        }
    }

}

void assign_datapoints(int n, int d, double x[n][d], int y[n], int k, double centroids[k][d], distance dist) {

    for (int i = 0; i < n; i++) {

        // uninitialised

        double min_distance = INFINITY;
        int closest_centroid = -1;

        for (int j = 0; j < k; j++) {

            // calc

            double current_distance = compute_distance(d, x[i], centroids[j], dist);

            if (current_distance < min_distance) {
                min_distance = current_distance;
                closest_centroid = j;
            }

        }

        if (y[i] != closest_centroid) {
            y[i] = closest_centroid;
        }

    }

}

void update_centroids(int n, int d, double x[n][d], int y[n], int k, double centroids[k][d]) {
    double sums[k][d]; // accumulate the coordinates of data points within each cluster
    int counts[k]; // keep track of how many data points are in each cluster

    // sums: sum of the coordinates of the data points
    // counts: how many data points are in each cluster, e.g. cluster 0 = 30

    // initialise both variables (set them to 0 for every value)
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < d; j++) {
            sums[i][j] = 0.0;
        }
        counts[i] = 0;
    }

    // go through every data point and sum them and increment count everytime
    for (int i = 0; i < n; i++) {
        int cluster = y[i];
        for (int j = 0; j < d; j++) {
            sums[cluster][j] += x[i][j];
        }
        counts[cluster]++;
    }

    // calc the mean and move the centroids
    for (int i = 0; i < k; i++) {
        if (counts[i] > 0) {
            for (int j = 0; j < d; j++) {
                centroids[i][j] = sums[i][j] / counts[i];
            }
        }
    }
}

void cluster(int n, int d, double x[n][d], int y[n], int k, double centroids[k][d], distance dist) {

    assert(n > 0 && "hey, I need at least one instance in the training data!"); // credits to mr baumann

    // 1: define amount of cluster centroids (in this case k)

    // 2: randomly place k centroids
    randomize_centroids(n, d, x, k, centroids);

    int iterations = 0;

    bool converged = false;
    int previous[n];

    for (int i = 0; i < n; ++i) {
        printf("%d\n", previous[i]);
    }

    do {

        // 3: calculate the distance between each point to the centroids and add the nearest to its cluster
        assign_datapoints(n, d, x, y, k, centroids, dist);

        // 4: calculate the mean of each cluster centroid and move it
        update_centroids(n, d, x, y, k, centroids);

        // 5: check if the previous cluster assignment matches the newer assignment and if nothing changed then it converged
        converged = is_identical_cluster_assignment(n, y, previous);

        // 6: copy the new cluster assignment to the old one
        copy_cluster_assignment(n, y, previous);

        iterations++;

    } while (!converged && iterations < 100);

}

bool tests() {
    // either have very many assert statements in here, or call more functions which test via assert statements
    return true;
}

void check_usage(int argc, char** argv) {
    if (argc != 6) {
        printf("usage: %s <n> <d> <filename.csv> <k> <distance-metric>\n", argv[0]);
        printf("<n> number of data points to be read\n");
        printf("<d> dimensionality of data points\n");
        printf("<filename.csv> contains the data to be classified (1st line will be ignored)\n");
        printf("<k> number of neighbours for majority voting\n");
        printf("<distance-metric>: must be \"manhattan\" or \"euclidean\"\n");
        exit(1);
    }
}

int main(int argc, char** argv) {
    assert(tests() && printf("tests have passed\n"));
    srand(1);
    check_usage(argc, argv);
    int n = atoi(argv[1]);
    int d = atoi(argv[2]);
    char* filename = argv[3];
    int k = atoi(argv[4]);
    distance dist = strcmp(argv[5], "manhattan") == 0 ? MANHATTAN : EUCLIDEAN;

    double x[n][d];
    int y[n];
    n = read_csv(filename, n, d, x, y);
    // printf("Original data:\n");
    // print_csv(n, d, x, y);
    initialize_cluster_assignment(n, y, -1); // overwrite clusterIDs so that cluster() can't use it.

    double centroids[k][d];

    cluster(n, d, x, y, k, centroids, dist);

    printf("Clustering result:\n");
    for (int i = 0; i < k; i++) {
        printf("Centroid for cluster %d: ", i);
        print_datapoint(d, centroids[i]);
        printf("\n");
    }
    print_csv(n, d, x, (int*) y);
}
