
from matplotlib import pyplot as plt
import sys

data = open("./../output/tick_creep_count.txt").read().split("\n")
if data[-1] == "": data = data[:-1]
data = map(int, data)

X, Y = zip(*enumerate(data))
plt.plot(X, Y, "ro")
plt.show()