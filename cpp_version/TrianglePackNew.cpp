#include <iostream>
#include <chrono>
#include "JobDef.h"
#include "CompressedInput.h"

int main()
{
    // We upscale the manageably sized input programmatically
    std::map<jobPrecision, jobPrecision> smallGreedy = { {8, 1}, {3, 1}, {2, 2}};
    std::map<jobPrecision, jobPrecision> smallBintree = { {6, 1}, {3, 1}, {1, 4}};
    std::map<jobPrecision, jobPrecision> initialBintree = { {18, 1}, {9, 1}, {3, 4}, {1, 12} }; // 18 jobs
    std::map<jobPrecision, jobPrecision> initialGreedy = { {48, 1}, {18, 1}, {12, 2}, {5, 4}, {3, 8} }; // 16 jobs
    std::map<jobPrecision, jobPrecision> initialMixed = { {180, 1}, {49, 2}, {30, 3}, {10, 12} };

    std::vector<CompressedInput> inputs =
    {
    //CompressedInput(initialGreedy, 1, Antagonist::GREEDY_ANTAGONIST),
    //CompressedInput(initialBintree, 1, Antagonist::BINTREE_ANTAGONIST),
    //CompressedInput(initialGreedy, 2, Antagonist::GREEDY_ANTAGONIST),
    //CompressedInput(initialBintree, 2, Antagonist::BINTREE_ANTAGONIST),
    //CompressedInput(initialMixed, 2, Antagonist::GREEDY_ANTAGONIST),
    //CompressedInput(initialMixed, 2, Antagonist::BINTREE_ANTAGONIST),
    CompressedInput(initialGreedy, 3, Antagonist::GREEDY_ANTAGONIST),
    CompressedInput(initialBintree, 3, Antagonist::BINTREE_ANTAGONIST),
    //CompressedInput(initialMixed, 3, Antagonist::GREEDY_ANTAGONIST),
    //CompressedInput(initialMixed, 3, Antagonist::BINTREE_ANTAGONIST),
    };

    for (int i = 0; i < inputs.size(); ++i)
    {
        std::cout << "--------------------------------------------------------\n\n";
        auto alg_start = std::chrono::high_resolution_clock::now();
        bool checkFeas = inputs[i].tryToStoreTimes(true);

        jobPrecision makespan = inputs[i].binTreeCompressed();

        auto alg_end = std::chrono::high_resolution_clock::now();
        std::string inputtype(i % 2 ? " (anti-Bintree)" : " (anti-Greedy)");

        std::cout << "On input #" << i + 1 << inputtype << " the algorithm took "
            << std::chrono::duration_cast<std::chrono::seconds>(alg_end - alg_start).count()
            << " seconds\n";

        if (checkFeas)
        {
            auto feas_start = std::chrono::high_resolution_clock::now();

            if (!inputs[i].checkFeasibility())
                throw(std::exception());

            auto feas_end = std::chrono::high_resolution_clock::now();

            std::cout << "On input #" << i + 1 << inputtype << " feasibility checking took "
                << std::chrono::duration_cast<std::chrono::seconds>(feas_end - feas_start).count()
                << " seconds\n";
        }
        else
        {
            std::cout << "On input #" << i + 1 << inputtype << " feasibility checking is not tractable (allocated array would exceed one gigabyte of memory), skipped";
        }

        double opt = inputs[i].jobSizes.rbegin()->first;
        std::cout << "Approximation ratio: " << (double)makespan / opt << "\n";
    }

    return 0;
}