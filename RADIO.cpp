// RADIO.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <string>
#include <fstream>
#include <complex>
#include <cmath>
#include <SFML/Audio.hpp>


#define PI 3.14159265359
#define FS  2480000
#define FC 103900000
#define BWSERV 256000
#define BWAUDIO 16000

using namespace std;

fstream file;
const double SIZE = 5e6;
const int BUFF_SIZE = SIZE * BWAUDIO / FS + 1;
sf::Int16* ym = new sf::Int16[2*SIZE*BWAUDIO / FS + 1];

bool isBufferUnoOrDuo = true;
bool isTempSamplesLoaded2 = false;
bool isTempSamplesLoaded = false;
sf::Int16* tempSamples = new sf::Int16[BUFF_SIZE];
sf::Int16* tempSamples2 = new sf::Int16[BUFF_SIZE];
//alokacje pami�ci
char* samples = new char[2*SIZE];
double* wideband_signal_shifted = new double[2*SIZE];
sf::Int16* raw = new sf::Int16[BUFF_SIZE];
double* y = new double[2*SIZE / FS * BWSERV];
//sf::Int16* ym2 = new sf::Int16[SIZE*BWAUDIO / FS + 1];
double *wideband_signal_filtered = new double[2*SIZE*BWSERV / FS];

//const double b1[6] = { 0.0102416486969833,	0.0252210246018771,	0.0672142705568179,	0.124880958451606,	0.175013354121922,	0.194857487141587 };
const double b1[5] = { 0.0146377165290559,	0.0441755395501676,	0.120381320135061,	0.202164446782920,	0.237281954005590 };
//const double b2[8] = { 0.00178204928431725,	0.00602911430737609,	0.0190628591984030,	0.0445805568328161,	0.0806047125159429,	0.119109490298634,	0.148826058465054,	0.160010318194913 };
const double b2[5] = { 0.0166686694468294,	0.0468703455665084,	0.121783755139877,	0.198963250023160,	0.231427959647251 };
double y_b2[14];

void lowpassFilter(double* data) {
	wideband_signal_filtered[0] = b1[0] * data[0];
	wideband_signal_filtered[1] = b1[0] * data[1];

	// do p�tli, nie i++ i j+ tyle ile trzeba �eby by�o od razu podpr�bkowanie 
	for (uint64_t i = 2, j = 0; i < 2*SIZE*BWSERV / FS; i += 2) {
		j = 2 * round(i / 2 * (double)FS / BWSERV);
		//wideband_signal_filtered[i] = b1[0] * (data[j] + data[j - 8]) + b1[1] * (data[j - 1] + data[j - 7]) + b1[2] * (data[j - 2] + data[j - 6]) + b1[3] * (data[j - 3] + data[j - 5]) + b1[4] * data[j - 4];
		wideband_signal_filtered[i] = b1[0] * (data[j] + data[j - 16]) + b1[1] * (data[j - 2] + data[j - 14]) + b1[2] * (data[j - 4] + data[j - 12]) + b1[3] * (data[j - 6] + data[j - 10]) + b1[4] * data[j - 8];
		wideband_signal_filtered[i + 1] = b1[0] * (data[j + 1] + data[j - 15]) + b1[1] * (data[j - 1] + data[j - 13]) + b1[2] * (data[j - 3] + data[j - 11]) + b1[3] * (data[j - 5] + data[j - 9]) + b1[4] * data[j - 7];
	}
}

sf::Int16* lowpassFilterCzebyshev(double* data) {
	ym[0] = 10000 * b2[0] * data[0];
	for (uint64_t i = 1, j = 0; i < 2*SIZE*BWAUDIO / FS + 1; i++) {
		j = i * BWSERV / BWAUDIO;
		//10000 razy bo to od razu zwraca probki do sfml
		ym[i] = 10000 * (b2[0] * (data[j] + data[j - 8]) + b2[1] * (data[j - 1] + data[j - 7]) + b2[2] * (data[j - 2] + data[j - 6]) + b2[3] * (data[j - 3] + data[j - 5]) + b2[4] * data[j - 4]);
	}

	return ym;
}

//void pilotReconstruction2() {
//	//może te wszystkie SIZE*BWSERV / FS + 1 i tak dalej dać do jakichś zmiennych bo to też chyba opóźnia
//	//deklaracja na górę
//	double f_pilot = 19000;
//	//const double b_pilot[21] = { 0 - 0.00104807103566828 - 0.00778722836484360 - 0.0235782392137385 - 0.0445281353362101 - 0.0554704178966375 - 0.0351407145454122	0.0280573987710758	0.120710833210739	0.204909463010863	0.238996624310128	0.204909463010863	0.120710833210739	0.0280573987710758 - 0.0351407145454122 - 0.0554704178966375 - 0.0445281353362101 - 0.0235782392137385 - 0.00778722836484360 - 0.00104807103566828	0 };
//	const double* pilotREC = new double[SIZE*BWSERV / FS];
//	//filtering
//	/*
//
//	*/
//
//	//deklaracja na górę
//	double freq = 2 * PI * f_pilot / BWSERV;
//	double* theta = new double[SIZE*BWSERV / FS + 1];
//	theta[0] = 0;
//	const double alpha = 1e-2;
//	const double beta = alpha * alpha / 4;
//
//	//PLL
//	double perr;
//	for (int i = 0; i < SIZE*BWSERV / FS; i++) {
//		perr = -pilotREC[i] * sin(theta[i]);
//		theta[i + 1] = theta[i] + freq + alpha*perr;
//		freq = freq + beta*perr;
//	}
//	//mode typ auto
//	auto pilot2x = [](int i)->double {return cos(2 * theta[SIZE*BWSERV / FS + 1]); };
//
//
//
//
//}

sf::Int16* getNextPortionOfData()
{
	file.read(samples, SIZE);

	for (int k = 0, i = 0; k < 2*SIZE; k += 2, i++) {
		wideband_signal_shifted[k] = samples[k] * cos(2 * i*PI*FC / FS) - samples[k + 1] * sin(-2 * i*PI*FC / FS);
		wideband_signal_shifted[k + 1] = samples[k] * sin(-2 * i*PI*FC / FS) + samples[k + 1] * cos(2 * i*PI*FC / FS);
	}

	lowpassFilter(wideband_signal_shifted);

	for (int i = 0, j = 0; i < 2*SIZE*BWSERV / FS - 3; i += 2, j++) {
		y[j] = atan2(
			wideband_signal_filtered[i + 3] * wideband_signal_filtered[i] - wideband_signal_filtered[i + 2] * wideband_signal_filtered[i + 1],
			wideband_signal_filtered[i + 2] * wideband_signal_filtered[i] + wideband_signal_filtered[i + 1] * wideband_signal_filtered[i + 3]
		);
	}

	raw = lowpassFilterCzebyshev(y);


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