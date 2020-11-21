//
// Created by Billy Walters on 11/18/2020.
//
//#include "../../../../../../../../../../../AndroidStudio/ndk/21.3.6528147/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/include/c++/v1/string" // <string>
//#include "../../../../../../../../../../../AndroidStudio/ndk/21.3.6528147/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/include/c++/v1/vector" // <vector>
#ifndef LAB4_NUMPY_SCIPY_FUNCS_H
#define LAB4_NUMPY_SCIPY_FUNCS_H

#include <string>
#include <vector>

///@todo function headers
//int signal_argrelextrema(float* signal, int sigLen, std::string eval, std::vector<int> & output);
//void signal_relextrema(float* signal, int sigLength, std::string eval, std::vector<float> & output);
//void numpy_nonzero(std::vector<float> & signal, std::vector<float> & output);
void numpy_nonzero_idx(float* signal, int sigLength, std::vector<int> & output);
void numpy_kaiser(unsigned int M, double beta, std::vector<double> & win);
int numpy_argmax(int* arr, int len);
//int numpy_argmin(int* arr, int len);

class kaiserWinObj {
    public:
        kaiserWinObj(int M, double beta);
        double getKaiserCoef(int idx);
    private:
        std::vector<double> window;

};

#endif //LAB4_NUMPY_SCIPY_FUNCS_H
