#include <iostream>
#include <string>
#include <fstream>
#include <complex>
#include <cmath>
#include<SFML/Audio.hpp>


#define PI 3.14159265359
//#define FS  2480000
//#define FC 103900000
#define FS 1200000
#define FC 104100000
#define BWSERV 256000
#define BWAUDIO 16000

using namespace std;

const double SIZE = 2e6;
const int BUFF_SIZE = SIZE*BWAUDIO / FS;
double* ym = new double[SIZE*BWAUDIO / FS + 1];
fstream file;
sf::Int16* tempSamples = new sf::Int16[BUFF_SIZE];
sf::Int16* tempSamples2 = new sf::Int16[BUFF_SIZE];
bool isBufferUnoOrDuo = true;
bool isTempSamplesLoaded2 = false;
bool isTempSamplesLoaded = false;
sf::Int16* tempZeros = new sf::Int16[10];

char* samples = new char[2 * SIZE];
complex < double>* wideband_signal = new complex<double>[SIZE];
complex < double>* wideband_signal_shifted = new complex<double>[SIZE];
complex < double> dx;
double* y = new double[SIZE / FS* BWSERV];
complex<double> *wideband_signal_filtered = new complex<double>[SIZE*BWSERV / FS];
sf::Int16* raw = new sf::Int16[BUFF_SIZE];
complex<double> y_b1[10];
double y_b2[14];

//PLL
double f_pilot = 19000;
const double b_pilot[10] = { 0, -0.00164198190541088, -0.0101184457883958, -0.0275849619486905, -0.0466555376128923, -0.0476648940777955, -0.00866486680067846,	0.0742874803336435,	0.173957680307199,	0.242528331270080 };
double* pilotREC = new double[SIZE*BWSERV / FS];

double freq = 2 * PI * f_pilot / BWSERV;
double*  theta = new double[SIZE*BWSERV / FS + 1];
const double alpha = 1e-2;
const double beta = alpha * alpha / 4;

double* esrlup = new double[SIZE*BWSERV / FS];
double* esrl = new double[SIZE*BWAUDIO / FS];
sf::Int16* esl = new sf::Int16[SIZE*BWAUDIO / FS];
sf::Int16* esr = new sf::Int16[SIZE*BWAUDIO / FS];
double perr;

uint64_t fi = 0;
complex<double>* expValue = new complex<double>[FS];
void expArrayInit() {
	double arg = 0;
	for (int p = 0; p < FS; p++) {
		arg = 1.0 * p / FC;
		expValue[p] = exp(-2 * PI * FC * arg / FS * 1i);
		
	}
}

complex<double> EXP(complex<double> x) {
	x = 1.0 + x / 1048576.0;
	x *= x; x *= x; x *= x; x *= x;
	x *= x; x *= x; x *= x; x *= x;
	x *= x; x *= x; x *= x; x *= x;
	x *= x; x *= x; x *= x; x *= x;
	x *= x; x *= x; x *= x; x *= x;
	return x;
}

// int tableSize should we convert double to int???
void SetBeginingBuffor(double* table, int count, int tableSize) {
	for (int i = 0; i < count; i++) table[i] = table[tableSize - count + i];
}

void SetBufforZeros(double* table, int count) {
	for (int i = 0; i < count; i++) table[i] = 0;
}

void SetVariables() {
	expArrayInit();
	theta[0] = 0;
	SetBufforZeros(pilotREC, 19);
	SetBufforZeros(esrlup, 20);
}

void ReadSamples() {
	file.read(samples, 2 * SIZE);
}


void SamplesToComplex() {
	for (int i = 0; i < SIZE; i++) wideband_signal[i] = complex<double>(samples[i * 2], samples[i * 2 + 1]);
}

void ShiftToBaseband() {
	for (int k = 0; k < SIZE; fi++, k++) {
		//wideband_signal_shifted[k] = wideband_signal[k] * (cos(2 * PI*FC / FS * k) - 1i*sin(2 * PI*FC / FS * k));
		//cout << expValue[(fi*FC) % FS] << "   " << exp(-2 * PI*FC / FS * k * 1i) << endl;
		//cout << (fi*FC) % FS << endl;
		wideband_signal_shifted[k] = wideband_signal[k] * expValue[(fi*FC)%FS];
	}
}

const double b1[5] = { 0.0146377165290559,	0.0441755395501676,	0.120381320135061,	0.202164446782920,	0.237281954005590 };

void LowpassFilter() {
	wideband_signal_filtered[0] = b1[0] * wideband_signal_shifted[0];

	for (uint64_t i = 1, j = 0; i < SIZE*BWSERV / FS; i++) {
		j = round(i*(double)FS / BWSERV) - 1;
		wideband_signal_filtered[i] = b1[0] * (wideband_signal_shifted[j] + wideband_signal_shifted[j - 8]) + b1[1] * (wideband_signal_shifted[j - 1] + wideband_signal_shifted[j - 7]) + b1[2] * (wideband_signal_shifted[j - 2] + wideband_signal_shifted[j - 6]) + b1[3] * (wideband_signal_shifted[j - 3] + wideband_signal_shifted[j - 5]) + b1[4] * wideband_signal_shifted[j - 4];
	}

}

void DemodulationFM() {
	for (int i = 0; i < SIZE*BWSERV / FS; i++) {
		dx = wideband_signal_filtered[i + 1] * conj(wideband_signal_filtered[i]);
		y[i] = atan2(dx.imag(), dx.real());
	}
}

const double b2[5] = { 0.0166686694468294,	0.0468703455665084,	0.121783755139877,	0.198963250023160,	0.231427959647251 };

void Mono() {
	ym[0] = b2[0] * y[0];
	for (uint64_t i = 1, j = 0; i < SIZE*BWAUDIO / FS + 1; i++) {
		j = i * BWSERV / BWAUDIO - 1;
		ym[i] = (b2[0] * (y[j] + y[j - 8]) + b2[1] * (y[j - 1] + y[j - 7]) + b2[2] * (y[j - 2] + y[j - 6]) + b2[3] * (y[j - 3] + y[j - 5]) + b2[4] * y[j - 4]);
	}
}


//PLL
void PilotReconstruction() {
	for (uint64_t i = 19, j = 0; i < SIZE*BWSERV / FS; i++) {
		pilotREC[i] = b_pilot[0] * (y[i] + y[i - 19]) + b_pilot[1] * (y[i - 1] + y[i - 18]) + b_pilot[2] * (y[i - 2] + y[i - 17]) + b_pilot[3] * (y[i - 3] + y[i - 16]) + b_pilot[4] * (y[i - 4] + y[i - 15]) + b_pilot[5] * (y[i - 5] + y[i - 14]) + b_pilot[6] * (y[i - 6] + y[i - 13]) + b_pilot[7] * (y[i - 7] + y[i - 12]) + b_pilot[8] * (y[i - 8] + y[i - 11]) + b_pilot[9] * (y[i - 9] + y[i - 10]);
	}


	//PLL

	for (int i = 0; i < SIZE*BWSERV / FS; i++) {
		perr = -pilotREC[i] * sin(theta[i]);
		theta[i + 1] = theta[i] + freq + alpha*perr;
		freq = freq + beta*perr;
	}

	SetBeginingBuffor(pilotREC, 19, SIZE*BWSERV / FS);
}

//pll theta ostatnia jest pierwsza nastepnego pierwsze elementy filtru to ostatnie :3

const double hLP[10] = { 0,	0.000809467721695811,	0.00404453781854362,	0.0113897239835690,	0.0246066750731956,	0.0444243866844004,	0.0694655711274116,	0.0959040257742370,	0.118252426340399,	0.131103185476548 }; // low pass 
const double hBP[10] = { 0, -0.000119057799757280,	0.00612314062564759,	0.0222187747664582,	0.0199368390682425, -0.0437792502661133, -0.137841764773162, -0.132216695745961,	0.0404202524384409,	0.234133430242033 }; // band pass

void LmRreconstruction() {
	for (uint64_t i = 19, j = 0; i < SIZE*BWSERV / FS; i++) {
		esrlup[i] = hBP[0] * (y[i] + y[i - 19]) + hBP[1] * (y[i - 1] + y[i - 18]) + hBP[2] * (y[i - 2] + y[i - 17]) + hBP[3] * (y[i - 3] + y[i - 16]) + hBP[4] * (y[i - 4] + y[i - 15]) + hBP[5] * (y[i - 5] + y[i - 14]) + hBP[6] * (y[i - 6] + y[i - 13]) + hBP[7] * (y[i - 7] + y[i - 12]) + hBP[8] * (y[i - 8] + y[i - 11]) + hBP[9] * (y[i - 9] + y[i - 10]);
	}


	for (int i = 0; i< SIZE*BWSERV / FS; i++) esrlup[i] = 2 * esrlup[i] * cos(2 * theta[i + 1]);
	//set first new theta value 
	theta[0] = theta[int(SIZE*BWSERV / FS)];

	for (int i = 0; i < 2; i++) esrl[i] = esrlup[i];
	for (uint64_t i = 2, j = 0; i < SIZE*BWAUDIO / FS; i++) {
		j = i * BWSERV / BWAUDIO - 1;
		// zmiec na to potem, krótszy filt ten sam co w mono//esrl[i] = (b2[0] * (esrlup[j] + esrlup[j - 8]) + b2[1] * (esrlup[j - 1] + esrlup[j - 7]) + b2[2] * (esrlup[j - 2] + esrlup[j - 6]) + b2[3] * (esrlup[j - 3] + esrlup[j - 5]) + b2[4] * esrlup[j - 4]);
		esrl[i] = hLP[0] * (esrlup[j] + esrlup[j - 19]) + hLP[1] * (esrlup[j - 1] + esrlup[j - 18]) + hLP[2] * (esrlup[j - 2] + esrlup[j - 17]) + hLP[3] * (esrlup[j - 3] + esrlup[j - 16]) + hLP[4] * (esrlup[j - 4] + esrlup[j - 15]) + hLP[5] * (esrlup[j - 5] + esrlup[j - 14]) + hLP[6] * (esrlup[j - 6] + esrlup[j - 13]) + hLP[7] * (esrlup[j - 7] + esrlup[j - 12]) + hLP[8] * (esrlup[j - 8] + esrlup[j - 11]) + hLP[9] * (esrlup[j - 9] + esrlup[j - 10]);
		//10000 razy bo to od razu zwraca probki do sfml
		esl[i] = 10000 * (ym[i] + esrl[i]) / 2;
		esr[i] = 10000 * (ym[i] - esrl[i]) / 2;
	}
	SetBeginingBuffor(esrlup, /*tu bêdzie rz¹d filtra z mono*/20, SIZE*BWSERV / FS);
}


sf::Int16* getNextPortionOfData()
{
	ReadSamples();

	SamplesToComplex();

	ShiftToBaseband();

	LowpassFilter();

	DemodulationFM();

	Mono();

	PilotReconstruction();

	LmRreconstruction();

	//raw = ym;	//mono
	raw = esr; //esl or esr
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
	SetVariables();

	file.open("fm_fo-104.4M_fs-1.2M_g-36.4.raw", std::ios::in | std::ios::out);

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