#include <iostream>
#include <queue>
#include <cmath>
#include <map>
#include <numeric>
#include <algorithm>
#include <chrono>
#include <fstream>

typedef unsigned long long jobType;
namespace old {
    class CompressedInput {
    public:
        std::map<jobType, jobType> jobSizes;
    private:
        jobType cacheSize;
        jobType cacheRealSize;
        jobType cacheHeight;
        bool dirty;

    public:
        CompressedInput() : cacheSize(0), cacheHeight(0), dirty(false), cacheRealSize(0) {}

        void addJob(jobType size, jobType count) {
            if (jobSizes.find(size) != jobSizes.end()) {
                jobSizes[size] += count;
            }
            else {
                jobSizes[size] = count;
            }
            dirty = true;
        }

        jobType getIdx(jobType idx) {
            if (idx > getRealSize()) {
                return 0; // virtual job
            }
            std::vector<jobType> sortedKeys;
            for (auto const& element : jobSizes) {
                sortedKeys.push_back(element.first);
            }
            std::sort(sortedKeys.begin(), sortedKeys.end(), std::greater<jobType>());

            jobType itrIdx = 0;
            for (jobType key : sortedKeys) {
                itrIdx += jobSizes[key];
                if (itrIdx > idx) {
                    return key;
                }
            }
            return 0;
        }

        jobType getSize() {
            if (dirty) {
                reinitializeSizes();
            }
            return cacheSize;
        }

        jobType getRealSize() {
            if (dirty) {
                reinitializeSizes();
            }
            return cacheRealSize;
        }

        jobType getHeight() {
            if (dirty) {
                reinitializeSizes();
            }
            return cacheHeight;
        }

        void reinitializeSizes() {
            jobType tempvar = std::accumulate(jobSizes.begin(), jobSizes.end(), jobType(0),
                [](jobType acc, const std::pair<jobType, jobType>& p) {
                return acc + p.second;
            });
            cacheRealSize = tempvar;
            cacheHeight = static_cast<jobType>(std::ceil(std::log2(tempvar)));
            cacheSize = static_cast<jobType>(std::pow(2, cacheHeight));
            dirty = false;
        }

        // indexing on a full binary tree is easier, and we just return empty jobs if needed.
        // the inverse of this function, idxToPlacement, generates an increasing non-continuous sequence of jobs, and null jobs can be skipped when iterating the array
        // Python code, just for variety's sake
        // order[len(order) // 2] = jobs[1]
        // idx = 0
        //    for idx in range(2, len(jobs)) :
        //        placement = len(order) / 2
        //        cap = math.ceil(math.log(idx + 1, 2)) + 1
        //        for logidx in range(2, cap) :
        //            sign = idx & (1 << cap - 1 - logidx) and 1 or -1
        //            placement += sign * len(order) / math.pow(2, logidx)
        //            placement = int(placement)
        //            order[placement] = jobs[idx]
        jobType placementToIdx(jobType place) {
            if (place == 0) {
                return 0;
            }
            jobType size = getSize();

            jobType rowIdxSearcher = 2;
            jobType rowNum = 1;

            while (place % rowIdxSearcher == 0) {
                rowIdxSearcher *= 2;
                rowNum += 1;
            }

            jobType rowIdx = getHeight() - rowNum;
            jobType inRowIdx = place / rowIdxSearcher;
            jobType beginIdx = size / rowIdxSearcher;
            jobType idx = beginIdx + inRowIdx;
            return idx;
        }
    };

    void binTreeCompressed(CompressedInput& jobs) {
        // minheap priority queue, in cpp the default is maxheap
        std::priority_queue<std::tuple<jobType, jobType, jobType, jobType>, std::vector<std::tuple<jobType, jobType, jobType, jobType>>, std::greater<std::tuple<jobType, jobType, jobType, jobType>>> startTimes;
        // always pack the first job at t=0
        jobType priority = jobs.getIdx(0);
        // tuple params:
        // gap start time, gap height, "collision mode", ceiling start time
        // gap start time: here, we consider finite gaps to be right-angle triangles or trapezoids, or an infinite gap to be a quarter plane, this is the bottom left point's time coordinate
        // gap height: the rightmost vertical edge of the gap, ignored in the collision mode 0 case
        // collision mode: 0 means only rightmost point to current job left leg, 1 means only ceiling to current job right angle corner, 2 means both collision options are possible
        // ceiling start time: the timestamp at which the currently investigated gap's ceiling began, because the gap start time refers to the small job that is responsible for the rightmost point collision
        // this can probably be simplified, but I really liked and got stuck in the mentality of popping the gaps off by earliest possible start time for any priority job, size-agnostically
        startTimes.push({ 0, priority, 1, 0 });
        startTimes.push({ priority, priority, 0, 0 });
        // for the 11 billion job list
        jobType infoUnit = jobs.getSize() / 1000;
        jobType infoCounter = 0;
        jobType makespan = priority;
        jobType start = 0;
        std::fstream out;
        //out.open("E:/bintree_out_oldalg.txt", std::fstream::out);
        for (jobType packIdx = 1; packIdx < jobs.getSize(); packIdx++) {
            infoCounter += 1;
            if (infoCounter > infoUnit) {
                std::cout << packIdx << " out of " << jobs.getSize() << " already packed for a makespan of " << makespan << std::endl;
                //out.flush();
                infoCounter = 0;
            }
            // can be replaced with job object if we're using a container for real jobs
            priority = jobs.getIdx(jobs.placementToIdx(packIdx));
            if (!priority) {
                continue;
            }
            while (true) {
                std::tuple<jobType, jobType, jobType, jobType> testTime = startTimes.top();
                startTimes.pop();
                // we'd fit under this ceiling
                if (priority < std::get<1>(testTime)) {
                    // sanity check, if this happens, we have messed up the entire algorithm, depending on language, this either deadlocks or crashes or some other undesirable behavior
                    if (startTimes.empty()) {
                        throw std::runtime_error("asd");
                    }
                    std::tuple<jobType, jobType, jobType, jobType> nextTime = startTimes.top();
                    startTimes.pop();
                    // here we pick collision mode if both are available
                    if (std::get<2>(testTime) == 2) {
                        start = std::max(std::get<3>(testTime) + priority, std::get<0>(testTime));
                    }
                    // here it is forced
                    else {
                        start = std::get<0>(testTime) + std::get<2>(testTime) * priority;
                    }
                    // we are not cutting into the next gap, so it remains a valid gap, and we put it back along with our own
                    if (start + priority < std::get<0>(nextTime)) {
                        startTimes.push({ start + priority, std::get<1>(testTime), 2, std::get<3>(testTime) });
                        //std::cout << "Trapezoid gap " << start + priority << " " << std::get<1>(testTime) << " " << std::get<3>(testTime) << "\n";
                        startTimes.push(nextTime);
                        //std::cout << "Next gap: " << std::get<0>(nextTime) << " " << std::get<1>(nextTime) << " " << std::get<2>(nextTime) << " " << std::get<3>(nextTime) << "\n";
                    }
                    else {
                        // here we drop ALL the ceilings whose trapezoids we are cutting into, and this might even include pushing the infinite gap further to the right
                        // technically, trapezoid gaps can be degenerate (0-width), for example with inputs like 8, 4, 2, 2, 1, 1, 1, 1, where multiple rightmost points are above each other.
                        // we do handle that case, though, and all of those ceilings are popped in this branch if they weren't already popped by the "current job does not fit here" branch.
                        jobType ceilHeight = 0;
                        jobType ceilStart = 0;
                        auto tempTime = nextTime;
                        jobType mode = 2;
                        if (startTimes.empty()) {
                            ceilHeight = priority;
                            ceilStart = start;
                            mode = 0;
                        }
                        else {
                            while (true) {
                                tempTime = nextTime;
                                if (startTimes.empty()) {
                                    tempTime = nextTime;
                                    break;
                                }
                                nextTime = startTimes.top();
                                startTimes.pop();
                                if (start + priority < std::get<0>(nextTime) || startTimes.empty()) {
                                    break;
                                }
                            }
                            ceilHeight = std::get<1>(tempTime);
                            ceilStart = std::get<3>(tempTime);
                            startTimes.push(nextTime);
                            //std::cout << "Next gap alt: " << std::get<0>(nextTime) << " " << std::get<1>(nextTime) << " " << std::get<2>(nextTime) << " " << std::get<3>(nextTime) << "\n";
                        }
                        startTimes.push({ start + priority, ceilHeight, mode, ceilStart });
                        //std::cout << "follow gap alt: " << start + priority << " " << ceilHeight << " " << mode << " " << ceilStart << "\n";
                        makespan = std::max(makespan, start + priority);
                    }
                    startTimes.push({ start, priority, 1, start });
                    //std::cout << "self triangle gap: " << start << " " << priority << " " << start << "\n";
                    break;
                }
                // this is for when we overhang ALL the previous ceilings, the most trivial example of this is when the input consists of uniform priority jobs.
                if (startTimes.empty()) {
                    start = std::get<0>(testTime);
                    startTimes.push({ start, priority, 1, start });
                    //std::cout << "self triangle recreate: " << start << " " << priority << " " << start << "\n";
                    startTimes.push({ start + priority, priority, 0, start });
                    //std::cout << "infinite recreate: " << start + priority << " " << priority << " " << start << "\n";
                    makespan = std::max(makespan, start + priority);
                    break;
                }
            }
            // this is where you assign the start value to a job object if you have an actual job object
            //out << packIdx << " " << jobs.placementToIdx(packIdx) << " " << priority << " " << start << "\n";
        }
        std::cout << makespan << std::endl;
    }
}

int maina() {
    old::CompressedInput input;
    // We upscale the manageably sized input programmatically
    std::vector<jobType> jobSizes = { 104976, 52488, 17496, 5832, 2916, 972, 324, 162, 54, 18, 9, 3, 1 };
    std::vector<jobType> jobCounts = { 1, 1, 4, 12, 18, 72, 216, 324, 1296, 3888, 5832, 23328, 69984 };
    //std::vector<jobType> jobSizes = { 6, 3, 1 };
    //std::vector<jobType> jobCounts = { 1, 1, 4 };
    for (size_t i = 0; i < jobSizes.size(); ++i) {
        input.addJob(jobSizes[i] * jobSizes[0], jobCounts[i]);
        if (jobSizes[i] != jobSizes[0]) {
            input.addJob(jobSizes[i], jobCounts[i] * jobSizes[0]);
        }
    }

    //std::map<jobType, jobType> initialGreedy = { {96, 1}, {36, 1}, {24, 2}, {5, 12}, {3, 16} };
    //for (auto& j : initialGreedy)
    //    input.addJob(j.first, j.second);

    auto alg_start = std::chrono::high_resolution_clock::now();

    old::binTreeCompressed(input);

    auto alg_end = std::chrono::high_resolution_clock::now();

    std::cout << "the algorithm took "
        << std::chrono::duration_cast<std::chrono::seconds>(alg_end - alg_start).count()
        << " seconds\n";

    return 0;
}