#include <iostream>
#include <chrono>
#include "JobDef.h"
#include "CompressedInput.h"

int main() {
    CompressedInput input;
    // We upscale the manageably sized input programmatically
    //std::vector<jobPrecision> jobSizes = { 104976, 52488, 17496, 5832, 2916, 972, 324, 162, 54, 18, 9, 3, 1 };
    //std::vector<jobPrecision> jobCounts = { 1, 1, 4, 12, 18, 72, 216, 324, 1296, 3888, 5832, 23328, 69984 };
    
    std::vector<jobPrecision> jobSizes = { 18, 9, 3, 1 };
    std::vector<jobPrecision> jobCounts = { 1, 1, 4, 12 };
    for (size_t i = 0; i < jobSizes.size(); ++i) {
        input.addJob(jobSizes[i] * jobSizes[0], jobCounts[i]);
        if (jobSizes[i] != jobSizes[0]) {
            input.addJob(jobSizes[i], jobCounts[i] * jobSizes[0]);
        }
    }

    std::map<jobPrecision, jobPrecision> initial = { {18, 1}, {9, 1}, {3, 4}, {1, 12} };
    CompressedInput small(initial, 1);
    CompressedInput big(initial, 2);
    CompressedInput huge(initial, 3);

    auto alg_start = std::chrono::high_resolution_clock::now();

    binTreeCompressed(input);

    auto alg_end = std::chrono::high_resolution_clock::now();

    std::cout << "the algorithm took "
        << std::chrono::duration_cast<std::chrono::seconds>(alg_end - alg_start).count()
        << " seconds\n";

    return 0;
}