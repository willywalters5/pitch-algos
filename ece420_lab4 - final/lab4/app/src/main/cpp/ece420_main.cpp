//
// Created by daran on 1/12/2017 to be used in ECE420 Sp17 for the first time.
// Modified by dwang49 on 1/1/2018 to adapt to Android 7.0 and Shield Tablet updates.
//

#include "ece420_main.h"
#include "ece420_lib.h"
#include "kiss_fft/kiss_fft.h"
#include "pproc_helper.h"
#include "numpy_scipy_funcs.h"
#include <vector>

// JNI Function
extern "C" {
JNIEXPORT float JNICALL
Java_com_ece420_lab4_MainActivity_getFreqUpdate(JNIEnv *env, jclass,jint algo);
}
extern "C" {
JNIEXPORT void JNICALL
Java_com_ece420_lab4_MainActivity_cppCleanup(JNIEnv *env, jclass);
}
extern "C" {
JNIEXPORT float JNICALL
Java_com_ece420_lab4_PrerecordActivity_getUpdate(JNIEnv *env, jclass,jfloatArray curr_frame,jint algo);
}

// Student Variables
#define F_S 48000
#define FRAME_SIZE 2048
#define VOICED_THRESHOLD 1e8  // Find your own threshold
#define START 40
#define END FRAME_SIZE/4
float lastFreqDetected = -1;
int selectedAlgo=0; //0 for AUTOC, 1 for CEP, 2 for PPROC, 3 for SIFT
float bufferInPrerecord[FRAME_SIZE];

//CEP Variables

#define CEP_ZERO_CROSSING_THRESHOLD FRAME_SIZE/3
#define CEP_MAX_INTERVAL F_S/60
#define CEP_MIN_INTERVAL F_S/300
#define CEP_VOICED_THRESHOLD 1e8
#define PI 3.1415926

// PPROC Variables
float pprocCircBuf[PPROC_N_TAPS] = {};
int pprocCircBufIdx = 0;
kaiserWinObj kaiserWin(FRAME_SIZE, 1.75);
double* prevPPE_1 = NULL;
double* prevPPE_2 = NULL;
double* ppe = NULL;
double prevWinner = -1;

#define DS_FRAME_SIZE 410
float siftCircBuf[PPROC_N_TAPS] = {};
int siftCircBufIdx = 0;

void ece420ProcessFrame(sample_buf *dataBuf) {
    // Keep in mind, we only have 20ms to process each buffer!
    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);

    // Data is encoded in signed PCM-16, little-endian, mono
    float bufferIn[FRAME_SIZE];
    for (int i = 0; i < FRAME_SIZE; i++) {
        int16_t val = ((uint16_t) dataBuf->buf_[2 * i]) | (((uint16_t) dataBuf->buf_[2 * i + 1]) << 8);
        bufferIn[i] = (float) val;
    }

    // ********************** PITCH DETECTION ************************ //
    // In this section, you will be computing the autocorrelation of bufferIn
    // and picking the delay corresponding to the best match. Naively computing the
    // autocorrelation in the time domain is an O(N^2) operation and will not fit
    // in your timing window.
    //
    // First, you will have to detect whether or not a signal is voiced.
    // We will implement a simple voiced/unvoiced detector by thresholding
    // the power of the signal.
    //
    // Next, you will have to compute autocorrelation in its O(N logN) form.
    // Autocorrelation using the frequency domain is given as:
    //
    //  autoc = ifft(fft(x) * conj(fft(x)))
    //
    // where the fft multiplication is element-wise.
    //
    // You will then have to find the index corresponding to the maximum
    // of the autocorrelation. Consider that the signal is a maximum at idx = 0,
    // where there is zero delay and the signal matches perfectly.
    //
    // Finally, write the variable "lastFreqDetected" on completion. If voiced,
    // write your determined frequency. If unvoiced, write -1.
    // ********************* START YOUR CODE HERE *********************** //
    if(selectedAlgo==0){
        AutoCPitchDetection(bufferIn);
    } else if (selectedAlgo==1){
        CEPPitchDetection(bufferIn);
    } else if (selectedAlgo==2){
        PPROCPitchDetection(bufferIn);
    }
    else{
        SIFTPitchDetection(bufferIn);
    }
    // ********************* END YOUR CODE HERE ************************* //
    gettimeofday(&end, NULL);
    LOGD("Time delay: %ld us",  ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec)));
}

void AutoCPitchDetection(float *bufferIn){
    kiss_fft_cpx autoCfft[FRAME_SIZE] = {};
    kiss_fft_cpx autoC[FRAME_SIZE] = {};
    kiss_fft_cpx bufCpx[FRAME_SIZE];
    kiss_fft_cfg cfg = kiss_fft_alloc(FRAME_SIZE, 0, NULL, NULL);
    kiss_fft_cfg cfg_ifft = kiss_fft_alloc(FRAME_SIZE, 1, NULL, NULL);
    int numPeriods = 2;
    ///@todo find middle ground between V/U detection and accuraccy w/ tolerance and energy thresh
    float tolerance = 0.6;
    int searchRange = FRAME_SIZE / numPeriods;
    float maxAutoC = 0;
    float minAutoC = 999;
    int minFirst = 0;
    int maxFirst = 0;
    int minSecond = 0;
    int period = 0;
    ///@note This fixes the issue of no good energy threshold, but now the results are less accurate
    float normFactor = 0;

    //float testReal[2] = {1,2};
    //kiss_fft_cpx testCpx[2];
    //memcpy(testCpx, const_cast<{{testReal, 0}}>, sizeof(testCpx));
    // Check if frame is voiced and also prepare buffer for fft if needed
    float energy = 0;
    int i;
    for (i = 0; i < FRAME_SIZE; ++i) {
        energy += abs(bufferIn[i]) * abs(bufferIn[i]);
        bufCpx[i].r = bufferIn[i];
        bufCpx[i].i = 0;
        normFactor += energy;
    }
    if (energy < VOICED_THRESHOLD) {
        lastFreqDetected = -1;
        return;
    }

    // Find the auto correlation using fft method
    kiss_fft(cfg, bufCpx, autoCfft);
    free(cfg);
    for (i = 0; i < FRAME_SIZE; ++i) {
        // fft(x) * conj(fft(x))
        autoCfft[i].r = (autoCfft[i].r * autoCfft[i].r) + (autoCfft[i].i * autoCfft[i].i);
        autoCfft[i].i = 0;
    }
    kiss_fft(cfg_ifft, autoCfft, autoC);
    free(cfg_ifft);

    ///@todo integrate if works
    for (i = 0; i < FRAME_SIZE; ++i) {
        autoC[i].r /= normFactor;
    }

    // Find the min and max of the autoCorrelation
    //maxAutoC = std::max_element(autoC + 2, autoC + searchRange)->r;
    //minAutoC = std::min_element(autoC + 2, autoC + searchRange)->r;
    for (i = 2; i < searchRange; ++i) {
        if (maxAutoC < autoC[i].r) {
            maxAutoC = autoC[i].r;
        }
        if (minAutoC > autoC[i].r) {
            minAutoC = autoC[i].r;
        }
    }

    // Find first min
    for (i = 2; i < searchRange; ++i) {
        if (autoC[i].r <= minAutoC + tolerance) {
            minFirst = i;
            break;
        }
    }
    // Find First max after first min
    for (i = minFirst; i < searchRange; ++i) {
        if (autoC[i].r >= maxAutoC - tolerance) {
            maxFirst = i;
            break;
        }
    }
    // Find next min
    for (i = maxFirst; i < searchRange; ++i) {
        minSecond = i;
        if (autoC[i].r <= minAutoC + tolerance) {
            break;
        }
    }

    // Calculate period
    for (i = maxFirst; i < minSecond; ++i) {
        if (autoC[i].r > autoC[period].r) {
            period = i;
        }
    }
    //period = findMaxArrayIdx(autoC.r, maxFirst, minSecond) + maxFirst;
    period += maxFirst;
    lastFreqDetected = F_S / period;

    return;
}

void CEPPitchDetection(float *bufferIn) {
    //lastFreqDetected=-1;
    float zero_crossing=0;
    float energy=0;
    int prev_sign=0;
    int curr_sign=0;

    for(int i=0;i<FRAME_SIZE;i++){
        energy+=bufferIn[i]*bufferIn[i];
        //Count zero-crossings
        if(bufferIn[i]>0){
            curr_sign=1;
        }
        else{
            curr_sign=-1;
        }
        if(curr_sign!=prev_sign){
            zero_crossing++;
        }
        prev_sign=curr_sign;
    }
    if(energy<CEP_VOICED_THRESHOLD || zero_crossing>CEP_ZERO_CROSSING_THRESHOLD){
        lastFreqDetected=-1;
        return;
    }

    float windowed[FRAME_SIZE];
    for(int i=0;i<FRAME_SIZE;i++){
        windowed[i]=bufferIn[i]*(0.54-0.46*cos((2*PI*i)/(FRAME_SIZE-1)));
    }

    kiss_fft_cfg cfg=kiss_fft_alloc(FRAME_SIZE,0,NULL,NULL);
    kiss_fft_cfg cfg_inv=kiss_fft_alloc(FRAME_SIZE,1,NULL,NULL);

    kiss_fft_cpx in[FRAME_SIZE];
    kiss_fft_cpx fft[FRAME_SIZE];
    kiss_fft_cpx cep[FRAME_SIZE];
    //compute FFT
    for (int i=0;i<FRAME_SIZE;i++){
        in[i].r=windowed[i];
        in[i].i=0;
    }
    kiss_fft(cfg,in,fft);

    //compute log value of absolute value of fft
    for (int i=0;i<FRAME_SIZE;i++){
        fft[i].r=log(sqrt(fft[i].r*fft[i].r+fft[i].i*fft[i].i));
        fft[i].i=0;
    }
    kiss_fft(cfg_inv,fft,cep);
    int pitch_interval=0;
    float max_cep=-1;
    for (int i=CEP_MIN_INTERVAL;i<CEP_MAX_INTERVAL;i++){
        float curr_cep=cep[i].r*cep[i].r+cep[i].i*cep[i].i;
        if(curr_cep>max_cep){
            max_cep=curr_cep;
            pitch_interval=i;
        }
    }
    lastFreqDetected=F_S/pitch_interval;
}
void PPROCPitchDetection(float *bufferIn) {
    // Apply LPF
    for (int sampleIdx = 0; sampleIdx < FRAME_SIZE; sampleIdx++) {
        float sample = bufferIn[sampleIdx];
        float output = firFilter(sample, pprocCircBuf, pprocCircBufIdx);
        bufferIn[sampleIdx] = output;
    }

    // Apply Kaiser window
    for (int sampleIdx = 0; sampleIdx < FRAME_SIZE; sampleIdx++) {
        bufferIn[sampleIdx] *= kaiserWin.getKaiserCoef(sampleIdx);
    }

    // V/U detection
    float energy = 0;
    for (int i = 0; i < FRAME_SIZE; i++) {
        energy += bufferIn[i] * bufferIn[i];
    }
    if (energy < VOICED_THRESHOLD) {
        lastFreqDetected = -1;
        return;
    }

    // Make peak measurements
    float m1[FRAME_SIZE] = {};
    float m2[FRAME_SIZE] = {};
    float m3[FRAME_SIZE] = {};
    float m4[FRAME_SIZE] = {};
    float m5[FRAME_SIZE] = {};
    float m6[FRAME_SIZE] = {};
    find_peaks(bufferIn, m1, m2, m3, m4, m5, m6);
    float* peaks[6] = { m1, m2, m3, m4, m5, m6 };

    // Update previous PPEs
    ///@todo optimize w/out dynamic allocation
    if (prevPPE_2 != NULL)
        delete[] prevPPE_2;
    prevPPE_2 = prevPPE_1;
    prevPPE_1 = ppe;
    ppe = new double[6];

    // Get current PPE
    for (int j = 0; j < 6; ++j) {
        ppe[j] = peak_rundown(peaks[j]);
    }
    if (prevPPE_2 == NULL || prevPPE_1 == NULL) {
        lastFreqDetected = -1;
        return;
    }

    // Create PPE matrix
    ///@todo see if this is necessary
    double* peMatrix = new double[36];
    create_pitch_matrix(ppe, prevPPE_1, prevPPE_2, peMatrix);

    // Find current best pitch estimate
    prevWinner = calculate_ppe_winner(peMatrix, prevWinner);
    delete[] peMatrix;

    if (prevWinner == -1)
        lastFreqDetected = -1;
    else
        lastFreqDetected = 1 / prevWinner;
}

void SIFTPitchDetection(float *bufferIn){
    kiss_fft_cpx autoCfft[FRAME_SIZE] = {};
    kiss_fft_cpx autoC[FRAME_SIZE] = {};
    kiss_fft_cpx bufCpx[FRAME_SIZE];
    kiss_fft_cfg cfg = kiss_fft_alloc(FRAME_SIZE, 0, NULL, NULL);
    kiss_fft_cfg cfg_ifft = kiss_fft_alloc(FRAME_SIZE, 1, NULL, NULL);
    int numPeriods = 2;
    float tolerance = 0.6;
    int searchRange = FRAME_SIZE / numPeriods;
    float maxAutoC = 0;
    float minAutoC = 999;
    int minFirst = 0;
    int maxFirst = 0;
    int minSecond = 0;
    int period = 0;
    float normFactor = 0;

    for (int sampleIdx = 0; sampleIdx < FRAME_SIZE; sampleIdx++) {
        float sample = bufferIn[sampleIdx];
        float output = firFilter(sample, siftCircBuf, siftCircBufIdx);
        bufferIn[sampleIdx] = output;
    }

    float energy = 0;
    int counter = 0;
    int i =0;

    //Downsample by 5 after LPF
    for (int j = 0; j < FRAME_SIZE; j += 5) {
        energy += abs(bufferIn[j])*abs(bufferIn[j]);
        bufCpx[counter].r = bufferIn[j];
        bufCpx[counter].i = 0;
        normFactor += energy;
        counter++;
    }

    if (energy < 20000000) {
        lastFreqDetected = -1;
        return;
    }
    // Find the auto correlation using fft method
    kiss_fft(cfg, bufCpx, autoCfft);
    free(cfg);
    for (i = 0; i < FRAME_SIZE; ++i) {
        // fft(x) * conj(fft(x))
        autoCfft[i].r = (autoCfft[i].r * autoCfft[i].r) + (autoCfft[i].i * autoCfft[i].i);
        autoCfft[i].i = 0;
    }
    kiss_fft(cfg_ifft, autoCfft, autoC);
    free(cfg_ifft);

    for (i = 0; i < FRAME_SIZE; ++i) {
        autoC[i].r /= normFactor;
    }


    float R[4][4] = {};
    int len = (DS_FRAME_SIZE/2)-1;
    for (int j = 0; j<4; j++)
    {
        R[j][j]=autoC[len].r;
        for(int k = j+1; k<4; k++){
            R[j][k]=autoC[len+k-j].r;
            R[k][j]=autoC[len+k-j].r;
        }
    }

    float m[16] = {};
    for(int j = 0; j<4; j++) {
        for (int k = j; k < 4; k++) {
            m[j * 4 + k]+=R[j][k];
        }
    }

    float inv[16], det;

    inv[0] = m[5]  * m[10] * m[15] -
             m[5]  * m[11] * m[14] -
             m[9]  * m[6]  * m[15] +
             m[9]  * m[7]  * m[14] +
             m[13] * m[6]  * m[11] -
             m[13] * m[7]  * m[10];

    inv[4] = -m[4]  * m[10] * m[15] +
             m[4]  * m[11] * m[14] +
             m[8]  * m[6]  * m[15] -
             m[8]  * m[7]  * m[14] -
             m[12] * m[6]  * m[11] +
             m[12] * m[7]  * m[10];

    inv[8] = m[4]  * m[9] * m[15] -
             m[4]  * m[11] * m[13] -
             m[8]  * m[5] * m[15] +
             m[8]  * m[7] * m[13] +
             m[12] * m[5] * m[11] -
             m[12] * m[7] * m[9];

    inv[12] = -m[4]  * m[9] * m[14] +
              m[4]  * m[10] * m[13] +
              m[8]  * m[5] * m[14] -
              m[8]  * m[6] * m[13] -
              m[12] * m[5] * m[10] +
              m[12] * m[6] * m[9];

    inv[1] = -m[1]  * m[10] * m[15] +
             m[1]  * m[11] * m[14] +
             m[9]  * m[2] * m[15] -
             m[9]  * m[3] * m[14] -
             m[13] * m[2] * m[11] +
             m[13] * m[3] * m[10];

    inv[5] = m[0]  * m[10] * m[15] -
             m[0]  * m[11] * m[14] -
             m[8]  * m[2] * m[15] +
             m[8]  * m[3] * m[14] +
             m[12] * m[2] * m[11] -
             m[12] * m[3] * m[10];

    inv[9] = -m[0]  * m[9] * m[15] +
             m[0]  * m[11] * m[13] +
             m[8]  * m[1] * m[15] -
             m[8]  * m[3] * m[13] -
             m[12] * m[1] * m[11] +
             m[12] * m[3] * m[9];

    inv[13] = m[0]  * m[9] * m[14] -
              m[0]  * m[10] * m[13] -
              m[8]  * m[1] * m[14] +
              m[8]  * m[2] * m[13] +
              m[12] * m[1] * m[10] -
              m[12] * m[2] * m[9];

    inv[2] = m[1]  * m[6] * m[15] -
             m[1]  * m[7] * m[14] -
             m[5]  * m[2] * m[15] +
             m[5]  * m[3] * m[14] +
             m[13] * m[2] * m[7] -
             m[13] * m[3] * m[6];

    inv[6] = -m[0]  * m[6] * m[15] +
             m[0]  * m[7] * m[14] +
             m[4]  * m[2] * m[15] -
             m[4]  * m[3] * m[14] -
             m[12] * m[2] * m[7] +
             m[12] * m[3] * m[6];

    inv[10] = m[0]  * m[5] * m[15] -
              m[0]  * m[7] * m[13] -
              m[4]  * m[1] * m[15] +
              m[4]  * m[3] * m[13] +
              m[12] * m[1] * m[7] -
              m[12] * m[3] * m[5];

    inv[14] = -m[0]  * m[5] * m[14] +
              m[0]  * m[6] * m[13] +
              m[4]  * m[1] * m[14] -
              m[4]  * m[2] * m[13] -
              m[12] * m[1] * m[6] +
              m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] +
             m[1] * m[7] * m[10] +
             m[5] * m[2] * m[11] -
             m[5] * m[3] * m[10] -
             m[9] * m[2] * m[7] +
             m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] -
             m[0] * m[7] * m[10] -
             m[4] * m[2] * m[11] +
             m[4] * m[3] * m[10] +
             m[8] * m[2] * m[7] -
             m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] +
              m[0] * m[7] * m[9] +
              m[4] * m[1] * m[11] -
              m[4] * m[3] * m[9] -
              m[8] * m[1] * m[7] +
              m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] -
              m[0] * m[6] * m[9] -
              m[4] * m[1] * m[10] +
              m[4] * m[2] * m[9] +
              m[8] * m[1] * m[6] -
              m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];


    det = 1.0 / det;

    for ( i = 0; i < 16; i++)
        m[i] = inv[i] * det;

    // Find the min and max of the autoCorrelation
    //maxAutoC = std::max_element(autoC + 2, autoC + searchRange)->r;
    //minAutoC = std::min_element(autoC + 2, autoC + searchRange)->r;
    for (i = 2; i < searchRange; ++i) {
        if (maxAutoC < autoC[i].r) {
            maxAutoC = autoC[i].r;
        }
        if (minAutoC > autoC[i].r) {
            minAutoC = autoC[i].r;
        }
    }

    // Find first min
    for (i = 2; i < searchRange; ++i) {
        if (autoC[i].r <= minAutoC + tolerance) {
            minFirst = i;
            break;
        }
    }
    // Find First max after first min
    for (i = minFirst; i < searchRange; ++i) {
        if (autoC[i].r >= maxAutoC - tolerance) {
            maxFirst = i;
            break;
        }
    }
    // Find next min
    for (i = maxFirst; i < searchRange; ++i) {
        minSecond = i;
        if (autoC[i].r <= minAutoC + tolerance) {
            break;
        }
    }

    // Calculate period
    for (i = maxFirst; i < minSecond; ++i) {
        if (autoC[i].r > autoC[period].r) {
            period = i;
        }
    }
    //period = findMaxArrayIdx(autoC.r, maxFirst, minSecond) + maxFirst;
    period += maxFirst;
    lastFreqDetected = F_S / period/5;

    return;
}

extern "C" JNIEXPORT float JNICALL
Java_com_ece420_lab4_MainActivity_getFreqUpdate(JNIEnv *env, jclass, jint algo) {
    selectedAlgo=(int)algo;
    return lastFreqDetected;
}


extern "C" JNIEXPORT void JNICALL
Java_com_ece420_lab4_MainActivity_cppCleanup(JNIEnv *env, jclass) {
    if(ppe != NULL)
        delete[] ppe;
    if(prevPPE_1 != NULL)
        delete[] prevPPE_1;
    if(prevPPE_2 != NULL)
        delete[] prevPPE_2;
}

extern "C"
JNIEXPORT jfloat JNICALL
Java_com_ece420_lab4_PrerecordActivity_getUpdate(JNIEnv *env, jclass clazz,
                                                    jfloatArray jframe, jint algo) {
    // TODO: implement getCEPUpdate()
    int len = env->GetArrayLength(jframe);
    jfloat* jBuffer = env->GetFloatArrayElements(jframe, 0);

    for (int i=0; i<len; i++) {
        // Convert Java string to std::string
        bufferInPrerecord[i] = jBuffer[i];
    }
    if(algo==0){
        AutoCPitchDetection(bufferInPrerecord);
    } else if (algo==1){
        CEPPitchDetection(bufferInPrerecord);
    } else if (algo==2){
        PPROCPitchDetection(bufferInPrerecord);
    }
    else{
        SIFTPitchDetection(bufferInPrerecord);
    }
    // Release memory
    env->ReleaseFloatArrayElements(jframe,jBuffer,0);
    return lastFreqDetected;
}