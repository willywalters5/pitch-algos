import numpy as np
from matplotlib import pyplot as plt

plt.figure()
# Note: length is dependent on sample rate
plt.plot(np.kaiser(round(48000*.042), 1.75))
plt.title("Numpy Kaiser, beta=1.75")
plt.xlabel("Samples")
plt.ylabel("Magnitdue")
plt.show()