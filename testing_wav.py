from scipy.io.wavfile import read

rate,data=read("Synthesized Speech/6_C_247.wav")
print(rate,(data[:130]))
