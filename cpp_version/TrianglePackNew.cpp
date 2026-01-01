#include <iostream>
#include <chrono>
#include "JobDef.h"
#include "CompressedInput.h"

int main()
{
    // We upscale the manageably sized input programmatically
    std::map<jobPrecision, jobPrecision> initialBintree = { {18, 1}, {9, 1}, {3, 4}, {1, 12} };
    std::map<jobPrecision, jobPrecision> initialGreedy = { {96, 1}, {36, 1}, {24, 2}, {5, 12}, {3, 16} };
    CompressedInput tiny(initialGreedy, 0, Antagonist::GREEDY_ANTAGONIST);
    CompressedInput small(initialGreedy, 1, Antagonist::GREEDY_ANTAGONIST);
    CompressedInput big(initialGreedy, 2, Antagonist::GREEDY_ANTAGONIST);
    CompressedInput huge(initialGreedy, 3, Antagonist::GREEDY_ANTAGONIST);

    auto alg_start = std::chrono::high_resolution_clock::now();

    binTreeCompressed(tiny);

    auto alg_end = std::chrono::high_resolution_clock::now();

    std::cout << "the algorithm took "
        << std::chrono::duration_cast<std::chrono::seconds>(alg_end - alg_start).count()
        << " seconds\n";

    return 0;
}