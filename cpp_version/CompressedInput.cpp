#include "CompressedInput.h"
#include "JobDef.h"
#include <queue>
#include <cmath>
#include <numeric>
#include <map>
#include <algorithm>
#include <iostream>

void CompressedInput::addJob(jobPrecision size, jobPrecision count)
{
    if (jobSizes.find(size) != jobSizes.end())
    {
        jobSizes[size] += count;
    }
    else
    {
        jobSizes[size] = count;
    }
    dirty = true;
}

jobPrecision CompressedInput::getIdx(jobPrecision idx)
{
    if (idx > getRealSize())
    {
        return 0; // virtual job
    }
    std::vector<jobPrecision> sortedKeys;
    for (auto const& element : jobSizes)
    {
        sortedKeys.push_back(element.first);
    }
    std::sort(sortedKeys.begin(), sortedKeys.end(), std::greater<jobPrecision>());

    jobPrecision itrIdx = 0;
    for (jobPrecision key : sortedKeys)
    {
        itrIdx += jobSizes[key];
        if (itrIdx > idx)
        {
            return key;
        }
    }
    return 0;
}

jobPrecision CompressedInput::getSize()
{
    if (dirty) {
        reinitializeSizes();
    }
    return cacheSize;
}

jobPrecision CompressedInput::getRealSize()
{
    if (dirty) {
        reinitializeSizes();
    }
    return cacheRealSize;
}

jobPrecision CompressedInput::getHeight()
{
    if (dirty) {
        reinitializeSizes();
    }
    return cacheHeight;
}

void CompressedInput::reinitializeSizes()
{
    jobPrecision tempvar = std::accumulate(jobSizes.begin(), jobSizes.end(), jobPrecision(0),
        [](jobPrecision acc, const std::pair<jobPrecision, jobPrecision>& p) {
        return acc + p.second;
    });
    cacheRealSize = tempvar;
    cacheHeight = static_cast<jobPrecision>(std::ceil(std::log2(tempvar)));
    cacheSize = static_cast<jobPrecision>(std::pow(2, cacheHeight));
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
jobPrecision CompressedInput::placementToIdx(jobPrecision place)
{
    if (place == 0) {
        return 0;
    }
    jobPrecision size = getSize();

    jobPrecision rowIdxSearcher = 2;
    jobPrecision rowNum = 1;

    while (place % rowIdxSearcher == 0) {
        rowIdxSearcher *= 2;
        rowNum += 1;
    }

    jobPrecision rowIdx = getHeight() - rowNum;
    jobPrecision inRowIdx = place / rowIdxSearcher;
    jobPrecision beginIdx = size / rowIdxSearcher;
    jobPrecision idx = beginIdx + inRowIdx;
    return idx;
}

struct CompareJobGreater
{
    bool operator()(const std::shared_ptr<Gap>& t1, std::shared_ptr<Gap>& t2)
    {
        return t1->startTime > t2->startTime;
    }
};

void binTreeCompressed(CompressedInput& jobs)
{
    // minheap priority queue, in cpp the default is maxheap
    std::priority_queue<std::shared_ptr<Gap>, std::vector<std::shared_ptr<Gap>>, CompareJobGreater> startTimes_;
    // always schedule the first job at t=0
    jobPrecision priority = jobs.getIdx(0);
    // gap start time: here, we consider finite gaps to be right-angle triangles or trapezoids, or an infinite gap to be a quarter plane, this is the bottom left point's time coordinate
    // gap height: the rightmost vertical edge of the gap for triangles and trapezoids, and the left edge of the infinite gap (for infinite gaps, it is mostly irrelevant either way)
    // ceiling start time: the timestamp at which the currently investigated gap's ceiling began, because we can collide into that ceiling, not just the previous job
    // "ceilings" are relevant for trapezoid gaps only, but since trapezoid gaps can be created directly under triangle gaps AND by slicing off a trapezoid gap, they need to be able to request the big parent triangle's starting time point from any situation
    startTimes_.push(std::make_shared<TriangleGap>(TriangleGap(0, priority, 0)));
    startTimes_.push(std::make_shared<InfiniteGap>(InfiniteGap(priority, priority, 0)));
    // for the 11 billion job list
    jobPrecision infoUnit = jobs.getSize() / 1000;
    jobPrecision infoCounter = 0;
    jobPrecision makespan = priority;
    jobPrecision start = 0;
    for (jobPrecision packIdx = 1; packIdx < jobs.getSize(); packIdx++)
    {
        infoCounter += 1;
        if (infoCounter > infoUnit)
        {
            std::cout << packIdx << " out of " << jobs.getSize() << " already packed for a makespan of " << makespan << std::endl;
            infoCounter = 0;
        }
        // can be replaced with job object if we're using a container for real jobs
        priority = jobs.getIdx(jobs.placementToIdx(packIdx));
        if (!priority)
        {
            continue;
        }
        while (true)
        {
            auto testGap = startTimes_.top();
            startTimes_.pop();
            // we'd fit under this ceiling
            if (priority < testGap->gapHeight)
            {
                // sanity check, if this happens, we have messed up the entire algorithm, depending on language, this either deadlocks or crashes or some other undesirable behavior
                if (startTimes_.empty())
                {
                    throw std::runtime_error("asd");
                }
                auto nextGap = startTimes_.top();
                startTimes_.pop();
                start = testGap->getStartTimeFor(priority);
                auto selfGap = std::make_shared<TriangleGap>(TriangleGap(start, priority, start));
                // here we drop ALL the ceilings whose trapezoids we are cutting into, and this might even include pushing the infinite gap further to the right
                // technically, trapezoid gaps can be degenerate (0-width), for example with inputs like 8, 4, 2, 2, 1, 1, 1, 1, where multiple rightmost points are above each other.
                // we do handle that case, though, and all of those ceilings are popped
                // _____
                // p_k//
                //   //
                //  //____
                // //p_l//
                // ....
                // This is what is being cleared out here, and p_l can theoretically invalidate out any number of gaps created by earlier jobs
                jobPrecision ceilHeight = 0;
                jobPrecision ceilStart = 0;
                while (nextGap->startTime <= start + priority)
                {
                    if (startTimes_.empty())
                    {
                        break;
                    }
                    testGap = nextGap;
                    nextGap = startTimes_.top();
                    startTimes_.pop();
                }
                // we are not cutting into the next gap, so it remains a valid gap, and we put it back along with our own
                if (start + priority < nextGap->startTime)
                {
                    ceilHeight = testGap->gapHeight;
                    ceilStart = testGap->ceilingStart;
                    //std::cout << "Found gaps: ";
                    auto newGap = std::make_shared<TrapezoidGap>(TrapezoidGap(start + priority, ceilHeight, ceilStart));
                    startTimes_.push(newGap);
                    //std::cout << *newGap << *nextGap << *selfGap;
                    startTimes_.push(nextGap);
                    startTimes_.push(selfGap);
                    makespan = std::max(makespan, start + priority);
                    break;
                }
            }
            // this is for when we overhang ALL the previous ceilings, the most trivial example of this is when the input consists of uniform priority jobs.
            if (startTimes_.empty())
            {
                start = testGap->startTime;
                auto addGap = std::make_shared<InfiniteGap>(InfiniteGap(start + priority, priority, start));
                auto selfGap = std::make_shared<TriangleGap>(TriangleGap(start, priority, start));
                startTimes_.push(addGap);
                startTimes_.push(selfGap);
                //std::cout << "Reinit: " << *selfGap << *addGap;
                makespan = std::max(makespan, start + priority);
                break;
            }
        }
        // this is where you assign the start value to a job object if you have an actual job object
    }
    std::cout << makespan << std::endl;
}