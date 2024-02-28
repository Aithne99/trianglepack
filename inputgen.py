import random


def generate_input(length, mode):
    if mode == "random":
        return _generate_random_input(length)
    elif mode == "fibonacci":
        return _generate_fibonacci_input(length)
    elif mode == "3dtrue":
        return _generate_true_matching_input(length)
    elif mode == "3dfalse":
        return _generate_false_matching_input(length)


# length is just the length of the input
def _generate_random_input(length):
    return random.sample(range(1, length + 1), length)


# length is the number of fib sequence items used
def _generate_fibonacci_input(length):
    pass


# length is the number of elements in the vector
def _generate_true_matching_input(length):
    pass


def _generate_false_matching_input(length):
    pass
