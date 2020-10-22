import numpy as np
import scipy.signal as signal

def generate_filter(filterType, fs=12000):
    '''
    args: 
        str filterType -> {'900Hz FIRLS_LPF'}
        int fs -> sampling rate (defaults to 12000 for python)
    '''
    #if filterType == ...
    # TODO: add different filter types
    # TODO: test numtaps, possibly have it as arg into generate filter
    numtaps = 53
    bands = [0, 100, 800, 900, 1000, fs/2]
    gain = [1, 1, 1, 0, 0, 0]
    myFilter = signal.firls(numtaps, bands, gain, fs=fs)
    return myFilter
    
def filter_audio(filename, filter):
    '''
    Apply the given filter to the given audio file
    '''
    return
    