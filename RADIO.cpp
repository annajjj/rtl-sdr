// RADIO.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <string>
#include <fstream>
#include <complex>
#include <cmath>
#include<SFML/Audio.hpp>

#define PI 3.14159265359
#define FS  2480000
#define FC 103900000
#define BWSERV 256000
#define BWAUDIO 32000

using namespace std;

const double SIZE = 5e5;
const int BUFF_SIZE = SIZE*BWAUDIO / FS + 1;
double* ym = new double[SIZE*BWSERV / FS];
fstream file;
sf::Int16* tempSamples = new sf::Int16[BUFF_SIZE];
bool isTempSamplesLoaded = false;
sf::Int16* tempZeros = new sf::Int16[BUFF_SIZE];

//alokacje pamiêci
char* samples = new char[2 * SIZE];
complex < double>* wideband_signal = new complex<double>[SIZE];
complex < double>* wideband_signal_shifted = new complex<double>[SIZE];
complex < double>* x = new complex<double>[SIZE*BWSERV / FS];
complex < double> dx;
double* y = new double[SIZE / FS* BWSERV];
double* ym2 = new double[double(SIZE)*BWAUDIO / FS + 1];
sf::Int16* raw = new sf::Int16[BUFF_SIZE];


complex<double>* lowpassFilter(complex<double>* data) {
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

sf::Int16* getNextPortionOfData()
{
	file.read(samples, 2 * SIZE);

	for (int i = 0; i < SIZE; i++) wideband_signal[i] = complex<double>(samples[i * 2], samples[i * 2 + 1]);

	for (int k = 0; k < SIZE; k++) wideband_signal_shifted[k] = wideband_signal[k] * exp(-2 * PI*FC / FS * k * 1i);


	complex < double>* wideband_signal_filtered = lowpassFilter(wideband_signal_shifted);


	for (int i = 0, j = 0; i < SIZE*BWSERV / FS; i++) {
		j = round(i*(double)FS / BWSERV);
		x[i] = wideband_signal_filtered[j];
	}

	for (int i = 0; i < SIZE*BWSERV / FS; i++) {
		dx = x[i + 1] * conj(x[i]);
		y[i] = atan2(dx.imag(), dx.real());
	}

	double* ym1 = lowpassFilterCzebyshev(y);

	for (uint64_t i = 0, j = 0; i < SIZE*BWAUDIO / FS + 1; i++) {
		j = i * BWSERV / BWAUDIO;
		ym2[i] = ym1[j];
		raw[i] = ym2[i] * 10000;
	}
	return raw;
}

class MyStream : public sf::SoundStream
{
public:

	void load()
	{
		initialize(1, BWAUDIO);
	}

private:

	virtual bool onGetData(Chunk& data)
	{
		data.sampleCount = BUFF_SIZE;
		if (isTempSamplesLoaded)
		{
			data.samples = tempSamples;
			cout << "NEED MORE SAMPLES..." << endl;
			isTempSamplesLoaded = false;
		}
		else data.samples = tempZeros;
		return true;
	}

	virtual void onSeek(sf::Time timeOffset){}
};

int main()
{
	file.open("fm_fo-104M_fs-2.48M_g-36.4.raw", std::ios::in | std::ios::out);

	if (file.good()) {
		
		// initialize and play our custom stream
		MyStream stream;
		stream.load();
		stream.play();

		// let it play until it is finished
		/*while (stream.getStatus() == MyStream::Playing)
			sf::sleep(sf::seconds(0.1f));*/

		while (1)
		{
			if (!isTempSamplesLoaded)
			{
				tempSamples = getNextPortionOfData();
				isTempSamplesLoaded = true;
				cout << "SAMPLES LOADED..." << endl;
			}
		}
	}
	else cout << "can't open file" << endl;
}
