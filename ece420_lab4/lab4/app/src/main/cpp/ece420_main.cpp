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
Java_com_ece420_lab4_MainActivity_getFreqUpdate(JNIEnv *env, jclass,jint algo);
}

// Student Variables
#define F_S 48000
#define FRAME_SIZE 1024
#define VOICED_THRESHOLD 3e9  // Find your own threshold
#define START 40
#define END FRAME_SIZE/4
float lastFreqDetected = -1;
int selectedAlgo=0; //0 for AUTOC, 1 for CEP, 2 for PPROC, 3 for SIFT

//CEP Variables

#define CEP_ZERO_CROSSING_THRESHOLD FRAME_SIZE/3
#define CEP_MAX_INTERVAL F_S/60
#define CEP_MIN_INTERVAL F_S/300
#define CEP_VOICED_THRESHOLD 2e9
#define PI 3.1415926

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
    lastFreqDetected=-1;
}
void SIFTPitchDetection(float *bufferIn){
    lastFreqDetected=-1;
}


extern "C" JNIEXPORT float JNICALL
Java_com_ece420_lab4_MainActivity_getFreqUpdate(JNIEnv *env, jclass, jint algo) {
    selectedAlgo=(int)algo;
    return lastFreqDetected;
}