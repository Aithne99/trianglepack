#include "JobDef.h"
#include <numeric>
#include <sstream>

Gap::operator std::string() const
{
    std::stringstream ret;
    ret << "Gap :" << startTime << " " << gapHeight << " " << ceilingStart << "\n";
    return ret.str();
}

jobPrecision TriangleGap::getStartTimeFor(jobPrecision jobPriority)
{
    jobPrecision start = startTime + jobPriority;
    return start;
}

TriangleGap::operator std::string() const
{
    std::stringstream ret;
    ret << "Triangle " << Gap::operator std::string();
    return ret.str();
}


jobPrecision TriangleGap::insert(Job job)
{
    return jobPrecision();
}

jobPrecision InfiniteGap::getStartTimeFor(jobPrecision jobPriority)
{
    jobPrecision start = startTime;
    return start;
}


InfiniteGap::operator std::string() const
{
    std::stringstream ret;
    ret << "Infinite " << Gap::operator std::string();
    return ret.str();
}


jobPrecision InfiniteGap::insert(Job job)
{
    return jobPrecision();
}

jobPrecision TrapezoidGap::getStartTimeFor(jobPrecision jobPriority)
{
    jobPrecision start = std::max(ceilingStart + jobPriority, startTime);
    return start;
}

TrapezoidGap::operator std::string() const
{
    std::stringstream ret;
    ret << "Trapezoid " << Gap::operator std::string();
    return ret.str();
}

jobPrecision TrapezoidGap::insert(Job job)
{
    return jobPrecision();
}
