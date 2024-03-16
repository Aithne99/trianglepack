import math
from queue import PriorityQueue

import numpy as np


class compressed_input:
    def __init__(self):
        self.jobsizes = dict()
        self.cachesize = 0
        self.cacheheight = 0
        self.dirty = False

    def add_job(self, size, count):
        if size in self.jobsizes:
            self.jobsizes[size] += count
        else:
            self.jobsizes[size] = count
        self.dirty = True
    def get_idx(self, idx: int):
        if idx > self.get_size():
            return None
        sorted_keys = sorted(self.jobsizes.keys(), reverse=True)
        itr_idx = 0
        for key in sorted_keys[0:]:
            itr_idx += self.jobsizes[key]
            if itr_idx > idx:
                return key
    def get_size(self):
        if self.dirty:
            self.reinit_sizes()
        return self.cachesize
    def get_height(self):
        if self.dirty:
            self.reinit_sizes()
        return self.cacheheight
    def reinit_sizes(self):
        self.cacheheight = math.ceil(math.log(sum(self.jobsizes.values()), 2))
        self.cachesize = int(math.pow(2, self.cacheheight))
        self.dirty = False
    def placement_to_idx(self, place: int):
        if not place:
            return 0
        size = int(math.pow(2, self.get_height()))
        numlen = math.log(size, 2) + 1

        rowidx_searcher = 2
        rownum = 1

        while place % rowidx_searcher == 0:
            rowidx_searcher *= 2
            rownum += 1

        row_idx = self.get_height() - rownum
        inrow_idx = place // rowidx_searcher
        begin_idx = size // rowidx_searcher
        idx = begin_idx + inrow_idx
        return idx


def bintree_compressed(jobs: compressed_input):
    starttimes = PriorityQueue()
    # gap start, gap height, edge or corner gap (0: gap height is limited by upper corner 1: gap height is limited by diagonal edge), 2: both, ceiling start (when both)
    priority = jobs.get_idx(0)
    starttimes.put((0, priority, 1, 0))
    starttimes.put((priority, priority, 0, 0))
    info_unit = jobs.get_size() / 10000
    infocounter = 0
    makespan = priority
    start = 0
    for pack_idx in range(1, jobs.get_size()):
        infocounter += 1
        if infocounter > info_unit:
            print(f"{pack_idx} out of {jobs.get_size()} already packed for a makespan of {makespan}")
            infocounter = 0
        priority = jobs.get_idx(jobs.placement_to_idx(pack_idx))
        if priority is None:
            continue
        while True:
            testtime = starttimes.get()
            if priority < (testtime[1]):
                if (starttimes.empty()):
                    raise RuntimeError("asd")
                nexttime = starttimes.get()
                if testtime[2] == 2:
                    possiblestart = max(testtime[3] + priority, testtime[0])
                    start = possiblestart
                else:
                    start = testtime[0] + testtime[2] * priority
                if start + priority < nexttime[0]:
                    starttimes.put((start + priority, testtime[1], 2, testtime[3]))
                    starttimes.put(nexttime)
                else:
                    temptime = nexttime
                    mode = 2
                    if starttimes.empty():
                        ceilheight = priority
                        ceilstart = start
                        mode = 0
                    else:
                        # pop all the ones we've overgone
                        while True:
                            temptime = nexttime
                            if starttimes.empty():
                                temptime = nexttime
                                break
                            nexttime = starttimes.get()
                            if start + priority < nexttime[0] or starttimes.empty():
                                break
                        ceilheight = temptime[1]
                        ceilstart = temptime[3]
                        starttimes.put(nexttime)
                    starttimes.put((start + priority, ceilheight, mode, ceilstart))
                    makespan = max(makespan, start + priority)
                starttimes.put((start, priority, 1, start))
                break
            if starttimes.empty():
                start = testtime[0]
                starttimes.put((start, priority, 1, start))
                starttimes.put((start + priority, priority, 0, start))
                makespan = max(makespan, start + priority)
                break

    print(makespan)

'''
job size 104976 count 1
job size 52488 count 1
job size 17496 count 4
job size 5832 count 12
job size 2916 count 18
job size 972 count 72
job size 324 count 216
job size 162 count 324
job size 54 count 1296
job size 18 count 3888
job size 9 count 5832
job size 3 count 23328
job size 1 count 69984 
'''
input = compressed_input()
jobsizes = [104976, 52488, 17496, 5832, 2916, 972, 324, 162, 54, 18, 9, 3, 1]
jobcounts = [1, 1, 4, 12, 18, 72, 216, 324, 1296, 3888, 5832, 23328, 69984]
for size, idx in zip(jobsizes, jobcounts):
    input.add_job(size * jobsizes[0], idx)
    if size != jobsizes[0]:
        input.add_job(size, idx * jobsizes[0])

for key, val in input.jobsizes.items():
    print(f"{key} priority: {val} jobs")

bintree_compressed(input)

