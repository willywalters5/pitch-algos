//
// Created by Billy Walters on 11/18/2020.
//

#ifndef LAB4_PPROC_HELPER_H
#define LAB4_PPROC_HELPER_H

#include <vector>

#define PPROC_N_TAPS 100

float firFilter(float sample, float* circBuf, int & circBufIdx);
void find_peaks(float* bufferIn, float* m1, float* m2, float* m3, float* m4, float* m5, float* m6);
double peak_rundown(float* m);
void create_pitch_matrix(double* curPPE, double* prevPPE_1, double* prevPPE_2, double* peMatrix);
double calculate_ppe_winner(double* peMatrix, double prevWinner);

#endif //LAB4_PPROC_HELPER_H
