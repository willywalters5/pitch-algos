#include <string>
#include <vector> // could also probably use a queue

#pragma once

///@todo function headers
int signal_argrelextrema(std::vector<double> & signal, std::string eval, std::vector<int> & output);
void numpy_nonzero(std::vector<double> & signal, std::vector<double> & output);
double numpy_kaiser(unsigned int M, double beta, std::vector<double> & win);