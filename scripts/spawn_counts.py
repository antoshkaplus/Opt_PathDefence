
from matplotlib import pyplot as plt
import sys

data = open("./../output/spawn_ticks.txt").read().split("\n")
if data[-1] == "": data = data[:-1]
data = map(int, data)
N = 2000
times = N*[0]
for d in data:
    times[d] += 1

X, Y = zip(*enumerate(times))
plt.plot(X, Y, "ro-")
plt.show()