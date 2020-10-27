import pproc
import pitch_parselmouth
import parselmouth
from matplotlib import pyplot as plt
import scipy.signal as signal
import numpy as np

def plot_spectrogram(title, w, fs=12000):
    ff, tt, Sxx = signal.spectrogram(w, fs=fs)
    plt.figure()
    plt.pcolormesh(tt, ff, Sxx, cmap='viridis')#, shading='gouraud')
    plt.title(title)
    plt.xlabel('t (sec)')
    plt.ylabel('Frequency (Hz)')
    plt.grid()

def test_generate_filters(fs=12000):
    '''
    Test to make sure the filter appears as desired by viewing a graph :)
    '''
    b = pproc.generate_filter('default')
    w, h = signal.freqz(b)
    plt.figure()
    plt.title('Filter Magnitude Response')
    plt.xlabel('Frequency (Hz)')
    plt.ylabel('dB')
    plt.plot((w/np.pi)*(fs/2), 20* np.log10(abs(h)))
    plt.show()
    return

def test_filter_audio(fs=12000):
    '''
    Test to make sure the filter attenuates as desired by plotting spectrogram of chirp
    '''
    b = pproc.generate_filter('default')
    t = np.arange(0, int(5*fs)) / fs
    chirp = signal.chirp(t, 50, 5, 1500, method='linear')
    #plt.figure()
    #plt.plot(t,chirp)
    #plt.show()
    filteredSound = pproc.filter_audio(chirp, b)
    plot_spectrogram('Spectrogram of filtered chirp [50,1500]', filteredSound, fs)
    plt.show()
    return

def test_find_peaks(fs=12000):
    # TODO: More logical test array, better test than printing
    # TODO: Add key for different colors
    #   Maybe make seperate subplots for each color to make it more clear what is what
    
    sound = parselmouth.Sound("PureTones/100Hz.wav")
    simple_test = np.array(sound.convert_to_mono()).flatten()[0:3000]
    x = sound.xs()[0:3000]
    '''
    x = np.arange(0, int(1*fs)) / fs#np.linspace(0,4*np.pi,fs*2)
    simple_test = np.sin(2*np.pi*100*x)#(np.sin(np.pi*x)*np.sin(np.pi/2*x)) + .3 #np.array([1,2,3,2,5,1])
    '''
    simpleMs = pproc.find_peaks(simple_test)
    plt.figure()
    plt.plot(x,simple_test, '-k')
    #print("Pair 1:")
    #print(simpleMs[0][np.nonzero(simpleMs[0])])
    #print(simpleMs[3][np.nonzero(simpleMs[3])])
    plt.plot(x[np.nonzero(simpleMs[0])], simpleMs[0][np.nonzero(simpleMs[0])], 'ro')
    print(np.nonzero(simpleMs[0]))
    print(x[np.nonzero(simpleMs[0])])
    '''
    plt.plot(x[np.nonzero(simpleMs[3])], simpleMs[3][np.nonzero(simpleMs[3])], 'r.')
    #print(simpleMs[0].shape)

    #print("Pair 2:")
    plt.plot(x[np.nonzero(simpleMs[1])], simpleMs[1][np.nonzero(simpleMs[1])], 'bo')
    plt.plot(x[np.nonzero(simpleMs[4])], simpleMs[4][np.nonzero(simpleMs[4])], 'b.')
    #print(simpleMs[1][np.nonzero(simpleMs[1])])
    #print(simpleMs[4][np.nonzero(simpleMs[4])])
    #print("Pair 3:")
    plt.plot(x[np.nonzero(simpleMs[2])], simpleMs[2][np.nonzero(simpleMs[2])], 'go')
    plt.plot(x[np.nonzero(simpleMs[5])], simpleMs[5][np.nonzero(simpleMs[5])], 'g.')
    #print(simpleMs[2][np.nonzero(simpleMs[2])])
    #print(simpleMs[5][np.nonzero(simpleMs[5])])
    '''
    plt.show()
    return

def test_peak_rundown_helper(m, t, fs=12000):
    '''
    Elementry period detection using a peak rundown circuit
    TODO: Initial condition for Pav?
    TODO: Limit Pav to be between 4 ms and 10 ms (or somethin else to match our sample rate)
    NOTE: We must convert samples to ms
        y (samples) = fs/1000 * x(ms)
        Pav -> in ms
        tau -> in ms
        beta -> in samples
        i -> in samples
    input:
        m -> np.array of pulse train
    return:
        Pav -> smoothed period estimate
    '''
    m = m[1:len(m)]
    plotMe = np.zeros(len(m)+1)
    Pav_prev = 0
    Pnew = 0
    # Find first peak
    if(len(np.nonzero(m)[0]) !=0):
        start = np.array(np.nonzero(m)).flatten()[0] # np.nonzero(m)[0][0]
    else:
        start = len(m)
    lastPeak = start
    Pav = 4/1000
    beta = (Pav/.695)#*msToSamples

    # Start blanking
    tau = .4*Pav
    for i in range(start, len(m)):
        #print(tau)
        if tau <= 0:
            # Start exponential decay, if peak exceeds exponential decay update period and restart blanking
            plotMe[i] = m[lastPeak]*np.exp(-beta*(t[i] - t[lastPeak]))
            if m[i] > m[lastPeak]*np.exp(-beta*(t[i] - t[lastPeak])) and m[i] > 0:
                Pnew = t[i] - t[lastPeak]
                #print("i: {} FreqNew: {}".format(i, 1/Pnew))
                temp = Pav
                Pav = (Pav_prev + Pnew)/2
                Pav_prev = temp
                lastPeak = i
                tau = .4*Pav#min(max(Pav, 4), 10)
                beta = (Pav/.695)#*msToSamples#(min(max(Pav, 4), 10)/.695)*msToSamples
        else:
            tau -= 1/fs
            plotMe[i] = m[lastPeak]

    return plotMe

def test_peak_rundown(t, m, fs=12000, xlim=None):
    plt.figure()
    plt.plot(t[np.nonzero(m)], m[np.nonzero(m)], 'ko')
    if xlim != None:
        plt.xlim(xlim)
    plt.xlabel("Time (sec)")
    plt.ylabel("Magnitdue")
    plt.plot(t, test_peak_rundown_helper(m, t), '-b')
    plt.show()
    return

def test_all(filename, fs=12000, frameSize=None):
    # NOTE: Framesize in sec
    sound = parselmouth.Sound(filename)
    if frameSize == None:
        soundMono = np.array(sound.convert_to_mono()).flatten()
        t = sound.xs()
    else:
        #soundMono = np.array(sound.convert_to_mono()).flatten()[0:round(frameSize*fs)]
        soundMono = np.array(sound.convert_to_mono()).flatten()[round(frameSize*fs):2*round(frameSize*fs)]
        t = sound.xs()[0:round(frameSize*fs)]
        #t = sound.xs()[round(frameSize*fs):2*round(frameSize*fs)]

    plt.figure(num=1)
    plt.plot(t, soundMono)
    #plt.xlim([0, .25])
    plt.xlabel("Time (sec)")
    plt.ylabel("Amplitude")
    plt.title("Input Audio")
    plt.show()
    
    b = pproc.generate_filter('default')
    filtSound = soundMono#pproc.filter_audio(soundMono, b)
    if frameSize == None:
        plt.figure()
        plot_spectrogram("Filtered Audio Spectrogram", filtSound)
        plt.ylim([0,1200])
        plt.show()

    else:
        plt.figure(num=2, clear=True)
        plt.plot(t, filtSound)
        plt.xlabel("Time (sec)")
        plt.ylabel("Amplitude")
        plt.title("Filtered Input Audio")
        plt.show()
    
    peaks = pproc.find_peaks(filtSound)
    #peaks = pproc.find_peaks(soundMono)
    
    #updateSize = round(fs * frameSize)
    Pav_1 = pproc.peak_rundown(peaks[0], t)
    #Pav_2 = pproc.peak_rundown(peaks[1])
    #Pav_3 = pproc.peak_rundown(peaks[2])
    #Pav_4 = pproc.peak_rundown(peaks[3])
    #Pav_5 = pproc.peak_rundown(peaks[4])
    #Pav_6 = pproc.peak_rundown(peaks[5])
    print("Estimate 1: " + str(1/Pav_1))
    #print("Estimate 2: " + str(1/Pav_2))
    #print("Estimate 3: " + str(1/Pav_3))
    #print("Estimate 4: " + str(1/Pav_4))
    #print("Estimate 5: " + str(1/Pav_5))
    #print("Estimate 6: " + str(1/Pav_6))
    
    test_peak_rundown(t, peaks[0])
    #test_peak_rundown(t, peaks[1])
    #test_peak_rundown(t, peaks[2])

    return

def test_pproc_full(filename, framesize=.043, fs=12000):
    sound = parselmouth.Sound(filename)
    soundMono = np.array(sound.convert_to_mono()).flatten()
    #pitches = pproc.pproc_calculate_pitch(soundMono[0:3000], sound.xs()[0:3000], framesize=framesize, fs=fs)
    pitches = pproc.pproc_calculate_pitch(soundMono, sound.xs(), framesize=framesize, fs=fs)
    #print(len(np.nonzero(pitches)))
    t = np.array(np.nonzero(pitches)).flatten()
    print(1/pitches[t])
    pitch_parselmouth.compute_pitch_praat(filename, 1/pitches[t], computed_pitch_xs=t/fs)
    return

def test_gen_sin(freq, length, silenceLength=0, noiseRat=0, framesize=.043, fs=12000):
    '''
    Test with numpy sine wave
    freq -> Hz
    length -> sec
    '''
    t = np.arange(0, int(length*fs)) / fs
    tSilence = np.arange(t[-1], t[-1]+int(silenceLength*fs)) / fs
    tFull = np.append(t, tSilence)
    sound = np.sin(2*np.pi*freq*t) 
    sound = np.append(sound, np.zeros(len(tSilence))) + noiseRat*np.random.normal(0,1,len(tFull))
    pitches = pproc.pproc_calculate_pitch(sound, tFull, framesize=framesize, fs=fs)
    tnz = np.array(np.nonzero(pitches)).flatten()
    calcFreqAvg = np.sum(1/pitches[tnz])/len(tnz)
    print(1/pitches[tnz])
    print("Calculated Frquency Average: {}\n Actual Frequency: {}\n Difference: {}".format(calcFreqAvg, freq, freq-calcFreqAvg))

    filt = pproc.generate_filter('default')
    filtSound = pproc.filter_audio(sound, filt)

    plt.figure()
    plt.plot(tFull[0:3000], filtSound[0:3000])
    plt.title("Original Signal")
    plt.show()
    
    plot_spectrogram("Filtered Sine", filtSound)
    plt.plot(tFull[tnz], 1/pitches[tnz], 'ro')
    plt.xlim([0,length+silenceLength])
    plt.ylim([0,1200])
    plt.show()
    return

#test_generate_filters()
#test_filter_audio()
#test_find_peaks()
#test_all("PureTones/100Hz.wav", frameSize=.2)
#test_all("PureTones/100Hz.wav")
#test_pproc_full("PureTones/100Hz.wav", framesize=.05)
for i in range(1,9):
    test_pproc_full("Recordings/{}_AF1.wav".format(i), framesize=.043)

#test_gen_sin(200, 1.5, silenceLength=.25, noiseRat=0.05)




