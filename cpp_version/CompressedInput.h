#pragma once
#include "JobDef.h"
#include <vector>
#include <map>
#include <assert.h>

enum class Antagonist
{
    BINTREE_ANTAGONIST,
    GREEDY_ANTAGONIST
};

class CompressedInput {
public:
    std::map<jobPrecision, jobPrecision> jobSizes;
private:
    jobPrecision cacheSize = 0;
    jobPrecision cacheRealSize = 0;
    jobPrecision cacheHeight = 0;
    bool dirty = false;
    bool storeTimes = false;
    std::vector<Job> startTimes;

public:
    CompressedInput() {}

    CompressedInput(std::map<jobPrecision, jobPrecision> initial, jobPrecision iterCount, Antagonist alg);

    void addJob(jobPrecision size, jobPrecision count);

    jobPrecision getIdx(jobPrecision idx);

    jobPrecision getSize();

    jobPrecision getRealSize();

    jobPrecision getHeight();

    void reinitializeSizes();

    jobPrecision placementToIdx(jobPrecision place);

    void setJobStartTime(jobPrecision idx, jobPrecision start);

    bool tryToStoreTimes(bool enable);

    bool checkFeasibility();
};

jobPrecision binTreeCompressed(CompressedInput& jobs);