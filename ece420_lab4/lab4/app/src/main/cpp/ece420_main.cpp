//
// Created by daran on 1/12/2017 to be used in ECE420 Sp17 for the first time.
// Modified by dwang49 on 1/1/2018 to adapt to Android 7.0 and Shield Tablet updates.
//

#include "ece420_main.h"
#include "ece420_lib.h"
#include "kiss_fft/kiss_fft.h"

// JNI Function
extern "C" {
JNIEXPORT float JNICALL
Java_com_ece420_lab4_MainActivity_getFreqUpdate(JNIEnv *env, jclass);
}

// Student Variables
#define F_S 48000
#define FRAME_SIZE 1024
#define VOICED_THRESHOLD 3e9  // Find your own threshold
#define START 40
#define END FRAME_SIZE/4
float lastFreqDetected = -1;

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
//    def ece420ProcessFrame(frame, Fs):
//    freq = -1
//    if np.sum(np.square(frame))>threshold:
//    auto=np.fft.ifft(np.fft.fft(frame)*np.conjugate(np.fft.fft(frame)))
//    peak=np.argmax(np.abs(auto[start:len(frame)//2]))
//    freq=Fs/peak
//    return freq
    float energy=0;
    for (int i=0;i<FRAME_SIZE;i++){
        energy+=bufferIn[i]*bufferIn[i];
    }
    if(energy>VOICED_THRESHOLD){
        kiss_fft_cfg cfg=kiss_fft_alloc(FRAME_SIZE,0,NULL,NULL);
        kiss_fft_cfg cfg_inv=kiss_fft_alloc(FRAME_SIZE,1,NULL,NULL);

        kiss_fft_cpx in[FRAME_SIZE];
        kiss_fft_cpx fft[FRAME_SIZE];
        kiss_fft_cpx ifft[FRAME_SIZE];

        for (int i=0;i<FRAME_SIZE;i++){
            in[i].r=bufferIn[i];
            in[i].i=0;
        }
        kiss_fft(cfg,in,fft);
        for (int i=0;i<FRAME_SIZE;i++){
            in[i].r=(fft[i].r*fft[i].r+fft[i].i*fft[i].i);
            in[i].i=0;
        }
        kiss_fft(cfg_inv,in,ifft);
        float max_value=-1;
        int peak=0;
        for (int i=START;i<END;i++){
            float mag=ifft[i].r*ifft[i].r+ifft[i].i*ifft[i].i;
            if(mag>max_value){
                max_value=mag;
                peak=i;
            }
        }
        lastFreqDetected=F_S/peak;
        free(cfg);
        free(cfg_inv);
    } else{
        lastFreqDetected=-1;
    }

    // ********************* END YOUR CODE HERE ************************* //
    gettimeofday(&end, NULL);
    LOGD("Time delay: %ld us",  ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec)));
}

JNIEXPORT float JNICALL
Java_com_ece420_lab4_MainActivity_getFreqUpdate(JNIEnv *env, jclass) {
    return lastFreqDetected;
}