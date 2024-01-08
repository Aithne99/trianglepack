import math
from operator import attrgetter
from typing import List


class job:
    def __init__(self, p):
        self.priority = p
        self.gap = p
        self.start = 0


def check_binary_tree(sizes: List[int]):
    for idx, s in enumerate(sizes[1:]):
        i = idx + 1
        if s < sizes[math.ceil(i/2) - 1] / 2:
            return False
    return True


def greedy(jobs: List[job]):
    i = 1
    while i < len(jobs):
        maxjob = max(jobs[0:i], key=attrgetter('gap'), default=jobs[0])
        maxidx = jobs.index(maxjob)
        jobs[i].start = jobs[maxidx].start + jobs[i].priority
        if 2 * jobs[i].priority > maxjob.gap:
            for jitr in jobs:
                if jitr.start > jobs[i].start:
                    jitr.start += 2 * jobs[i].priority - maxjob.gap
        jobs[i].gap = max(jobs[i].gap, maxjob.gap - jobs[i].gap)
        maxjob.gap = jobs[i].priority
        i += 1

def if_it_fits_i_sits(jobs: List[job]):
    i = 1
    while i < len(jobs):
        fits = [j for j in jobs[0:i] if j.gap >= 2 * jobs[i].priority]
        if fits:
            maxjob = min(fits, key=attrgetter('gap'))
        else:
            maxjob = max(jobs[0:i], key=attrgetter('gap'), default=jobs[0])
        maxidx = jobs.index(maxjob)
        jobs[i].start = jobs[maxidx].start + jobs[i].priority
        if 2 * jobs[i].priority > maxjob.gap:
            for jitr in jobs:
                if jitr.start > jobs[i].start:
                    jitr.start += 2 * jobs[i].priority - maxjob.gap
        jobs[i].gap = max(jobs[i].gap, maxjob.gap - jobs[i].gap)
        maxjob.gap = jobs[i].priority
        i += 1


if __name__ == '__main__':
    jobsizes = [97, 89, 83, 79, 73, 71, 67, 61, 59, 53, 47, 43, 41, 37, 31, 29, 23, 19, 17, 13, 11, 7, 5, 3, 2]
    #jobsizes = [3, 4, 5, 6]
    jobsizes.sort(reverse=True)
    if check_binary_tree(jobsizes):
        print("Greedy is optimal")
    joblist = [job(i) for i in jobsizes]
    joblist2 = [job(i) for i in jobsizes]
    greedy(joblist)
    for j in joblist:
        print(j.priority, j.start)
    worst = max(joblist, key=lambda j: j.start + j.priority)
    print(f"greedy makespan {worst.start + worst.priority}")

    if_it_fits_i_sits(joblist2)
    for j in joblist2:
        print(j.priority, j.start)
    worst = max(joblist2, key=lambda j: j.start + j.priority)
    print(f"tight fitting makespan {worst.start + worst.priority}")


# See PyCharm help at https://www.jetbrains.com/help/pycharm/
