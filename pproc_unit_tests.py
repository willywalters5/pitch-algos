import pproc
from matplotlib import pyplot as plt
import scipy.signal as signal
import numpy as np
from pydub import AudioSegment
from pydub.playback import play

def test_filters(fs=12000):
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

test_filters()

play(AudioSegment.from_wav('PureTones/100Hz.wav'))