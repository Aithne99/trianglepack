import math
from operator import attrgetter
from typing import List
import cplex


class job:
    def __init__(self, p):
        self.priority = p
        self.gap = p
        self.start = 0
        self.wiggle = 0
        self.child = None


def check_binary_tree(sizes: List[int]):
    for idx, s in enumerate(sizes[1:]):
        i = idx + 1
        if s < sizes[math.ceil(i/2) - 1] / 2:
            return False
    return True


def greedy_base(maxjob: job, curjob: job, jobs: List[job]):
    curjob.start = maxjob.start + curjob.priority
    if 2 * curjob.priority > maxjob.gap:
        deltatime = 2 * curjob.priority - maxjob.gap
        if maxjob.child and maxjob.child.wiggle:
            if deltatime >= maxjob.child.wiggle:
                maxjob.child.start += maxjob.child.wiggle
                deltatime -= maxjob.child.wiggle
                maxjob.child.wiggle = 0
            else:
                maxjob.child.start += deltatime
                maxjob.child.wiggle -= deltatime
                deltatime = 0
        for jitr in jobs:
            if jitr.start >= curjob.start and jitr != curjob:
                jitr.start += deltatime
    potentialgap = maxjob.gap - curjob.gap
    if potentialgap > curjob.gap:
        curjob.wiggle = potentialgap - curjob.gap
        curjob.gap = potentialgap
    maxjob.gap = curjob.priority
    maxjob.child = curjob



def greedy(jobs: List[job]):
    i = 1
    while i < len(jobs):
        maxjob = max(jobs[0:i], key=attrgetter('gap'), default=jobs[0])
        greedy_base(maxjob, jobs[i], jobs)
        i += 1


def if_it_fits_i_sits(jobs: List[job]):
    i = 1
    # key=lambda j: j.gap + (j.child and j.child.wiggle or 0)
    while i < len(jobs):
        fits = [j for j in jobs[0:i] if j.gap + (j.child and j.child.wiggle or 0) >= 2 * jobs[i].priority]
        if fits:
            maxjob = min(fits, key=attrgetter('gap'))
        else:
            maxjob = max(jobs[0:i], key=attrgetter('gap'), default=jobs[0])
        greedy_base(maxjob, jobs[i], jobs)
        i += 1


def build_cplex_model(jobs: List[job]):
    #ezt kisebbre nem tudom allitani mert infeas lesz...
    N = jobs[0].priority * 10
    cpx = cplex.Cplex()
    cpx.set_problem_type(cpx.problem_type.MILP)
    cpx.objective.set_sense(cpx.objective.sense.minimize)
    cpx.variables.add(obj=[1], names=["z"])
    for idx, jitr in enumerate(jobs):
        cpx.variables.add(lb=[0], names=[f"{idx}:{jitr.priority}"])
        cpx.linear_constraints.add(lin_expr=[cplex.SparsePair(ind=["z", f"{idx}:{jitr.priority}"], val=[1, -1])], senses=['G'], rhs=[jitr.priority], names=[f"zctr{idx}"], range_values=[0])
    for idx, j1 in enumerate(jobs):
        for idx2, j2 in enumerate(jobs[idx + 1:]):
            cpx.variables.add(types=[cpx.variables.type.binary], names=[f"I{idx}:{idx2 + idx + 1}"])
            names = [f"{idx}:{j1.priority}", f"{idx2 + idx + 1}:{j2.priority}", f"I{idx}:{idx2 + idx + 1}"]
            cpx.linear_constraints.add(lin_expr=[cplex.SparsePair(ind=names, val=[1, -1, N])], senses=['G'], rhs=[min(j1.priority, j2.priority)], names=[f"timepos{idx}:{idx2 + idx + 1}"])
            cpx.linear_constraints.add(lin_expr=[cplex.SparsePair(ind=names, val=[-1, 1, -N])], senses=['G'], rhs=[min(j1.priority, j2.priority) - N], names=[f"timeneg{idx}:{idx2 + idx + 1}"])
    cpx.solve()
    print(cpx.solution.get_values()[1:len(jobs) + 1])
    print(cpx.solution.get_objective_value())


if __name__ == '__main__':
    #jobsizes = [89, 83, 79, 73, 71, 67, 61, 59, 53, 47, 43, 41, 37, 31, 29, 23, 19, 17, 13, 11, 7, 5, 3, 2]
    #jobsizes = [8, 2.9, 2, 2, 8, 2.9, 2, 2, 8, 2.9, 2, 2, 8, 2.9, 2, 2, 8, 2.9, 2, 2, 8, 2.9, 2, 2]
    jobsizes = [16, 6, 5, 4]
    jobsizes.sort(reverse=True)
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
    if check_binary_tree(jobsizes):
        print("Greedy is optimal")
    else:
        build_cplex_model(joblist)


