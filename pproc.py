import numpy as np
import scipy.signal as signal

#TODO: voice/unvoiced detection and silence detector/threshold

def generate_filter(filterType, fs=12000):
    '''
    args: 
        str filterType -> {'900Hz FIRLS_LPF'}
        int fs -> sampling rate (defaults to 12000 for python)
    return:
        myFilter -> np.array of the filter coefficients
    '''
    #if filterType == ...
    # TODO: add different filter types
    #   Try something like signal.firfilt or signal.remez
    # TODO: test numtaps, possibly have it as arg into generate filter
    '''
    numtaps = 53
    bands = [0, 100, 800, 900, 1000, fs/2]
    gain = [1, 1, 1, 0, 0, 0]
    myFilter = signal.firls(numtaps, bands, gain, fs=fs)
    '''
    cutoff = 900
    trans_width = 100  # Width of transition from pass band to stop band, Hz
    numtaps = 300      # Size of the FIR filter.
    myFilter = signal.remez(numtaps, [0, cutoff, cutoff + trans_width, 0.5*fs], [1, 0], Hz=fs)
    
    return myFilter
    
def filter_audio(sound, mFilter):
    '''
    Apply the given filter to the given audio file
    args:
        sound -> np.array from parselmouth sound
        filter -> np.array from generate_filter
    '''
    # TODO: test application methods? 
    #       Implement without buit-ins
    #           Implement with circular convolution
    #       Wil this work with all filter types?
    filteredSound = signal.lfilter(mFilter, 1, sound)
    #filteredSound = np.zeros(sound.shape)
    #print(sound.shape)
    #print(mFilter.shape)
    #filteredSound = np.convolve(sound, mFilter, mode='same')

    return filteredSound

def simple_peaks_helper(sound):
    '''
    Find each local min/max
    return:
        [m1, m4]
            m1 -> np.array of local maxes
            m4 -> np.array of local mins
    '''
    # TODO: No built-ins
    m1 = signal.argrelextrema(sound, np.greater)
    m4 = signal.argrelextrema(sound, np.less)
    return [m1, m4]

def peak_valley_helper(m1, m4):
    '''
    Find distance between mins and maxes
    return:
        [m2, m5]
            m2 -> np.array of abs(max)-abs(prev_min)
            m5 -> np.array of abs(prev_max) - abs(min)
    '''
    m2 = np.zeros(m1.size)
    m5 = np.zeros(m4.size)
    m1NonZeroIdx = np.array(np.nonzero(m1)).flatten()
    m4NonZeroIdx = np.array(np.nonzero(m4)).flatten()
    # TODO: Test all cases
    # First peak before first valley
    if(m1NonZeroIdx[0] < m4NonZeroIdx[0]):
        m2[m1NonZeroIdx[0]] = m1[m1NonZeroIdx[0]]
        m5[m4NonZeroIdx[0]] = np.abs(m1[m1NonZeroIdx[0]]) - np.abs(m4[m4NonZeroIdx[0]])
        for i in range(1, len(m1NonZeroIdx)):
            m2[m1NonZeroIdx[i]] = np.abs(m1[m1NonZeroIdx[i]]) - np.abs(m4[m4NonZeroIdx[i-1]])
            # Check bounds
            if i < len(m4NonZeroIdx):
                m5[m4NonZeroIdx[i]] = np.abs(m1[m1NonZeroIdx[i]]) - np.abs(m4[m4NonZeroIdx[i]])
    # First peak after first valley
    else:
        m5[m4NonZeroIdx[0]] = m4[m4NonZeroIdx[0]]
        for i in range(len(m1NonZeroIdx)):
            m2[m1NonZeroIdx[i]] = np.abs(m1[m1NonZeroIdx[i]]) - np.abs(m4[m4NonZeroIdx[i]])
            # Check bounds
            if i + 1 < len(m4NonZeroIdx):
                m5[m4NonZeroIdx[i+1]] = np.abs(m1[m1NonZeroIdx[i]]) - np.abs(m4[m4NonZeroIdx[i+1]]) 
    
    return [m2, m5]

def adj_peak_helper(m1, m4):
    '''
    Find the difference between adjacent peaks
    NOTE: The heights start at first peak and look ahead
    NOTE: Uncertain what should be done in case of 1 peak
        I guess in a real scenario it wouldn't matter
    return:
        [m3, m6]
            m2 -> np.array of peak to peak differences
            m5 -> np.array of valley to valley differences
    '''
    m3 = np.zeros(m1.size)
    m6 = np.zeros(m4.size)
    m1NonZeroIdx = np.array(np.nonzero(m1)).flatten()
    m4NonZeroIdx = np.array(np.nonzero(m4)).flatten()
    startLen = len(m1NonZeroIdx) - len(m1NonZeroIdx)%2
    if len(m1NonZeroIdx) > 1:
        for i in range(0, startLen, 2):
            if m1NonZeroIdx[i+1] > m1NonZeroIdx[i]:
                m3[m1NonZeroIdx[i]] = np.abs(m1[m1NonZeroIdx[i+1]] - m1[m1NonZeroIdx[i]])
    startLen = len(m4NonZeroIdx) - len(m4NonZeroIdx)%2
    if len(m4NonZeroIdx) > 1:
        for i in range(0, startLen, 2):
            if m4NonZeroIdx[i+1] > m4NonZeroIdx[i]:
                m6[m4NonZeroIdx[i]] = np.abs(m6[m4NonZeroIdx[i+1]] - m6[m4NonZeroIdx[i]])

    return [m3, m6]


def find_peaks(sound):
    '''
    Get the pulse heights [m1...m6]
    broken down for each pair [m1,m4], [m2,m5], [m3,m6]
    '''
    m1 = np.zeros(sound.shape)
    m2 = np.zeros(sound.shape)
    m3 = np.zeros(sound.shape)
    m4 = np.zeros(sound.shape)
    m5 = np.zeros(sound.shape)
    m6 = np.zeros(sound.shape)

    m1Idx, m4Idx = simple_peaks_helper(sound)
    m1[m1Idx] = sound[m1Idx]
    m4[m4Idx] = sound[m4Idx]
    m2, m5 = peak_valley_helper(m1, m4)
    # TODO: is m6 working?
    m3, m6 = adj_peak_helper(m1, m4)
    # Convert everything to positive impulses
    m1 = np.abs(np.array(m1))
    m2 = np.abs(np.array(m2))
    m3 = np.abs(np.array(m3))
    m4 = np.abs(np.array(m4))
    m5 = np.abs(np.array(m5))
    m6 = np.abs(np.array(m6))
    return[m1, m2, m3, m4, m5, m6]

def peak_rundown(m, fs=12000):
    '''
    Elementry period detection using a peak rundown circuit
    TODO: Initial condition for Pav?
    TODO: Limit Pav to be between 4 ms and 10 ms (???)
    NOTE: We must convert samples to ms
        y (samples) = fs/1000 * x(ms)
        Pav -> in ms
        tau -> in ms
        beta -> in samples
        i -> in samples
    NOTE: Can probably be done in parallel for each m
    input:
        m -> np.array of pulse train
    return:
        Pav -> smoothed period estimate
    '''
    msToSamples = fs/1000 # Multiply to get samples, divide to get ms
    Pav_prev = 0
    Pnew = 0
    # Find first peak
    if(len(np.nonzero(m)[0]) !=0 ):
        start = np.nonzero(m)[0][0]
    else:
        start = len(m)
    lastPeak = start
    Pav = 4
    beta = (Pav/.695)*msToSamples

    # Start blanking
    tau = .4*Pav
    for i in range(start, len(m)):
        if tau <= 0:
            # Start exponential decay, if peak exceeds exponential decay update period and restart blanking 
            if m[i] > m[lastPeak]*np.exp(-beta*i):
                Pnew = (i - lastPeak)/msToSamples
                temp = Pav
                Pav = (Pav_prev + Pnew)/2
                Pav_prev = temp
                lastPeak = i
                tau = .4 * Pav
                beta = (Pav/.695)*msToSamples
        else:
            tau -= 1*msToSamples

    return Pav

def create_pitch_matrix(curPPE, prevPPE_1, prevPPE_2):
    '''
    Generate the pitch period estimation matrix from the 3 most recent estimations
    TODO:   idk if the order of n -> n-1 -> n-2 is correct for inputting into matrix
            I also don't know if this is supposed to be from the ppes or from full iterations of the algorithm 
    NOTE: This can be optimized
    NOTE: Only elements in row one are candidates for estimated pitch (ie. my assumption above should be correct)
    return:
        ppeMatrix -> 6x6 np.ndarray of pitch period estimations (in ms)
    '''
    peMatrix = np.zeros((6,6))
    # Most recent PPEs
    for i in range(6):
        peMatrix[0][i] = curPPE[i]
    for i in range(6):
        peMatrix[1][i] = prevPPE_1[i]
    for i in range(6):
        peMatrix[2][i] = prevPPE_2[i]
    # Sum of first and second rows
    for i in range(6):
        peMatrix[3][i] = peMatrix[0][i] + peMatrix[1][i]
    # Sum of second and third rows
    for i in range(6):
        peMatrix[4][i] = peMatrix[1][i] + peMatrix[2][i]
    # Sum of the first three rows
    for i in range(6):
        peMatrix[5][i] = peMatrix[0][i] + peMatrix[1][i] + peMatrix[2][i]
    
    return peMatrix

def findThreshold(ppe, bias):
    '''
    Find the coincidence window length for a given ppe and bias
    NOTE: There has got to be a more prettier way of doing this...
    return:
        threshold -> int of threshold length in ms
            inf if the ppe or bias are invalid
    '''
    # Invalid ppe checks
    if ppe < 1.6:
        threshold = np.inf
    elif ppe >= 25.5:
        threshold = np.inf
    # Find given valid ppe and bias
    elif 1.6 <= ppe < 3.1:
        if bias == 1:
            threshold = 100
        elif bias == 2:
            threshold = 200
        elif bias == 5:
            threshold = 300
        elif bias == 7:
            threshold = 400
        else:
            threshold = np.inf
    elif 3.1 <= ppe < 6.3:
        if bias == 1:
            threshold = 200
        elif bias == 2:
            threshold = 400
        elif bias == 5:
            threshold = 600
        elif bias == 7:
            threshold = 800
        else:
            threshold = np.inf
    elif 6.3 <= ppe < 12.7:
        if bias == 1:
            threshold = 400
        elif bias == 2:
            threshold = 800
        elif bias == 5:
            threshold = 1200
        elif bias == 7:
            threshold = 1600
        else:
            threshold = np.inf
    else: # 12.7 <= ppe < 25.5
        if bias == 1:
            threshold = 800
        elif bias == 2:
            threshold = 1600
        elif bias == 5:
            threshold = 2400
        elif bias == 7:
            threshold = 3200
        else:
            threshold = np.inf

    return threshold

def calculate_coincidence(peMatrix, bias):
    '''
    Calculate coicidences of elements in the first row to all 35 other elements.
        We estimate coincidence between two pitch periods by finding the absolute difference between them and 
        if it is less then the coincidence window length we say it coincides.
    NOTE: Can be done in parallel for each bias
    NOTE: probably a better way to check thresholds and make threshold matrix (possibly custom class)
    return:
        [winnerIdx, winnerVal]
            winnerIdx -> index into peMatrix[0] for ppe with most coincidences
            winnerVal -> number of coincidences for winnerIdx
    '''
    coincidences = np.zeros(6)

    # Check ppe_i to all elements
    for i in range(6):
        # Find the coincidence window length (threshold) for ppe_i
        threshold = findThreshold(peMatrix[0][i], bias)
        for x in range(6):
            for y in range(6):
                # NOTE: we are checking when peMatrix[0][i] == peMatrix[y][x] but that shouldn't matter
                if np.abs(peMatrix[0][i] - peMatrix[y][x]) < threshold:
                    coincidences[i] += 1

    winnerIdx = np.argmax(coincidences)
    winnerVal = coincidences[winnerIdx]
    return (winnerIdx, winnerVal)

def calculate_ppe_winner(peMatrix):
    '''
    Find the current ppe with the most coincidences in the peMatrix
    TODO: Can be done in parallel for each bias
    return:
        winner -> ppe (in ms) that best estimates the current pitch period
    '''
    winnerArr = np.zeros((6,2))
    biases = [1,2,5,7]
    for i in range(4):
        winnerArr[i] = calculate_coincidence(peMatrix, biases[i])
    winner = peMatrix[0][np.argmax(winnerArr, axis=0)[1]]

    return winner

def pproc_calculate_pitch(sound, framesize=.043, fs=12000):
    '''
    Combine the entire pproc algorith into one easy to use function
    TODO: What to do with potential leftover frames
    NOTE: Assuming sound is mono
    return:
        estimates -> np.array of pitch estimates
    '''
    estimates = np.zeros(sound.size)

    # Filter the sound
    filt = generate_filter('default', fs=fs)
    filtSound = filter_audio(sound, filt)
    prevPPE_1 = np.zeros(6)
    prevPPE_2 = np.zeros(6)
    ppe = np.zeros(6)
    
    # Convert framesize to samples
    updateSize = round(fs * framesize)
    i = updateSize
    while i < len(sound):
        # Make peak measurements
        peaks = find_peaks(filtSound[i-updateSize:i])
        # Update previous PPEs
        prevPPE_2 = prevPPE_1
        prevPPE_1 = ppe
        # Get current PPE
        # TODO: Parallel?
        for j in range(6):
            ppe[j] = peak_rundown(peaks[j], fs=fs)
        # Create the ppe matrix
        peMatrix = create_pitch_matrix(ppe, prevPPE_1, prevPPE_2)
        # Find current best pitch estimate
        # NOTE: idk what idx into estimtes this is supposed to be?
        estimates[i-updateSize] = calculate_ppe_winner(peMatrix) / 1000 # Convert from ms to seconds
        i += updateSize

    return estimates
