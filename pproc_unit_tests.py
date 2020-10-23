import pproc
from matplotlib import pyplot as plt
import scipy.signal as signal
import numpy as np

def plot_spectrogram(title, w, fs):
    ff, tt, Sxx = signal.spectrogram(w, fs=fs)
    plt.pcolormesh(tt, ff, Sxx, cmap='gray_r', shading='gouraud')
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
    x = np.linspace(0,4*np.pi,fs*2)
    simple_test = (np.sin(np.pi*x)*np.sin(np.pi/2*x)) + .3 #np.array([1,2,3,2,5,1])
    simpleMs = pproc.find_peaks(simple_test)
    plt.figure()
    plt.plot(x,simple_test, '-k')
    #print("Pair 1:")
    #print(simpleMs[0][np.nonzero(simpleMs[0])])
    #print(simpleMs[3][np.nonzero(simpleMs[3])])
    plt.plot(x[np.nonzero(simpleMs[0])], simpleMs[0][np.nonzero(simpleMs[0])], 'ro')
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
    plt.show()
    return


test_generate_filters()
test_filter_audio()
test_find_peaks()