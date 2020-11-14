#include <iostream>
#include <string>
#include <vector> // could also probably use a queue
#include <cmath>

using namespace std;

///@note could be templated
static bool compareOp(std::string eval, int a, int b);
static double _zeroethOrderBessel( double x );

///@note output is 0 if success, !=0 if failure
///@note no checking for invalid inputs for now
int signal_argrelextrema(vector<double> & signal, string eval, vector<int> & output) {
    // Parse eval argument
    /*
    ///@note can be optimized by using branchless checking
    if(eval.length() < 1 || eval.length() > 2 || signal == NULL)
        return NULL;
    else if(eval.length() == 1) {
        if(eval[0] == '>')
            // Use >
            ;
        else if (eval[0] == '<')
            // Use <
            ;
        else
            return NULL;
    }
    else { // eval.length() == 2
        if(eval[0] == '>' && eval[1] == '=')
            // Use >=
            ;
        else if (eval[0] == '<' && eval[1] == '=')
            // Use <=
            ;
        else
            return NULL;
    } 
    */

    // Find the extrema
    ///@todo figure out how I want to get this    
    int sigLength = 6;
    int i;
    for(i = 1; i < sigLength - 1; i++) {
        if(compareOp(eval, signal[i-1], signal[i]) && compareOp(eval, signal[i+1], signal[i]))
            output.push_back(i);
    } 

    return 0;
}

///@note np.nonzero returns indices, this does not (hopefully that is fine)
void numpy_nonzero(std::vector<double> & signal, std::vector<double> & output) {
    for(unsigned int i = 0; i < signal.size(); ++i) {
        if(signal[i] != 0)
            output.push_back(signal[i]);
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

/******* Helpers ********/


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
