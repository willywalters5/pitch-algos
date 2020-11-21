//
// Created by Billy Walters on 11/18/2020.
//

#include "pproc_helper.h"
#include "numpy_scipy_funcs.h"
#include <vector>
#include <cmath>

///@note these are also defined in ece420_main.cpp, would be ideal to have it in one spot
#define FRAME_SIZE 2048
#define F_S 48000

using namespace std;

/* Helper functions */
static void simple_peaks_helper(float* bufferIn, float* m1, float* m4);
static void peak_valley_helper(float* m1, float* m4, float* m2, float* m5);
static void adj_peak_helper(float* m1, float* m4, float* m3, float* m6);
static double find_threshold_helper(double ppe, int bias);
static int calculate_coincidence_helper(double* peMatrix, int bias, int & winnerVal);

static const double myfilter[PPROC_N_TAPS] = {-0.045707026468951036, -0.000736464650731801, -0.0006802991242345555, -0.0005820337312843551, -0.00043747169503914607, -0.00025401277843976194, -2.2701615827569466e-05,
                           0.000254090008842682, 0.0005740518987143381, 0.0009405796800272398, 0.0013501188488803245, 0.0018041359392405587, 0.0023051194880170586, 0.002845253658192416, 0.003428641437550249,
                           0.004051080441101529, 0.0045970354009871595, 0.0053421356686286225, 0.006052911822927963, 0.006791226491028854, 0.007560343840727943, 0.008344780121754027, 0.009152756328461638,
                           0.009978708005491721, 0.01081599491541043, 0.011664841169365885, 0.012518161217832474, 0.01337390225170864, 0.014232523596784452, 0.015085021709040601, 0.01593274816571686,
                           0.01677104555589419, 0.017556509244288275, 0.018363046974366255, 0.01913183876062738, 0.019870704464694598, 0.02058701455590612, 0.021257046548274428, 0.021892877989646425,
                           0.022489715086781333, 0.023040522344335977, 0.023546375341996238, 0.024000763731923953, 0.02440198712269808, 0.02475230680552492, 0.02504567777192765, 0.02528530837175122,
                           0.02547058471713809, 0.025577990109039377, 0.02564290015271211, 0.02564290015271211, 0.025577990109039377, 0.02547058471713809, 0.02528530837175122, 0.02504567777192765, 0.02475230680552492,
                           0.02440198712269808, 0.024000763731923953, 0.023546375341996238, 0.023040522344335977, 0.022489715086781333, 0.021892877989646425, 0.021257046548274428, 0.02058701455590612,
                           0.019870704464694598, 0.01913183876062738, 0.018363046974366255, 0.017556509244288275, 0.01677104555589419, 0.01593274816571686, 0.015085021709040601, 0.014232523596784452, 0.01337390225170864,
                           0.012518161217832474, 0.011664841169365885, 0.01081599491541043, 0.009978708005491721, 0.009152756328461638, 0.008344780121754027, 0.007560343840727943, 0.006791226491028854, 0.006052911822927963,
                           0.0053421356686286225, 0.0045970354009871595, 0.004051080441101529, 0.003428641437550249, 0.002845253658192416, 0.0023051194880170586, 0.0018041359392405587, 0.0013501188488803245,
                           0.0009405796800272398, 0.0005740518987143381, 0.000254090008842682, -2.2701615827569466e-05, -0.00025401277843976194, -0.00043747169503914607, -0.0005820337312843551, -0.0006802991242345555,
                           -0.000736464650731801, -0.045707026468951036};

// FirFilter Function
float firFilter(float sample, float* circBuf, int & circBufIdx) {
    float output = myfilter[0] * sample;
    for(int i = 1; i < PPROC_N_TAPS; ++i) {
        output += circBuf[(circBufIdx + i) % PPROC_N_TAPS] * myfilter[i];
    }
    circBuf[circBufIdx] = sample;
    circBufIdx = (circBufIdx + 1) % PPROC_N_TAPS;
    return output;
}

/**
 * Get the heights for the pulses used in peak rundown (descriptions for each mi found in helpers)
 * @param bufferIn - input audio frame (filtered and windowed) w/ len of FRAME_SIZE
 * @params m1...m6 - output vectors of pulse heights w/ len of FRAME_SIZE
 * @note m1...m6 must be len FRAME_SIZE w/ all elements initialized to zero
 * @return none
 */
void find_peaks(float* bufferIn, float* m1, float* m2, float* m3, float* m4, float* m5, float* m6) {
    // Helpers ensure all mi are positive pulses
    simple_peaks_helper(bufferIn, m1, m4);
    peak_valley_helper(m1, m4, m2, m5);
    adj_peak_helper(m1, m4, m3, m6);

    return;
}

/**
 * Elementary period detection using a peak rundown circuit
 * @param m - input filled pulse vector from find_peaks
 * @return Pav - smoothed period estimate (in seconds)
 * @todo offline tests for this function (especially the (i/F_S) - (lastPeak/F_S) part
 */
double peak_rundown(float* m) {
    int lastPeak;
    double Pav = 0;
    double Pav_prev = 0;
    double Pnew = 0;
    double beta = .016/.000695;//(16/1000)/0.695;
    double tau = .0004*.16;//0.4*(16/1000);
    vector<int> mNonZeroIdx;
    numpy_nonzero_idx(m, FRAME_SIZE, mNonZeroIdx);

    // Find first peak (excluding first sample)
    int start = mNonZeroIdx.size();
    if(mNonZeroIdx.size() != 0)
        start = mNonZeroIdx[0];
    lastPeak = start;

    // Start blanking
    ///@todo could probably add efficiency by only looking at nonzero m values
    for(int i = start; i < FRAME_SIZE; ++i) {
        if(tau <= 0) {
            // Start exponential decay, if peak exceeds exp decay
            // then update period and restart blanking
            if(m[i] > m[lastPeak]*exp(-beta*(((double)i/(double)F_S) - ((double)lastPeak/(double)F_S))) && m[i] > 0 && i != 0) {
                Pnew = ((double)i/(double)F_S) - ((double)lastPeak/(double)F_S);
                double temp = Pav;
                if(Pav_prev == 0)
                    Pav = Pnew;
                else
                    Pav = fmin(fmax((Pav_prev + Pnew)/2, .003), .016);
                Pav_prev = temp;
                lastPeak = i;
                tau = 0.4*fmin(fmax(Pav, 4/1000), 10/1000);
                beta = (fmin(fmax(Pav, 4/1000), 10/1000)/.695);
            }
        }
        else
            tau -= .0000208;//(double)(1/F_S);
    }

    // Avoid divide by zero
    if(Pav == 0)
        Pav = -1;

    return Pav;
}

/**
 * Generate the pitch period estimation matrix from the 3 most recent pitch estimations
 * @param curPPE - input array of most recent pitch period estimations
 * @param prevPPE_1 - input array of previous pitch period estimations
 * @param prevPPE_2 - input array of 2nd previous pitch period estimations
 * @param peMatrix - output 6x6 matrix of pitch period estimations
 * @return none
 */
void create_pitch_matrix(double* curPPE, double* prevPPE_1, double* prevPPE_2, double* peMatrix) {

    for(int i = 0; i < 6; ++i) {
        peMatrix[i] = curPPE[i];     // [0][i]
        peMatrix[6 + i] = prevPPE_1[i];  // [1][i]
        peMatrix[6*2 + i] = prevPPE_2[i];  // [2][i]
        peMatrix[6*3 + i] = peMatrix[i] + peMatrix[6 + i]; // [3][i]
        peMatrix[6*4 + i] = peMatrix[6 + i] + peMatrix[6*2 + i]; // [4][i]
        peMatrix[5*4 + i] = peMatrix[i] + peMatrix[6 + i] + peMatrix[6*2 + i]; // [5][i]
    }

    return;
}

/**
 * Find the current pitch estimate with the most coincidences in peMatrix
 * @param peMatrix - input 6x6 matrix of pitch period estimates
 * @param prevWinner - input previous detected pitch
 * @return winner - detected pitch, the pitch estimate with the most coincidences
 */
double calculate_ppe_winner(double* peMatrix, double prevWinner) {
    int winnerIdx;
    double winner;
    int winnerArrIdx[4];
    int winnerArrCoin[4] = {};
    int biases[4] = {1,2,5,7};

    for(int i = 0; i < 4; ++i) {
        winnerArrIdx[i] = calculate_coincidence_helper(peMatrix, biases[i], winnerArrCoin[i]);
    }

    winnerIdx = numpy_argmax(winnerArrCoin, 4);
    winner = peMatrix[winnerArrIdx[winnerIdx]];

    if(winnerArrCoin[winnerIdx] < 0)
        winner = -1;

    return winner;
}

/******************************* Helper Functions **********************************************/

/**
 * Find each local min/max
 * @param bufferIn - input audio signal
 * @param m1 - output vector of local maxima
 * @param m4 - output vector of local minima
 * @return none
 */
static void simple_peaks_helper(float* bufferIn, float* m1, float* m4) {

    for(int i = 1; i < FRAME_SIZE - 1; i++) {
        if(bufferIn[i-1] <= bufferIn[i] && bufferIn[i+1] <= bufferIn[i])
            m1[i] = bufferIn[i];
    }
    for(int i = 1; i < FRAME_SIZE - 1; i++) {
        if(bufferIn[i-1] >= bufferIn[i] && bufferIn[i+1] >= bufferIn[i])
            m4[i] = bufferIn[i];
    }

    return;
}

/**
 * Find distance between mins and maxes
 * @param m1 - input vector of local maxima
 * @param m4 - input vector of local minima
 * @param m2 - output vector of abs(max)-abs(prev_min)
 * @param m5 - output vector of abs(prev_max)-abs(min)
 * @note caller must reserve m2 and m5 to be initialized to zero for the size of m1
 * @return none
 */
static void peak_valley_helper(float* m1, float* m4, float* m2, float* m5) {
    vector<int> m1NonZeroIdx;
    vector<int> m4NonZeroIdx;
    numpy_nonzero_idx(m1, FRAME_SIZE, m1NonZeroIdx);
    numpy_nonzero_idx(m4, FRAME_SIZE, m4NonZeroIdx);

    // First peak before first valley
    if(m4NonZeroIdx.size() > 0 && m1NonZeroIdx[0] < m4NonZeroIdx[0]) {
        m2[m1NonZeroIdx[0]] = abs( m1[m1NonZeroIdx[0]] ); // added abs(...) -> Pospulse
        m5[m4NonZeroIdx[0]] = abs( abs(m1[m1NonZeroIdx[0]]) - abs(m4[m4NonZeroIdx[0]]) ); // added abs(...) -> Pospulse

        for(int i = 1; i < m1NonZeroIdx.size(); ++i) {
            if(i-1 < m4NonZeroIdx.size())
                m2[m1NonZeroIdx[i]] = abs( abs(m1[m1NonZeroIdx[i]]) - abs(m4[m4NonZeroIdx[i-1]]) ); // added abs(...) -> Pospulse
            if(i < m4NonZeroIdx.size())
                m5[m4NonZeroIdx[i]] = abs( abs(m1[m1NonZeroIdx[i]]) - abs(m4[m4NonZeroIdx[i]]) ); // added abs(...) -> Pospulse
        }
    }

    // First peak after first valley
    else if(m1NonZeroIdx.size() > 0) {
        m5[m4NonZeroIdx[0]] = abs( m4[m4NonZeroIdx[0]] ); // added abs(...) -> Pospulse

        for(int i = 0; i < m1NonZeroIdx.size(); ++i) {
            if(i < m4NonZeroIdx.size())
                m2[m1NonZeroIdx[i]] = abs( abs(m1[m1NonZeroIdx[i]]) - abs(m4[m4NonZeroIdx[i]]) ); // added abs(...) -> Pospulse
            if(i+1 < m4NonZeroIdx.size())
                m5[m4NonZeroIdx[i+1]] = abs( abs(m1[m1NonZeroIdx[i]]) - abs(m4[m4NonZeroIdx[i+1]]) ); // added abs(...) -> Pospulse
        }
    }

    return;
}

/**
 * Find the difference between adjacent peaks/valleys
 * @param m1 - input vector of local maxima
 * @param m4 - input vector of local minima
 * @param m3 - output vector of peak-to-peak differences
 * @param m6 - output vector of valley-to-valley differences
 * @note caller must reserve m2 and m5 to be initialized to zero for the size of m1
 * @todo the negative/zero checking in this one? Or it might already be done w/ if inside loops
 * @return none
 */
static void adj_peak_helper(float* m1, float* m4, float* m3, float* m6) {
    vector<int> m1NonZeroIdx;
    vector<int> m4NonZeroIdx;
    numpy_nonzero_idx(m1, FRAME_SIZE, m1NonZeroIdx);
    numpy_nonzero_idx(m4, FRAME_SIZE, m4NonZeroIdx);

    int startLen = m1NonZeroIdx.size() - (m1NonZeroIdx.size() % 2);
    if(m1NonZeroIdx.size() > 1) {
        for(int i = 0; i < startLen; i += 2) {
            if(m1NonZeroIdx[i+1] > m1NonZeroIdx[i])
                m3[m1NonZeroIdx[i]] = abs( m1[m1NonZeroIdx[i+1]] - m1[m1NonZeroIdx[i]] );
        }
    }

    startLen = m4NonZeroIdx.size() - (m4NonZeroIdx.size() % 2);
    if(m4NonZeroIdx.size() > 1) {
        for(int i = 0; i < startLen; i += 2) {
            if(m4NonZeroIdx[i+1] > m4NonZeroIdx[i])
                m6[m4NonZeroIdx[i]] = abs (m6[m4NonZeroIdx[i+1]] - m6[m4NonZeroIdx[i]] );
        }
    }

    return;
}

/**
 * Find the coincidence threshold for a given ppe and bias
 * @param ppe - input pitch period estimation in seconds
 * @param bias - input coincidence bias
 * @return threshold - the coincidence threshold, -1 if invalid ppe or bias
 */
static double find_threshold_helper(double ppe, int bias) {
    int threshold = -1;
    ppe *= 1000;

    if(1.6 <= ppe && ppe < 3.1) {
        switch(bias) {
            case 1:
                threshold = 100;
                break;
            case 2:
                threshold = 200;
                break;
            case 5:
                threshold = 300;
                break;
            case 7:
                threshold = 400;
                break;
            default:
                threshold = -1;
        }
    }
    else if(3.1 <= ppe && ppe < 6.3) {
        switch(bias) {
            case 1:
                threshold = 100*2;
                break;
            case 2:
                threshold = 200*2;
                break;
            case 5:
                threshold = 300*2;
                break;
            case 7:
                threshold = 400*2;
                break;
            default:
                threshold = -1;
        }
    }
    else if(6.3 <= ppe && ppe < 12.7) {
        switch(bias) {
            case 1:
                threshold = 100*4;
                break;
            case 2:
                threshold = 200*4;
                break;
            case 5:
                threshold = 300*4;
                break;
            case 7:
                threshold = 400*4;
                break;
            default:
                threshold = -1;
        }
    }
    else if(12.7 <= ppe && ppe < 25.5) {
        switch(bias) {
            case 1:
                threshold = 100*8;
                break;
            case 2:
                threshold = 200*8;
                break;
            case 5:
                threshold = 300*8;
                break;
            case 7:
                threshold = 400*8;
                break;
            default:
                threshold = -1;
        }
    }

    return ((double)threshold)*.000001; // microseconds to seconds
}

/**
 * Calculate coincidences of elements in the first row of peMatrix to all other 35 elements
 * We estimate coincidence between two pitch periods by finding the absolute difference between
 * them and if it is less then the coincidence threshold we say it coincides.
 * @param peMatrix - input 6x6 pitch estimation matrix
 * @param bias - input bias for coincidence counting
 * @param winnerVal - greatest number of coincidences
 * @return winnerIdx - index of winnerVal
 */
static int calculate_coincidence_helper(double* peMatrix, int bias, int & winnerVal) {
    int winnerIdx;
    double threshold;
    int coincidences[6] = {};

    // Check ppe_i to all elements
    for(int i = 0; i < 6; ++i) {
        coincidences[i] -= bias;
        // Find the coincidence threshold for ppe_i
        threshold = find_threshold_helper(peMatrix[i], bias);
        for(int x = 0; x < 6; ++x) {
            for(int y = 0; y < 6; ++y) {
                if(abs(peMatrix[i] - peMatrix[6*y + x]) <= threshold)
                    ++coincidences[i];
            }
        }
    }

    winnerIdx = numpy_argmax(coincidences, 6);
    winnerVal = coincidences[winnerIdx];
    return winnerIdx;
}
