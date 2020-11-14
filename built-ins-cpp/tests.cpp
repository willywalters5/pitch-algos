#include <iostream>
#include <string>
#include <vector> // could also probably use a queue

#include "matplotlibcpp.h"
#include "numpy_scipy_funcs.h"

using namespace std;
namespace plt = matplotlibcpp;

static void _idx_to_ptsY(int signalLength, vector<double> & inputSignal, vector<int> & inputIdxs, vector<double> & output);
static void _idx_to_ptsX(int signalLength, vector<double> & output);

void test_argrelextrema(bool showPlots) {
    vector<int> max_idxs;
    vector<int> min_idxs;
    vector<double> maxPtsY;
    vector<double> minPtsY;
    vector<double> maxPtsX;
    vector<double> minPtsX;
    vector<double> mySignalVect = {1,2,3,1,4,2};
    unsigned int i;

    cout << "---------------------------------------------" << endl;
    cout << "C++ Version of scipy.signal.argrelextrema" << endl;
    cout << "---------------------------------------------" << endl;
    cout << "Input Signal: {";
    for(i = 0; i < mySignalVect.size()-1; i++) {
        cout << mySignalVect[i] << ", ";
    }
    cout << mySignalVect[i] << "}" << endl;

    signal_argrelextrema(mySignalVect, "<=", max_idxs);
    signal_argrelextrema(mySignalVect, ">=", min_idxs);

    cout << "Max indices: {";
    cout << max_idxs[0] << ", ";
    for(i = 1; i < max_idxs.size()-1; i++) {
        cout << max_idxs[i] << ", ";
    }
    if(max_idxs.size() > 1)
        cout << max_idxs[i] << "}" << endl;

    cout << "Min indices: {";
    i = 0;
    if(min_idxs.size() > 1) {
        cout << min_idxs[0] << ", ";
        for(i = 1; i < min_idxs.size()-1; i++) {
        cout << min_idxs[i] << ", ";
        }
    }
    cout << min_idxs[i] << "}" << endl;
    

    if(showPlots) { 
        plt::figure(1);
        plt::subplot(1,2,1);
        plt::title("Maxes using argrelextrema");
        plt::plot(mySignalVect);
        _idx_to_ptsY(6, mySignalVect, max_idxs, maxPtsY);
        _idx_to_ptsX(6, maxPtsX);
        plt::plot(max_idxs, maxPtsY, "o");

        plt::subplot(1,2,2);
        plt::title("Mins using argrelextrema");
        plt::plot(mySignalVect);
        _idx_to_ptsY(6, mySignalVect, min_idxs, minPtsY);
        _idx_to_ptsX(6, minPtsX);
        plt::plot(min_idxs, minPtsY, "o");

        plt::show();
    }

    return;
}

void test_nonzero() {
    unsigned int i;
    vector<double> mySignal = {1,0,0,3,0,.26,-1,.0003,0,0};
    vector<double> nonzerSignal;

    cout << "---------------------------------------------" << endl;
    cout << "C++ Version of numpy.nonzero" << endl;
    cout << "---------------------------------------------" << endl;
    cout << "Input Signal: {";
    for(i = 0; i < mySignal.size()-1; i++) {
        cout << mySignal[i] << ", ";
    }
    cout << mySignal[i] << "}" << endl;

    numpy_nonzero(mySignal, nonzerSignal);

    cout << "Nonzero Signal values: {";
    cout << nonzerSignal[0] << ", ";
    for(i = 1; i < nonzerSignal.size() - 1; ++i) {
        cout << nonzerSignal[i] << ", ";
    }
    cout << nonzerSignal[i] << "}" << endl; // Is it i or i+1 

    return;
}

///@note could also get array from python and subtract difference/calculate 'error'
void test_kaiserWin() {
    vector<double> myWindow;
    
    numpy_kaiser((int)(48000*.042), 1.75, myWindow);
    
    plt::figure(2);
    plt::title("C++ Kaiser window, beta=1.75");
    plt::xlabel("Samples");
    plt::ylabel("Magnitude");
    plt::plot(myWindow);
    plt::show();

    return;
}

/* Helpers */

///@note we assume input to be sorted
///@note this is overly complicated for what it is actually doing
static void _idx_to_ptsY(int signalLength, vector<double> & inputSignal, vector<int> & inputIdxs, vector<double> & output) {
    int inputIdx = 0;

    for(int i = 0; i < signalLength; i++) {
        if(inputIdxs[inputIdx] == i){
            output.push_back(inputSignal[i]);
            ++inputIdx;
        }
    }

    return;
}

///@todo delete
static void _idx_to_ptsX(int signalLength, vector<double> & output) {
    for(int i = 0; i < signalLength; i++) {
        output.push_back(i);
    }

    return;
}
