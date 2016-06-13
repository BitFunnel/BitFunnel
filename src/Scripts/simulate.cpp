#include <iostream>
#include <random>

#define MAX_NUM_ROWS 10
#define NUM_ITERS 10
#define BLOCK_SIZE 512 // cache line size in bits.
#define NUM_BLOCKS 10000
#define NUM_DOCS BLOCK_SIZE * NUM_BLOCKS

// DESIGN NOTE: tried using go, but the publicly available binomial rng is approximately 10x slower.
// TODO: change conventions to match BitFunnel coding conventions.

int run_once(std::mt19937& gen, std::binomial_distribution<int16_t>& base_dist, int num_rows) {
    int num_accesses = 0;

    // TODO: check to see if doing these allocations inside run_once matters for performance.
    // We could hoist this out and just clear the vectors in here.
    std::vector<int16_t> universe(NUM_BLOCKS, 0);
    for (int i = 0; i < universe.size(); ++i) {
        universe[i] = base_dist(gen);
        ++num_accesses;
    }

    for (int i = 1; i < num_rows; ++i) {
        for (int j = 0; j < universe.size(); ++j) {
            if (universe[j] == 0) {
                // universe[i] = 0;
            } else {
                ++num_accesses;
                int16_t row_num_set = base_dist(gen);
                float previous_fraction = static_cast<float>(universe[j]) / 512.0;
                std::binomial_distribution<int16_t> intersection_dist(row_num_set,previous_fraction);
                universe[j] = intersection_dist(gen);
            }
        }
    }
    return num_accesses;
}

int main()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::binomial_distribution<int16_t> base_dist(511, 0.2);

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
