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


bool InfiniteGap::insert(Job job)
{
    return true;
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

jobPrecision iterSquareBlowup(jobPrecision base, jobPrecision count)
{
    jobPrecision result = 1;
    while (count > 0)
    {
        result *= base;
        base *= base;
        --count;
    }
    return result;
}
