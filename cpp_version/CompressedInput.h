#pragma once
#include "JobDef.h"
#include <map>
class CompressedInput {
public:
    std::map<jobPrecision, jobPrecision> jobSizes;
private:
    jobPrecision cacheSize = 0;
    jobPrecision cacheRealSize = 0;
    jobPrecision cacheHeight = 0;
    bool dirty = false;

public:
    CompressedInput() {}

    CompressedInput(std::map<jobPrecision, jobPrecision> initial, jobPrecision iterCount);

    void addJob(jobPrecision size, jobPrecision count);

    jobPrecision getIdx(jobPrecision idx);

    jobPrecision getSize();

    jobPrecision getRealSize();

    jobPrecision getHeight();

    void reinitializeSizes();

    jobPrecision placementToIdx(jobPrecision place);
};

void binTreeCompressed(CompressedInput& jobs);