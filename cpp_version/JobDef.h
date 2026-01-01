#pragma once
typedef unsigned long long jobPrecision;
#include <string>

struct Job
{
    jobPrecision priority;
    jobPrecision startTime = 0;
    Job(jobPrecision p) : priority(p) {}
};

// gap start time, gap height, "collision mode", ceiling start time
struct Gap
{
    jobPrecision startTime;
    jobPrecision gapHeight;
    jobPrecision ceilingStart;

    Gap(jobPrecision s, jobPrecision h, jobPrecision c) : startTime(s), gapHeight(h), ceilingStart(c) {}

    virtual jobPrecision getStartTimeFor(jobPrecision jobPriority) = 0;
    virtual bool insert(Job job)
    {
        return job.priority < gapHeight;
    }
    virtual operator std::string() const;
    friend std::ostream& operator<<(std::ostream& _stream, Gap const& g) {
        _stream << std::string(g);
        return _stream;
    }
};

struct TriangleGap : public Gap
{
    TriangleGap(jobPrecision s, jobPrecision h, jobPrecision c) : Gap(s, h, c) {}
    jobPrecision getStartTimeFor(jobPrecision jobPriority) override;
    virtual operator std::string() const override;
};

struct TrapezoidGap : public Gap
{
    TrapezoidGap(jobPrecision s, jobPrecision h, jobPrecision c) : Gap(s, h, c) {}
    jobPrecision getStartTimeFor(jobPrecision jobPriority) override;
    virtual operator std::string() const override;
};

struct InfiniteGap : public Gap
{
    InfiniteGap(jobPrecision s, jobPrecision h, jobPrecision c) : Gap(s, h, c) {}
    jobPrecision getStartTimeFor(jobPrecision jobPriority) override;
    bool insert(Job job) override;
    virtual operator std::string() const override;
};

jobPrecision iterSquareBlowup(jobPrecision base, jobPrecision count);