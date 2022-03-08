// POKEY Frequencies Calculator
// Most of the global stuff will be defined here

#pragma once

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <fstream>
#include <time.h>
#include <cmath>
#include <limits>

#define VERSION "v0.1"
#define FREQ_17_NTSC 1789773
#define FREQ_17_PAL 1773447
#define SEPARATOR cout << endl << "----------------------------------------------------" << endl

int main();
void wait(int sec);
void real_freq();
void generate_freq();
void generate_table(int note, double freq, int distortion, bool IS_15KHZ, bool IS_179MHZ, bool IS_16BIT);
int get_audf(double freq, int coarse_divisor, double divisor, int modoffset);
int delta_audf(int audf, double freq, int coarse_divisor, double divisor, int modoffset, int distortion);
double get_tuning();
int get_region();
template <typename T> void read_input(T* output, std::string prompt);
