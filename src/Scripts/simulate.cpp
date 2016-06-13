#include <iostream>
#include <random>

#define MAX_NUM_ROWS 10
#define NUM_ITERS 1000
#define BLOCK_SIZE 512 // cache line size in bits.
#define NUM_BLOCKS 10000
#define NUM_DOCS BLOCK_SIZE * NUM_BLOCKS

// DESIGN NOTE: tried using go, but the publicly available binomial rng is approximately 10x slower.
// TODO: change conventions to match BitFunnel coding conventions.

int run_once(std::mt19937& gen, std::binomial_distribution<int>& base_dist, int num_rows) {
    int num_accesses;

    // TODO: reduce int to int16_t?
    // TODO: check to see if doing these allocations inside run_once matters for performance.
    // We could hoist this out and just clear the vectors in here.
    std::vector<std::vector<int>> universe(num_rows, std::vector<int>(NUM_BLOCKS));
    for (int j = 0; j < universe[0].size(); ++j) {
        universe[0][j] = base_dist(gen);
        ++num_accesses;
    }

    for (int i = 1; i < universe.size(); ++i) {
        for (int j = 0; j < universe[0].size(); ++j) {
            if (universe[i-1][j] == 0) {
                universe[i][j] = 0;
            } else {
                ++num_accesses;
                int row_num_set = base_dist(gen);
                float previous_fraction = static_cast<float>(universe[i-1][j]) / 512.0;
                std::binomial_distribution<int> intersection_dist(row_num_set,previous_fraction);
                universe[i][j] = intersection_dist(gen);
            }
        }
    }
    return num_accesses;
}

int main()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::binomial_distribution<int> base_dist(511, 0.2);

    for (int i = 1; i <= MAX_NUM_ROWS; ++i) {
        std::cout << i;
        if (i <= MAX_NUM_ROWS-1) {
            std::cout << ",";
        }
    }
    std::cout << std::endl;

    for (int i = 0; i < NUM_ITERS; ++i) {
        for (int j = 1; j <= MAX_NUM_ROWS; ++j) {
            std::cout << run_once(gen, base_dist, j);
            if (j <= MAX_NUM_ROWS-1) {
                std::cout << ",";
            }
        }
        std::cout << std::endl;
    }
    return 0;
}
