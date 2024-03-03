import random


def generate_input(mode, length, param=0):
    if mode == "random":
        return _generate_random_input(length)
    elif mode == "fibonacci":
        return _generate_fibonacci_input(length)
    elif mode == "3dtrue":
        return _generate_true_matching_input(length)
    elif mode == "3dfalse":
        return _generate_false_matching_input(length)
    elif mode == "pow3":
        return _generate_pow3_input(length)


# length is just the length of the input
def _generate_random_input(length):
    return [random.randint(1, length * 2) for p in range(0, length)]


# length is the number of fib sequence items used
def _generate_fibonacci_input(length):
    pass


# length is the number of elements in the vector
def _generate_true_matching_input(length):
    pass


def _generate_false_matching_input(length):
    pass


# length is the highest power used
def _generate_pow3_input(length):
    if length < 1:
        return [3, 3]
    arr = [3 ** length, 3 ** length, 3 ** (length - 1), 3 ** (length - 1), 3 ** (length - 1), 3 ** (length - 1)]
    if length < 2:
        print("nemár")
        return arr
    length = length - 2
    pow2 = 4
    while length >= 0:
        for i in range(0, pow2 + 2 * pow2):
            arr.append(3 ** length)
        length = length - 1
        pow2 = 4 * pow2
    return arr