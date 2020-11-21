//
// Created by Billy Walters on 11/18/2020.
//

#include "numpy_scipy_funcs.h"
//#include "../../../../../../../../../../../AndroidStudio/ndk/21.3.6528147/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/include/c++/v1/string" // <string>
//#include "../../../../../../../../../../../AndroidStudio/ndk/21.3.6528147/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/include/c++/v1/vector" // <vector>
//#include "../../../../../../../../../../../AndroidStudio/ndk/21.3.6528147/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/include/c++/v1/cmath" // <cmath>
#include <string>
#include <vector>
#include <cmath>

using namespace std;

///@note could be templated
//static bool compareOp(string eval, int a, int b);
static double _zeroethOrderBessel( double x );
///@todo namespaces?
/*
///@note output is 0 if success, !=0 if failure
///@note no checking for invalid inputs for now
int signal_argrelextrema(float* signal, int sigLength, string eval, vector<int> & output) {
    // Find the extrema
    int i;
    for(i = 1; i < sigLength - 1; i++) {
        if(compareOp(eval, signal[i-1], signal[i]) && compareOp(eval, signal[i+1], signal[i]))
            output.push_back(i);
    }

    return 0;
}

///@note non index based version of argrelextrema, includes zeros to maintain sigLength
///@note assumes output is already zeroed at length sigLength
///@todo delete whatever isn't used
void signal_relextrema(float* signal, int sigLength, string eval, vector<float> & output) {
    int i;
    if(eval == "<=") {
        for(i = 1; i < sigLength - 1; i++) {
            if(signal[i-1] <= signal[i] && signal[i+1] <= signal[i])
                output[i] = signal[i];
        }
    }
    else if(eval == ">=") {
        for(i = 1; i < sigLength - 1; i++) {
            if(signal[i-1] >= signal[i] && signal[i+1] >= signal[i])
                output[i] = signal[i];
        }
    }
    else if(eval == "<") {
        for(i = 1; i < sigLength - 1; i++) {
            if(signal[i-1] < signal[i] && signal[i+1] < signal[i])
                output[i] = signal[i];
        }
    }
    else if(eval == ">") {
        for(i = 1; i < sigLength - 1; i++) {
            if(signal[i-1] > signal[i] && signal[i+1] > signal[i])
                output[i] = signal[i];
        }
    }

    return;
}

///@note np.nonzero returns indices, this does not (hopefully that is fine)
void numpy_nonzero(vector<float> & signal, vector<float> & output) {
    for(unsigned int i = 0; i < signal.size(); ++i) {
        if(signal[i] != 0)
            output.push_back(signal[i]);
    }

    return;
}
*/
///@todo use float* for input
void numpy_nonzero_idx(float* signal, int sigLength, vector<int> & output) {
    for(unsigned int i = 0; i < sigLength; ++i) {
        if(signal[i] != 0)
            output.push_back(i);
    }

    return;
}

///@note could generate table on startup and then just grab using idx for max efficiency
///@note could also generate one val at a time (less efficient)
///@note kaiser cpp code will be taken (and modified) from Loris (by Fitz and Haken)
/* https://github.com/johnglover/simpl/blob/master/src/loris/KaiserWindow.C */
void numpy_kaiser(unsigned int M, double beta, vector<double> & win) {
    //  Pre-compute the shared denominator in the Kaiser equation.
    const double oneOverDenom = 1.0 / _zeroethOrderBessel( beta );

    //const unsigned int N = win.size() - 1;
    const double oneOverN = 1.0 / M;

    for ( unsigned int n = 0; n <= M; ++n )
    {
        const double K = (2.0 * n * oneOverN) - 1.0;
        const double arg = sqrt( 1.0 - (K * K) );

        win.push_back( _zeroethOrderBessel( beta * arg ) * oneOverDenom );
    }

    return;
}

///@note no need to optimize, arr is 6 elements for our uses
int numpy_argmax(int* arr, int len) {
    int max = 0;

    for(int i = 0; i < len; ++i) {
        if(arr[i] > arr[max])
            max = i;
    }

    return max;
}

/*
///@todo may delete if not used
int numpy_argmin(int* arr, int len) {
    int min = 999;

    for(int i = 0; i < len; ++i) {
        if(arr[i] < arr[min])
            min = i;
    }

    return min;
}
*/
/************************************* Helper Functions ******************************************/

/*
///@note would be even better if you could just do this check once then always use it for the rest of the func.
static bool compareOp(string eval, int a, int b) {
    if(eval.length() < 1 || eval.length() > 2)
        return false;
    else if(eval.length() == 1) {
        if(eval[0] == '>')
            return a > b;
        else if(eval[0] == '<')
            return a < b;
        else
            return false;
    }
    else { // eval.length() == 2
        if(eval[0] == '>' && eval[1] == '=')
            return a >= b;
        else if (eval[0] == '<' && eval[1] == '=')
            return a <= b;
        else
            return false;
    }
}
 */

/* https://github.com/johnglover/simpl/blob/master/src/loris/KaiserWindow.C */
static double _zeroethOrderBessel( double x )
{
    const double eps = 0.000001;

    //  initialize the series term for m=0 and the result
    double besselValue = 0;
    double term = 1;
    double m = 0;

    //  accumulate terms as long as they are significant
    while(term  > eps * besselValue)
    {
        besselValue += term;

        //  update the term
        ++m;
        term *= (x*x) / (4*m*m);
    }

    return besselValue;
}

kaiserWinObj::kaiserWinObj(int M, double beta) {
    numpy_kaiser(M, beta, this->window);
}

double kaiserWinObj::getKaiserCoef(int idx) {
    if(idx < 0 || idx > this->window.size())
            return -1;
    ///@todo check if at adds redundant check
    return this->window.at(idx);
}
