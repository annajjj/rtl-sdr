// RADIO.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include <iostream>
#include <string>
#include <fstream>
#include <cmath>
#include <complex>

#define PI 3.14159265359
#define FS  2480000
#define FC 103900000
#define BWSERV 256000
#define BWAUDIO 32000

using namespace std;

const uint32_t SIZE = 3e5;
double* ym = new double[SIZE*BWSERV / FS]; 

/*class Rtl {

public:
	double fs;
	double fc;
	double bwSERV;
	double bwAUDIO;
	int size;
	char* samples;

	Rtl(double fs, double fc, double bwSERV, double bwAUDIO, int size) {
		this->fs = fs;
		this->fc = fc;
		this->bwSERV = bwSERV;
		this->bwAUDIO = bwAUDIO;
		this->size = size;
	}
};*/



complex<double>* lowpassFilter(complex<double> data[SIZE]) {
	const double a[5] = { 1.0, -3.2070, 3.9205, -2.1578, 0.4502 };
	const double b[5] = { 0.0004, 0.0015, 0.0022, 0.0015, 0.0004 };
	static complex<double> *wideband_signal_filtered = new complex<double>[SIZE];

	wideband_signal_filtered[0] = b[0] * data[0];
	wideband_signal_filtered[1] = b[0] * data[1] + b[1] * data[0] - a[1] * wideband_signal_filtered[0];
	wideband_signal_filtered[2] = b[0] * data[2] + b[1] * data[1] + b[2] * data[0] - a[1] * wideband_signal_filtered[1] - a[2] * wideband_signal_filtered[0];
	wideband_signal_filtered[3] = b[0] * data[3] + b[1] * data[2] + b[2] * data[1] + b[3] * data[0] - a[1] * wideband_signal_filtered[2] - a[2] * wideband_signal_filtered[1] - a[3] * wideband_signal_filtered[0];
	wideband_signal_filtered[4] = b[0] * data[4] + b[1] * data[3] + b[2] * data[2] + b[3] * data[1] + b[4] * data[0] - a[1] * wideband_signal_filtered[3] - a[2] * wideband_signal_filtered[2] - a[3] * wideband_signal_filtered[1] - a[4] * wideband_signal_filtered[0];

	for (int i = 5; i < SIZE; i++) wideband_signal_filtered[i] = b[0] * data[i] + b[1] * data[i - 1] + b[2] * data[i - 2] + b[3] * data[i - 3] + b[4] * data[i - 4] - a[1] * wideband_signal_filtered[i - 1] - a[2] * wideband_signal_filtered[i - 2] - a[3] * wideband_signal_filtered[i - 3] - a[4] * wideband_signal_filtered[i - 4];

	return wideband_signal_filtered;
}

/*double* lowpassFilterCzebyshev(double data[SIZE*BWSERV / FS]) {
	const double a[9] = { 1.0000, -7.2117, 22.9518, -42.0901, 48.6330, -36.2476, 17.0156, -4.5989, 0.5479};
	double b[9] = { 0.0020, 0.0162,  0.0567,   0.1134,   0.1417,   0.1134,  0.0567,  0.0162, 0.0020};
	for (int i = 0; i < 9; i++) b[i] = b[i] / 100000;
	static double ym[SIZE*BWSERV/FS];

	ym[0] = b[0] * data[0];
	ym[1] = b[0] * data[1] + b[1] * data[0] - a[1] * ym[0];
	ym[2] = b[0] * data[2] + b[1] * data[1] + b[2] * data[0] - a[1] * ym[1] - a[2] * ym[0];
	ym[3] = b[0] * data[3] + b[1] * data[2] + b[2] * data[1] + b[3] * data[0] - a[1] * ym[2] - a[2] * ym[1] - a[3] * ym[0];
	ym[4] = b[0] * data[4] + b[1] * data[3] + b[2] * data[2] + b[3] * data[1] + b[4] * data[0] - a[1] * ym[3] - a[2] * ym[2] - a[3] * ym[1] - a[4] * ym[0];
	ym[5] = b[0] * data[5] + b[1] * data[4] + b[2] * data[3] + b[3] * data[2] + b[4] * data[1] + b[5] * data[0]- a[1] * ym[4] - a[2] * ym[3] - a[3] * ym[2] - a[4] * ym[1] - a[5] * ym[0];
	ym[6] = b[0] * data[6] + b[1] * data[5] + b[2] * data[4] + b[3] * data[3] + b[4] * data[2] + b[5] * data[1] + b[6] * data[0] - a[1] * ym[5] - a[2] * ym[4] - a[3] * ym[3] - a[4] * ym[2] - a[5] * ym[1] - a[6] * ym[0];
	ym[7] = b[0] * data[7] + b[1] * data[6] + b[2] * data[5] + b[3] * data[4] + b[4] * data[3] + b[5] * data[2] + b[6] * data[1] + b[7] * data[0] - a[1] * ym[6] - a[2] * ym[5] - a[3] * ym[4] - a[4] * ym[3] - a[5] * ym[2] - a[6] * ym[1] - a[7] * ym[0];
	//ym[8] = b[0] * data[8] + b[1] * data[7] + b[2] * data[6] + b[3] * data[5] + b[4] * data[4] + b[5] * data[3] + b[6] * data[2] + b[7] * data[1] + b[8] * data[0] - a[1] * ym[7] - a[2] * ym[6] - a[3] * ym[5] - a[4] * ym[4] - a[5] * ym[3] - a[6] * ym[2] - a[7] * ym[1] - a[8] * ym[0];

	for (int i = 8; i < SIZE*BWSERV / FS; i++) ym[i] = b[0] * data[i] + b[1] * data[i - 1] + b[2] * data[i - 2] + b[3] * data[i - 3] + b[4] * data[i - 4] + b[5] * data[i - 5] + b[6] * data[i - 6] + b[7] * data[i - 7] + b[8] * data[i - 8] - a[1] * ym[i - 1] - a[2] * ym[i - 2] - a[3] * ym[i - 3] - a[4] * ym[i - 4] - a[5] * ym[i - 5] - a[6] * ym[i - 6] - a[7] * ym[i -7] - a[8] * ym[i - 8];
	//cout << ym[0] << endl << ym[1] << endl << ym[2] << endl << ym[3] << endl << ym[4]<<endl<<ym[5]<<endl<<ym[6]<<endl<<ym[7]<<endl<<ym[8];
	//cout << ym[8] << endl << ym[9] << endl << ym[10] << endl << ym[11] << endl << ym[12]<<endl<<ym[13]<<endl<<ym[14]<<endl<<ym[15]<<endl<<ym[16];
	return ym;
}*/

double* lowpassFilterCzebyshev(double* data) {
	double a[5] = { 1, -3.26190114006689, 4.11211754150723, -2.36210053054586,	0.520159946121643 };
	double b[5] = { 0.000514269652404351,	0.00205707860961740,	0.00308561791442611,	0.00205707860961740,	0.000514269652404351 };

	ym[0] = b[0] * data[0];
	ym[1] = b[0] * data[1] + b[1] * data[0] - a[1] * ym[0];
	ym[2] = b[0] * data[2] + b[1] * data[1] + b[2] * data[0] - a[1] * ym[1] - a[2] * ym[0];
	ym[3] = b[0] * data[3] + b[1] * data[2] + b[2] * data[1] + b[3] * data[0] - a[1] * ym[2] - a[2] * ym[1] - a[3] * ym[0];
	ym[4] = b[0] * data[4] + b[1] * data[3] + b[2] * data[2] + b[3] * data[1] + b[4] * data[0] - a[1] * ym[3] - a[2] * ym[2] - a[3] * ym[1] - a[4] * ym[0];

	for (int i = 5; i < SIZE*BWSERV / FS; i++) ym[i] = b[0] * data[i] + b[1] * data[i - 1] + b[2] * data[i - 2] + b[3] * data[i - 3] + b[4] * data[i - 4] - a[1] * ym[i - 1] - a[2] * ym[i - 2] - a[3] * ym[i - 3] - a[4] * ym[i - 4];
	return ym;
}

int main()
{
	fstream file;
	file.open("fm_fo-104M_fs-2.48M_g-36.4.raw", std::ios::in | std::ios::out);

	if (file.good()) {
		char* samples = new char[2 * SIZE];
		file.read(samples, 2 * SIZE);

		complex < double>* wideband_signal = new complex<double>[SIZE];
		for (int i = 0; i < SIZE; i++) wideband_signal[i] = complex<double>(samples[i * 2], samples[i * 2 + 1]);

		complex < double>* wideband_signal_shifted = new complex<double>[SIZE];
		for (int k = 0; k < SIZE; k++) wideband_signal_shifted[k] = wideband_signal[k] * exp(-2 * PI*FC / FS * k * 1i);


		complex < double>* wideband_signal_filtered = lowpassFilter(wideband_signal_shifted);

		complex < double>* x = new complex<double>[SIZE*BWSERV / FS];

		for (int i = 0, j = 0; i < SIZE*BWSERV / FS; i++) {
			j = round(i*(double)FS / BWSERV);
			x[i] = wideband_signal_filtered[j];
		}

		complex < double> dx;
		double* y = new double[SIZE*BWSERV / FS];

		for (int i = 0; i < SIZE*BWSERV / FS; i++) {
			dx = x[i + 1] * conj(x[i]);
			y[i] = atan2(dx.imag(), dx.real());
		}

		double* ym1 = lowpassFilterCzebyshev(y);

		cout << ym1[50] << endl; 

		double* ym2 = new double[SIZE*BWAUDIO / FS + 1];

		for (int i = 0, j = 0; i < SIZE*BWAUDIO / FS + 1; i++) {
			j = i * BWSERV / BWAUDIO;
			ym2[i] = ym1[j];
		}

		cout << ym2[0] << endl << ym2[1] << endl << ym2[40] << endl << SIZE * BWAUDIO / FS;
		
	}
	else cout << "can't open file" << endl;

}
