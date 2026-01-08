#include <iostream>
#include <chrono>
#include "JobDef.h"
#include "CompressedInput.h"
#include <random>
#include <fstream>

int main()
{
    // We upscale the manageably sized input programmatically
    std::map<jobPrecision, jobPrecision> smallGreedy = { {8, 1}, {3, 1}, {2, 2}};
    std::map<jobPrecision, jobPrecision> smallBintree = { {6, 1}, {3, 1}, {1, 4}};
    std::map<jobPrecision, jobPrecision> initialBintree = { {18, 1}, {9, 1}, {3, 4}, {1, 12} }; // 18 jobs
    std::map<jobPrecision, jobPrecision> initialGreedy = { {48, 1}, {18, 1}, {12, 2}, {5, 4}, {3, 8} }; // 16 jobs
    std::map<jobPrecision, jobPrecision> initialMixedEpsilon = { {180, 1}, {49, 2}, {30, 3}, {10, 12} };

    std::vector<jobPrecision> jobSizes = { 104976, 52488, 17496, 5832, 2916, 972, 324, 162, 54, 18, 9, 3, 1 };
    std::vector<jobPrecision> jobCounts = { 1, 1, 4, 12, 18, 72, 216, 324, 1296, 3888, 5832, 23328, 69984 };

    //for (size_t i = 0; i < jobSizes.size(); ++i)
    // {
    //    input.addJob(jobSizes[i] * jobSizes[0], jobCounts[i]);
    //    if (jobSizes[i] != jobSizes[0]) {
    //        input.addJob(jobSizes[i], jobCounts[i] * jobSizes[0]);
    //    }
    //}

    std::mt19937 rng;
    rng.seed(68);
    std::uniform_real_distribution<double> dist(0.4, 0.6); // Bintree ratio will be near 2
    //std::fstream out;
    //out.open(<filename>);
    for (int i = 24; i < 51; ++i)
    {
        std::vector<CompressedInput> inputs(3);
        std::map<jobPrecision, jobPrecision> inputMap = { { i, 1 } };
        double size = i;
        double count = 1;
        jobPrecision total = 1;

        while (size > 1 && total < i)
        {
            size *= dist(rng);
            count *= 1.0 + dist(rng);
            inputMap[(jobPrecision)round(size + 0.5)] = (jobPrecision)count;
            total += (jobPrecision)count;
        }
        auto itr = inputMap.find(1);
        if (itr == inputMap.end())
            inputMap[1] = i - total;
        else
            itr->second += i - total;

        for (auto& mapItr : inputMap)
        {
            inputs[0].addJob(mapItr.first, mapItr.second);
        }

        for (auto& mapItr : inputs[0].jobSizes)
        {
            inputs[1].addJob(mapItr.first * i, mapItr.second);
            if (mapItr.first != i)
            {
                inputs[1].addJob(mapItr.first, mapItr.second * i);
            }
        }

        for (auto& mapItr : inputs[1].jobSizes)
        {
            inputs[2].addJob(mapItr.first * i * i, mapItr.second);
            if (mapItr.first != i * i) {
                inputs[2].addJob(mapItr.first, mapItr.second * i * i);
            }
        }

        //for (auto& input : inputs)
        //{
        //    out << "Input " << i << " size " << input.getRealSize() << "\n";
        //    for (auto& elem : input.jobSizes)
        //    {
        //        out << elem.first << " " << elem.second << "\n";
        //    }
        //    out << "\n";
        //}

        //out << "\n";
        //out.flush();

        if (i < 38)
            continue;

        for (auto& input : inputs)
        {
            jobPrecision lb = input.calcLowerBound();
            std::cout << "\n--------------------------------------------------------\n\n";
            std::cout << "Lower bound for input " << i << " size " << input.getRealSize() << " is " << lb << "\n";
            {
                auto greedy_start = std::chrono::high_resolution_clock::now();

                jobPrecision makespan = input.greedyCompressed();

                auto greedy_end = std::chrono::high_resolution_clock::now();

                std::cout << "On input " << i << " size " << input.getRealSize() << " the greedy algorithm took "
                    << std::chrono::duration_cast<std::chrono::seconds>(greedy_end - greedy_start).count()
                    << " seconds\n";

                std::cout << "Approximation ratio estimate for greedy: " << (double)makespan / lb << "\n";

                std::cout << "\n-----------------------\n";

                auto bintree_start = std::chrono::high_resolution_clock::now();

                makespan = input.binTreeCompressed();

                auto bintree_end = std::chrono::high_resolution_clock::now();

                std::cout << "On input " << i << " the bintree algorithm took "
                    << std::chrono::duration_cast<std::chrono::seconds>(bintree_end - bintree_start).count()
                    << " seconds\n";

                std::cout << "Approximation ratio estimate for bintree: " << (double)makespan / (double)lb << "\n";
            }
        }
    }

    return 0;
}