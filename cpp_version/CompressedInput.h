#pragma once
#include "JobDef.h"
#include <map>
class CompressedInput {
public:
    std::map<jobPrecision, jobPrecision> jobSizes;
private:
    jobPrecision cacheSize;
    jobPrecision cacheRealSize;
    jobPrecision cacheHeight;
    bool dirty;

public:
    CompressedInput() : cacheSize(0), cacheHeight(0), dirty(false), cacheRealSize(0) {}

    void addJob(jobPrecision size, jobPrecision count);

    jobPrecision getIdx(jobPrecision idx);

    jobPrecision getSize();

    jobPrecision getRealSize();

    jobPrecision getHeight();

    void reinitializeSizes();

    jobPrecision placementToIdx(jobPrecision place);
};

void binTreeCompressed(CompressedInput& jobs);