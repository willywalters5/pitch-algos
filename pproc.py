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
    # NOTE: A better filter might be good
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
    # NOTE: Changed to xxx_equal to get rid of errors in unit test
    m1 = signal.argrelextrema(sound, np.greater_equal)
    m4 = signal.argrelextrema(sound, np.less_equal)
    '''
    if(len(m1) > 1):
        m1 = m1[2:len(m1)]
    if(len(m4) > 1):
        m4 = m4[2:len(m4)]
    '''
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
    if(len(m4NonZeroIdx) > 0 and m1NonZeroIdx[0] < m4NonZeroIdx[0]):
        m2[m1NonZeroIdx[0]] = m1[m1NonZeroIdx[0]]
        m5[m4NonZeroIdx[0]] = np.abs(m1[m1NonZeroIdx[0]]) - np.abs(m4[m4NonZeroIdx[0]])
        for i in range(1, len(m1NonZeroIdx)):
            if i-1 < len(m4NonZeroIdx):
                m2[m1NonZeroIdx[i]] = np.abs(m1[m1NonZeroIdx[i]]) - np.abs(m4[m4NonZeroIdx[i-1]])
            # Check bounds
            if i < len(m4NonZeroIdx):
                m5[m4NonZeroIdx[i]] = np.abs(m1[m1NonZeroIdx[i]]) - np.abs(m4[m4NonZeroIdx[i]])
    # First peak after first valley
    elif(len(m1NonZeroIdx) > 0):
        m5[m4NonZeroIdx[0]] = m4[m4NonZeroIdx[0]]
        for i in range(len(m1NonZeroIdx)):
            if i < len(m4NonZeroIdx):
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

def peak_rundown(m, t, fs=12000):
    '''
    Elementry period detection using a peak rundown circuit
    TODO: Initial condition for Pav?
    TODO: Limit Pav to be between 4 ms and 10 ms (???)
    NOTE: We must convert samples to ms
        y (samples) = fs/1000 * x(ms)
        Pav -> in s
        tau -> in s
        beta -> in s
        i -> in samples
    NOTE: Can probably be done in parallel for each m
    input:
        m -> np.array of pulse train
    return:
        Pav -> smoothed period estimate
        NOTE: I think paper uses everything in ms?
    '''
    #m = m[2:len(m)]
    Pav_prev = 0
    Pnew = 0
    # Find first peak (excluding first sample)
    if(len(np.array(np.nonzero(m)).flatten()) != 0):
        start = np.array(np.nonzero(m)).flatten()[0] # np.nonzero(m)[0][0]
        #if(start == 0):
        #    start = np.array(np.nonzero(m)).flatten()[1]
    else:
        start = len(m)
    lastPeak = start
    Pav = 0#4/1000
    beta = ((4/1000)/.695)
    #print(start)
    #print(len(m))
    #print(np.array(np.nonzero(m)).flatten())
    # Start blanking
    tau = .4*(4/1000)
    for i in range(start, len(m)):
        if tau <= 0:
            # Start exponential decay, if peak exceeds exponential decay update period and restart blanking
            if m[i] > m[lastPeak]*np.exp(-beta*(t[i]-t[lastPeak])) and m[i] > 0 and i != 0:
                Pnew = t[i] - t[lastPeak]#(i - lastPeak)/fs
                #print("i: {} FreqNew: {}".format(i, 1/Pnew))
                temp = Pav
                if Pav_prev == 0:
                    Pav = Pnew
                else:
                    Pav = (Pav_prev + Pnew)/2 #min(max((Pav_prev + Pnew)/2, 4/1000), 10/1000)#
                Pav_prev = temp
                lastPeak = i
                tau = .4*min(max(Pav, 4/1000), 10/1000) #Pav
                beta = (min(max(Pav, 4/1000), 10/1000)/.695) #(Pav/.695)
        else:
            tau -= 1/fs

    if Pav == 0:
        Pav = 1

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
    activeUnits = 1000 # ms to s
    ppe *= activeUnits
    #print(ppe)
    # Invalid ppe checks
    if ppe < 1.6:
        threshold = -np.inf
    elif ppe >= 25.5:
        threshold = -np.inf
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
            threshold = -np.inf
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
            threshold = -np.inf
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
            threshold = -np.inf
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
            threshold = -np.inf

    if threshold > 0:
        threshold /= activeUnits*1000
#print(threshold)

    return threshold

def calculate_coincidence(peMatrix, bias):
    '''
    Calculate coicidences of elements in the first row to all 35 other elements.
        We estimate coincidence between two pitch periods by finding the absolute difference between them and 
        if it is less then the coincidence window length we say it coincides.
    NOTE: Can be done in parallel for each bias
    NOTE: PeMatrix is in secs now so threshold must accomodate
    NOTE: Tiebreaker is never specified ):<
    return:
        [winnerIdx, winnerVal]
            winnerIdx -> index into peMatrix[0] for ppe with most coincidences
            winnerVal -> number of coincidences for winnerIdx
    '''
    coincidences = np.zeros(6)

    # Check ppe_i to all elements
    for i in range(6):
        coincidences[i] -= bias
        # Find the coincidence window length (threshold) for ppe_i
        threshold = findThreshold(peMatrix[0][i], bias)
        for x in range(6):
            for y in range(6):
                # NOTE: we are checking when peMatrix[0][i] == peMatrix[y][x] but that shouldn't matter
                #print("curPPE: {} compPPE: {}".format(1/peMatrix[0][i],1/peMatrix[y][x]))
                if np.abs(peMatrix[0][i] - peMatrix[y][x]) <= threshold: #and (0 != y and x != i):
                    #print("Threshold: {}".format(threshold))
                #if np.abs(peMatrix[0][i] - peMatrix[y][x])/peMatrix[0][i] < threshold and (0 != y and x != i):
                    coincidences[i] += 1

    winnerIdx = np.argmax(coincidences)
    #print(1/peMatrix[0][:])
    #print(coincidences)
    winnerVal = coincidences[winnerIdx] 
    #print(winnerVal)
    #print("PPEs: {}\n#ofCoincidences: {}".format(1/peMatrix[0][:], coincidences))
    return [winnerIdx, winnerVal, bias]#[(winnerIdx, winnerVal), bias]

def calculate_ppe_winner(peMatrix, prevWinner):
    '''
    Find the current ppe with the most coincidences in the peMatrix
    TODO: Can be done in parallel for each bias
    return:
        winner -> ppe (in ms) that best estimates the current pitch period
    '''
    #winnerArr = np.zeros((4,2))
    winnerArrIdx = np.zeros(4)
    winnerArrCoin = np.zeros(4)
    biases = [1,2,5,7]
    thresh = np.zeros(4)
    for i in range(4):
        winnerArrIdx[i], winnerArrCoin[i], thresh[i] = calculate_coincidence(peMatrix, biases[i])
        #winnerArr[i], thresh[i] = calculate_coincidence(peMatrix, biases[i])
    '''
    winnerIdx = np.argmax(winnerArr, axis=0)[1]
    winner = peMatrix[0][winnerIdx]
    # If coincidences# - threshold or bias(?) < 0 -> unvoiced
    coincidenceNum = winnerArr[winnerIdx][1]
    #print("Coincidence #s: {} Winner Array: {}".format(winnerArr[:][1], 1/peMatrix[0][winnerArr[:][0]]))
    print(list(winnerArr)[0][:])
    #if coincidenceNum - thresh[winnerIdx] < 0 or winner == 1:
    if coincidenceNum < 0 or winner == 1:
        winner = 0 
    '''
    #Temp
    winnerIdx = np.argmax(winnerArrCoin)
    winner = peMatrix[0][int(winnerArrIdx[winnerIdx])]
    #print("(Freq, C#): ({}, {})".format(1/(peMatrix[0][np.array(winnerArrIdx, dtype=np.int)]), winnerArrCoin))
    #print(thresh[winnerIdx])
    #print(winnerArrCoin[winnerIdx] - thresh[winnerIdx])
    #print(winnerArrCoin[winnerIdx])
    #print(peMatrix[0][np.array(winnerArrIdx[:], dtype=np.int)])
    #print(winnerArrIdx)
    #print(1/winner)
    #print(1/(peMatrix[0][:]))
    #if (winnerArrCoin[winnerIdx] - 4 < 0 or np.abs(1/prevWinner-1/winner) < 200) or winner == 1:
    if winnerArrCoin[winnerIdx] - 0 < 0  or winner == 1:
        winner = 0
    return winner

def pproc_calculate_pitch(sound, t, framesize=.042, fs=12000, ecutoff=.35):
    '''
    Combine the entire pproc algorith into one easy to use function
    TODO: What to do with potential leftover frames
    NOTE: Assuming sound is mono
    return:
        estimates -> np.array of pitch estimates
    '''
    #estimates = np.zeros(sound.size)
    estimates = np.zeros(int((len(sound)/fs)/framesize))

    # Filter the sound
    filt = generate_filter('default', fs=fs)
    filtSound = filter_audio(sound, filt)
    prevPPE_1 = np.zeros(6)
    prevPPE_2 = np.zeros(6)
    ppe = np.zeros(6)
    prevWinner = 1
    
    # Convert framesize to samples
    updateSize = round(fs * framesize)
    i = updateSize
    counter = 0
    while i < len(sound):
        windowedFrame = filtSound[i-updateSize:i]*np.hamming(len(filtSound[i-updateSize:i]))
        # LAB 4: V/U detection
        energy = np.sum(np.square(np.abs(filtSound[i-updateSize:i])))
        if energy < ecutoff:
            i += updateSize
            counter += 1
            #print(energy)
            continue
        # Make peak measurements
        #print(len(filtSound[i-updateSize:i]))
        # NOTE: ok so hanning window is amazing... 
        # TODO: test different windows?
        peaks = find_peaks(windowedFrame)
        # Update previous PPEs
        prevPPE_2 = prevPPE_1
        prevPPE_1 = ppe
        # Get current PPE
        # TODO: Parallel?
        for j in range(6):
            ppe[j] = peak_rundown(peaks[j], t, fs=fs)
        if prevPPE_2.all() == 0:
            continue
        # Create the ppe matrix
        peMatrix = create_pitch_matrix(ppe, prevPPE_1, prevPPE_2)
        #print(1/peMatrix[0][:])
        # Find current best pitch estimate
        # NOTE: idk what idx into estimtes this is supposed to be?
        #estimates[(2*i-updateSize)//2] = calculate_ppe_winner(peMatrix, prevWinner)
        estimates[counter] = calculate_ppe_winner(peMatrix, prevWinner)
        #print(estimates[counter])
        #prevWinner = estimates[(2*i-updateSize)//2]
        prevWinner = estimates[counter]
        i += updateSize
        counter += 1

    return estimates
