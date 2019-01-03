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
#define BWAUDIO 16000

using namespace std;

const double SIZE = 5e6;
const int BUFF_SIZE = SIZE*BWAUDIO / FS + 1;
sf::Int16* ym = new sf::Int16[SIZE*BWAUDIO / FS + 1];
fstream file;
sf::Int16* tempSamples = new sf::Int16[BUFF_SIZE];
sf::Int16* tempSamples2 = new sf::Int16[BUFF_SIZE];
bool isBufferUnoOrDuo = true;
bool isTempSamplesLoaded2 = false;
bool isTempSamplesLoaded = false;
sf::Int16* tempZeros = new sf::Int16[10];

//alokacje pamiêci
char* samples = new char[2 * SIZE];
complex < double>* wideband_signal = new complex<double>[SIZE];
complex < double>* wideband_signal_shifted = new complex<double>[SIZE];
//complex < double>* x = new complex<double>[SIZE*BWSERV / FS];
complex < double> dx;
double* y = new double[SIZE / FS* BWSERV];
//sf::Int16* ym2 = new sf::Int16[SIZE*BWAUDIO / FS + 1];
complex<double> *wideband_signal_filtered = new complex<double>[SIZE*BWSERV / FS];
sf::Int16* raw = new sf::Int16[BUFF_SIZE];

//const double b1[6] = { 0.0102416486969833,	0.0252210246018771,	0.0672142705568179,	0.124880958451606,	0.175013354121922,	0.194857487141587 };
const double b1[5] = { 0.0146377165290559,	0.0441755395501676,	0.120381320135061,	0.202164446782920,	0.237281954005590 };
//const double b2[8] = { 0.00178204928431725,	0.00602911430737609,	0.0190628591984030,	0.0445805568328161,	0.0806047125159429,	0.119109490298634,	0.148826058465054,	0.160010318194913 };
const double b2[5] = { 0.0166686694468294,	0.0468703455665084,	0.121783755139877,	0.198963250023160,	0.231427959647251 };
complex<double> y_b1[10];
double y_b2[14];

complex<double>* lowpassFilter(complex<double>* data) {
	const double a[5] = { 1.0, -3.2070, 3.9205, -2.1578, 0.4502 };
	const double b[5] = { 0.0004, 0.0015, 0.0022, 0.0015, 0.0004 };

	// wiem wiem glupio wyglda ale to siê moze przydaæ 
	y_b1[0] = b1[0] * data[0];
	/*y_b1[1] = b1[0] * data[1] + b1[1] * data[0];
	y_b1[2] = b1[0] * data[2] + b1[1] * data[1] + b1[2] * data[0];
	y_b1[3] = b1[0] * data[3] + b1[1] * data[2] + b1[2] * data[1] + b1[3] * data[0];
	y_b1[4] = b1[0] * data[4] + b1[1] * data[3] + b1[2] * data[2] + b1[3] * data[1] + b1[4] * data[0];
	y_b1[5] = b1[0] * data[5] + b1[1] * data[4] + b1[2] * data[3] + b1[3] * data[2] + b1[4] * data[1] + b1[5] * data[0];
	y_b1[6] = b1[0] * data[6] + b1[1] * data[5] + b1[2] * data[4] + b1[3] * data[3] + b1[4] * (data[2] + data[0]) + b1[5] * data[1];
	y_b1[7] = b1[0] * data[7] + b1[1] * data[6] + b1[2] * data[5] + b1[3] * (data[4] + data[0]) + b1[4] * (data[3] + data[1]) + b1[5] * data[2];*/



	wideband_signal_filtered[0] = y_b1[0];

	// do pêtli, nie i++ i j+ tyle ile trzeba ¿eby by³o od razu podpróbkowanie 
	for (uint64_t i = 1, j = 0; i < SIZE*BWSERV / FS; i++) {
		j = round(i*(double)FS / BWSERV);
		wideband_signal_filtered[i] = b1[0] * (data[j] + data[j - 8]) + b1[1] * (data[j - 1] + data[j - 7]) + b1[2] * (data[j - 2] + data[j - 6]) + b1[3] * (data[j - 3] + data[j - 5]) + b1[4] * data[j - 4];
	}


	/////////////////////////////
	/*
	wideband_signal_filtered[0] = b[0] * data[0];
	wideband_signal_filtered[1] = b[0] * data[1] + b[1] * data[0] - a[1] * wideband_signal_filtered[0];
	wideband_signal_filtered[2] = b[0] * data[2] + b[1] * data[1] + b[2] * data[0] - a[1] * wideband_signal_filtered[1] - a[2] * wideband_signal_filtered[0];
	wideband_signal_filtered[3] = b[0] * data[3] + b[1] * data[2] + b[2] * data[1] + b[3] * data[0] - a[1] * wideband_signal_filtered[2] - a[2] * wideband_signal_filtered[1] - a[3] * wideband_signal_filtered[0];
	wideband_signal_filtered[4] = b[0] * data[4] + b[1] * data[3] + b[2] * data[2] + b[3] * data[1] + b[4] * data[0] - a[1] * wideband_signal_filtered[3] - a[2] * wideband_signal_filtered[2] - a[3] * wideband_signal_filtered[1] - a[4] * wideband_signal_filtered[0];


	//policzyæ wszystkie, potem pêtla i podprobkowac najpierw te pierwsze co ileœ apotem druga pêtla koleje juz dobrze liczone !!!!!!!!!!!!!!!!!!
	for (int i = 5; i < SIZE; i++) wideband_signal_filtered[i] = b[0] * data[i] + b[1] * data[i - 1] + b[2] * data[i - 2] + b[3] * data[i - 3] + b[4] * data[i - 4] - a[1] * wideband_signal_filtered[i - 1] - a[2] * wideband_signal_filtered[i - 2] - a[3] * wideband_signal_filtered[i - 3] - a[4] * wideband_signal_filtered[i - 4];
	*/
	return wideband_signal_filtered;
}

sf::Int16* lowpassFilterCzebyshev(double* data) {
	//double a[5] = { 1, -3.26190114006689, 4.11211754150723, -2.36210053054586,	0.520159946121643 };
	//double b[5] = { 0.000514269652404351,	0.00205707860961740,	0.00308561791442611,	0.00205707860961740,	0.000514269652404351 };

	/*zmien na b2 i wideband_signal filtered
	wideband_signal_filtered[0] = b1[0] * data[0];
	wideband_signal_filtered[1] = b1[0] * data[1] + b1[1] * data[0];
	wideband_signal_filtered[2] = b1[0] * data[2] + b1[1] * data[1] + b1[2] * data[0];
	wideband_signal_filtered[3] = b1[0] * data[3] + b1[1] * data[2] + b1[2] * data[1] + b1[3] * data[0];
	wideband_signal_filtered[4] = b1[0] * data[4] + b1[1] * data[3] + b1[2] * data[2] + b1[3] * data[1] + b1[4] * data[0];
	wideband_signal_filtered[5] = b1[0] * data[5] + b1[1] * data[4] + b1[2] * data[3] + b1[3] * data[2] + b1[4] * data[1] + b1[5] * data[0];
	wideband_signal_filtered[6] = b1[0] * data[6] + b1[1] * data[5] + b1[2] * data[4] + b1[3] * data[3] + b1[4] * data[2] + b1[5] * data[1] + b[6] * data[0];
	wideband_signal_filtered[7] = b1[0] * data[7] + b1[1] * data[6] + b1[2] * data[5] + b1[3] * data[4] + b1[4] * data[3] + b1[5] * (data[2] + data[0]) + b2[6] * data[1];
	*/

	y_b2[0] = b2[0] * data[0];


	/*
	ym[0] = b[0] * data[0];
	ym[1] = b[0] * data[1] + b[1] * data[0] - a[1] * ym[0];
	ym[2] = b[0] * data[2] + b[1] * data[1] + b[2] * data[0] - a[1] * ym[1] - a[2] * ym[0];
	ym[3] = b[0] * data[3] + b[1] * data[2] + b[2] * data[1] + b[3] * data[0] - a[1] * ym[2] - a[2] * ym[1] - a[3] * ym[0];
	ym[4] = b[0] * data[4] + b[1] * data[3] + b[2] * data[2] + b[3] * data[1] + b[4] * data[0] - a[1] * ym[3] - a[2] * ym[2] - a[3] * ym[1] - a[4] * ym[0];
	*/
	ym[0] = 10000*y_b2[0];
	for (uint64_t i = 1, j = 0; i < SIZE*BWAUDIO / FS + 1; i++) {
		j = i * BWSERV / BWAUDIO;
		//10000 razy bo to od razu zwraca probki do sfml
		ym[i] = 10000*(b2[0] * (data[j] + data[j - 8]) + b2[1] * (data[j - 1] + data[j - 7]) + b2[2] * (data[j - 2] + data[j - 6]) + b2[3] * (data[j - 3] + data[j - 5]) + b2[4] * data[j - 4]);
	}
	return ym;
}

sf::Int16* getNextPortionOfData()
{
	file.read(samples, 2 * SIZE);

	for (int i = 0; i < SIZE; i++) wideband_signal[i] = complex<double>(samples[i * 2], samples[i * 2 + 1]);

	for (int k = 0; k < SIZE; k++) wideband_signal_shifted[k] = wideband_signal[k] * exp(-2 * PI*FC / FS * k * 1i);


	complex < double>* wideband_signal_filtered = lowpassFilter(wideband_signal_shifted);



	for (int i = 0; i < SIZE*BWSERV / FS; i++) {
		dx = wideband_signal_filtered[i + 1] * conj(wideband_signal_filtered[i]);
		y[i] = atan2(dx.imag(), dx.real());
	}

	//nie wiem czy rzutowanie nie opoznia tego, todo
	raw = (sf::Int16*)lowpassFilterCzebyshev(y);

	if (file.eof()) return NULL;
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

		while (!isTempSamplesLoaded && !isTempSamplesLoaded2)
		{
			//cout << "NEED MORE SAMPLES..." << endl;
			//_sleep(100);
		}
		if (isBufferUnoOrDuo && isTempSamplesLoaded)
		{
			data.samples = tempSamples;
			isTempSamplesLoaded = false;
			isBufferUnoOrDuo = false;
			cout << "PLAYING BUFFOR 1" << endl;
			return true;
		}
		if (!isBufferUnoOrDuo && isTempSamplesLoaded2)
		{
			data.samples = tempSamples2;
			isTempSamplesLoaded2 = false;
			isBufferUnoOrDuo = true;
			cout << "PLAYING BUFFOR 2" << endl;
			return true;
		}
	}

	virtual void onSeek(sf::Time timeOffset) {}
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
				if (tempSamples == NULL) break;
				isTempSamplesLoaded = true;
				cout << "SAMPLES LOADED TO BUFFOR 1" << endl;
			}
			if (!isTempSamplesLoaded2)
			{
				tempSamples2 = getNextPortionOfData();
				if (tempSamples2 == NULL) break;
				isTempSamplesLoaded2 = true;
				cout << "SAMPLES LOADED TO BUFFOR 2" << endl;
			}
			//_sleep(100);
		}
	}
	else cout << "can't open file" << endl;
}