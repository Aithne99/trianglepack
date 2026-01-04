#include "CompressedInput.h"
#include "JobDef.h"
#include <queue>
#include <cmath>
#include <numeric>
#include <map>
#include <algorithm>
#include <iostream>

CompressedInput::CompressedInput(std::map<jobPrecision, jobPrecision> initial, jobPrecision iterCount, Antagonist alg)
{
    assert(initial.size() > 1);

    // we should be able to just take the provided map as the set of jobs
    if (!iterCount)
    {
        for (auto& mapItr : initial)
        {
            addJob(mapItr.first, mapItr.second);
        }
        return;
    }
    jobPrecision scaleFactor = 1;
    jobPrecision maxJobSize = initial.rbegin()->first;
    switch (alg)
    {
    case Antagonist::BINTREE_ANTAGONIST:
    {
        auto maxElement = initial.rbegin();
        ++maxElement;
        scaleFactor = maxElement->first * 2; // for combined antagonist inputs, we have to use a p_2 based scaling, as p_1 != 2 * p_2 for those
        --maxElement;
        break;
    }
    case Antagonist::GREEDY_ANTAGONIST:   
    {
        auto minElement = initial.begin();
        scaleFactor = minElement->second * 2;
    }
    default:
        break;
    }
    for (jobPrecision e = 1; e <= iterCount; ++e)
    {
        for (auto& mapItr : initial)
        {
            addJob(mapItr.first * iterSquareBlowup(scaleFactor, e), mapItr.second * iterSquareBlowup(scaleFactor, iterCount - e));
            if (mapItr.first != maxJobSize)
            {
                addJob(mapItr.first * iterSquareBlowup(scaleFactor, iterCount - e), mapItr.second * iterSquareBlowup(scaleFactor, e));
            }
        }
    }
}

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
    if (storeTimes)
    {
        startTimes.clear();
        startTimes.resize(cacheRealSize);
    }
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

void CompressedInput::setJobStartTime(jobPrecision idx, jobPrecision start)
{
    if (!storeTimes)
        return;
    jobPrecision jobIdx = placementToIdx(idx);
    jobPrecision priority = getIdx(jobIdx);
    startTimes[jobIdx].priority = priority;
    startTimes[jobIdx].startTime = start;
}

void CompressedInput::setGreedyJobStartTime(jobPrecision idx, jobPrecision start)
{
    startTimes[idx].startTime = start;
}

void CompressedInput::setGap(jobPrecision idx, jobPrecision gap)
{
    gapLengths[idx] = gap;
}

bool CompressedInput::tryToStoreTimes(bool enable)
{
    if (getRealSize() > 1024 * 1024 * 1024 / sizeof(Job))
        storeTimes = false;
    storeTimes = enable;
    if (storeTimes)
    {
        startTimes.clear();
        startTimes.resize(cacheRealSize);
    }
    return storeTimes;

}

struct CompareJobGreater
{
    bool operator()(const std::shared_ptr<Gap>& t1, std::shared_ptr<Gap>& t2)
    {
        return t1->startTime > t2->startTime;
    }
};

bool CompressedInput::checkFeasibility()
{
    if (!storeTimes || dirty)
        return true;

    bool ret = true;
    for (int i = 0; i < getRealSize(); ++i)
    {
        for (int j = i + 1; j < getRealSize(); ++j)
        {
            if (abs((long long)startTimes[i].startTime - (long long)startTimes[j].startTime) < std::min(startTimes[i].priority, startTimes[j].priority))
            {
                std::cout << "Failure between jobs " << i << " and " << j;
                ret = false;
            }
        }
    }
    return ret;
}

jobPrecision CompressedInput::binTreeCompressed()
{
    // minheap priority queue, in cpp the default is maxheap
    std::priority_queue<std::shared_ptr<Gap>, std::vector<std::shared_ptr<Gap>>, CompareJobGreater> startTimes_;
    // always schedule the first job at t=0
    jobPrecision priority = getIdx(0);
    // gap start time: here, we consider finite gaps to be right-angle triangles or trapezoids, or an infinite gap to be a quarter plane, this is the bottom left point's time coordinate
    // gap height: the rightmost vertical edge of the gap for triangles and trapezoids, and the left edge of the infinite gap (for infinite gaps, it is mostly irrelevant either way)
    // ceiling start time: the timestamp at which the currently investigated gap's ceiling began, because we can collide into that ceiling, not just the previous job
    // "ceilings" are relevant for trapezoid gaps only, but since trapezoid gaps can be created directly under triangle gaps AND by slicing off a trapezoid gap, they need to be able to request the big parent triangle's starting time point from any situation
    startTimes_.push(std::make_shared<TriangleGap>(TriangleGap(0, priority, 0)));
    startTimes_.push(std::make_shared<InfiniteGap>(InfiniteGap(priority, priority, 0)));
    setJobStartTime(0, 0);
    // for the 11 billion job list
    jobPrecision infoUnit = getSize() / 1000;
    if (log10(getSize()) < 8)
        infoUnit = getSize() / log10(getSize());
    std::cout << "Info unit: " << infoUnit << "\n";
    jobPrecision infoCounter = 0;
    jobPrecision makespan = priority;
    jobPrecision start = 0;
    for (jobPrecision packIdx = 1; packIdx < getSize(); packIdx++)
    {
        infoCounter += 1;
        if (infoCounter > infoUnit)
        {
            std::cout << packIdx << " out of " << getSize() << " already packed for a makespan of " << makespan << std::endl;
            infoCounter = 0;
        }
        // can be replaced with job object if we're using a container for real jobs
        priority = getIdx(placementToIdx(packIdx));
        if (!priority)
        {
            continue;
        }
        while (true)
        {
            auto testGap = startTimes_.top();
            startTimes_.pop();
            // we'd fit under this ceiling
            if (!testGap->insert(priority))
                continue;

            start = testGap->getStartTimeFor(priority);
            auto selfGap = std::make_shared<TriangleGap>(TriangleGap(start, priority, start));
            makespan = std::max(makespan, start + priority);
            // we are doing infinite gap here
            while (!startTimes_.empty())
            {
                auto nextGap = startTimes_.top();
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
                    testGap = nextGap;
                    nextGap = startTimes_.top();
                    startTimes_.pop();
                }
                if (startTimes_.empty())
                {
                    auto addGap = std::make_shared<InfiniteGap>(InfiniteGap(start + priority, priority, start));
                    startTimes_.push(addGap);
                }
                // we are not cutting into the next gap, so it remains a valid gap, and we put it back along with our own
                else
                {
                    ceilHeight = testGap->gapHeight;
                    ceilStart = testGap->ceilingStart;
                    //std::cout << "Found gaps: ";
                    startTimes_.push(selfGap);
                    auto newGap = std::make_shared<TrapezoidGap>(TrapezoidGap(start + priority, ceilHeight, ceilStart));
                    startTimes_.push(newGap);
                    //std::cout << *newGap << *nextGap << *selfGap;
                    startTimes_.push(nextGap);
                    makespan = std::max(makespan, start + priority);
                    break;
                }
            }
            startTimes_.push(selfGap);
        }
        // this is where you assign the start value to a job object if you have an actual job object
        setJobStartTime(packIdx, start);
    }
    std::cout << makespan << std::endl;
    return makespan;
}

jobPrecision CompressedInput::greedyCompressed()
{
    jobPrecision makespan = 0;
    startTimes.resize(getSize());
    gapLengths.resize(getSize());
    jobPrecision i = 0;
    for (auto jobsize = jobSizes.rbegin(); jobsize != jobSizes.rend(); ++jobsize)
    {
        for (jobPrecision j = 0; j < jobsize->second; ++j)
        {
            startTimes[i].priority = jobsize->first;
            gapLengths[i++] = jobsize->first;
        }
    }

    for (i = 1; i < getSize(); ++i)
    {
        jobPrecision maxjob = greedyGapSelect(i);
        setGreedyJobStartTime(i, startTimes[maxjob].startTime + startTimes[i].priority);
        if (2 * startTimes[i].priority > gapLengths[maxjob])
        {
            jobPrecision delta = 2 * startTimes[i].priority - gapLengths[maxjob];
            for (jobPrecision j = 1; j < i; ++j)
            {
                if (startTimes[j].startTime > startTimes[i].startTime)
                    startTimes[j].startTime += delta;
            }
        }
        setGap(i, std::max(gapLengths[i], gapLengths[maxjob] - gapLengths[i]));
        setGap(maxjob, startTimes[i].priority);
    }

    for (i = 0; i < getSize(); ++i)
    {
        if (startTimes[i].length() > makespan)
            makespan = startTimes[i].length();
    }
    std::cout << makespan << std::endl;
    return makespan;
}

jobPrecision CompressedInput::greedyGapSelect(jobPrecision i)
{
    jobPrecision ret = 0;
    for (jobPrecision j = 0; j < i; ++j)
    {
        if (gapLengths[j] > gapLengths[ret])
        {
            ret = j;
        }
    }
    return ret;
}
