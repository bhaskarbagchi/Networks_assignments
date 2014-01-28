/*
 * Assignment 2
 * FDM with FFT
 */

//Includes
#include <iostream>
#include <cmath>
#include <vector>

//Definitions
#define NO_OF_SINUSOIDS 5
#define NO_OF_SAMPLING_POINTS 10000

using namespace std;

//Function Prototypes
void generateWaveform(double fFrom, double* freq, double* amp, double* val, double* time);


//Main
int main(int arcg, cahr* argv[]){
  double *ampWave1, *freqWave1, *ampWave2, *freqWave2;
  double *valWave1, *timeWave1, *valWave2, *timeWave2;
  // First band from 6kHz to 10 kHz
  generateWaveform(6.0, freqWave1, ampWave1, valWave1, timeWave1);
  // Second band from 15 kHz to 19 kHz
  generateWaveform(15.0, freqWave2, ampWave2, valWave2, timeWave2);
  
  return 0;
}

//Function definitions

void generateWaveform(double fFrom, double* freq, double* amp, double* val, double* time){
        int i, j;

        MIN_FREQUENCY = MAX_FREQUENCY = fs[1] = (fmax - 1.0) * ((double) rand() / (RAND_MAX)) + 1.0;
        As[1] = (Amax - 1.0) * ((double) rand() / (RAND_MAX)) + 1.0;

        for (i = 2; i <= NO_WAVEFORMS; ++i)
        {
                fs[i] = (fmax - 1.0) * ((double)rand() / (RAND_MAX)) + 1.0;
                As[i] = ( Amax / NO_WAVEFORMS - 1.0) * ((double)rand() / (RAND_MAX)) + 1.0;

                if (fs[i] > MAX_FREQUENCY) MAX_FREQUENCY = fs[i];
                else if (fs[i] < MIN_FREQUENCY) MIN_FREQUENCY = fs[i];
        }

        for (i = 1; i <= NO_WAVEFORMS; ++i)
        {
                for (j = 0; j <= FREQUENCY_GENERATION; ++j)
                {
                        t[i][j] = ( (double) j * 2.0 / (FREQUENCY_GENERATION * MIN_FREQUENCY));
                        F[i][j] = As[i] * sinf(2.0 * PI * fs[i] * t[i][j]);
                }
        }

        for (j = 0; j <= FREQUENCY_GENERATION; ++j)
        {
                t[0][j] = ( (double) j * 2.0 / (FREQUENCY_GENERATION * MIN_FREQUENCY));
                F[0][j] = F[1][j] + F[2][j] + F[3][j] + F[4][j] + F[5][j];
        }
}

double* MyDFT(double* a, int size) {		// recursive function 
	int n = size;							// size of data given by vector a
	if(n == 1) {
		return a;							// deepest level
	}
	double arg = ((2.0 * PI)/n);			// argument of e
	complex<double> wn(cos(arg), sin(arg)); // wn - n complex square root of -1
	complex<double> w(1.0, 0.0);
	double* a0 = (double *)malloc(n/2 * sizeof(double));// from now on we divide a on 2 matrices
	int licz = 0;							// 1 - where n are odd
	for(int v = 0; v < n; v += 2) {			// 2 - where n are even
		a0[licz] = a[v];					// n is given by:
		licz++;								// W(x) = a_0 + a_1*x + ... + a_n*x^n
	}										// A_0[x] = a_0 + a_2*x + ... + a_(n-2)*x^(n/2)-1
											// A_1[x] = a_1 + a_3*x + ... + a_(n-1)*x^(n/2)-1
	double* a1 = (double *)malloc(n/2 * sizeof(double));
	licz = 0;
	for(int h = 1; h < n; h += 2) {
		a1[licz] = a[h];
		licz++;
	}										//	end of division
	double* y0 = (double *)malloc(n/2 * sizeof(double));
	y0 = MyDFT(a0, n/2);					// recursive getting y0 vector
	double* y1 = (double *)malloc(n/2 * sizeof(double));
	y1 = MyDFT(a1, n/2);					// -----------|| --- y1 -------
	double* cmplx = (double *)malloc(n * sizeof(double));				// that's where ale conversion come
	for(int k = 0; k < (n/2); k++) {		// merge vector A_0 and A_1
		cmplx[k] = y0[k] + real(w*y1[k]);			// A[x] = A_0[x^2] + x*A_1[x^2]
		cmplx[k + (n/2)] = y0[k] - real(w*y1[k]);	
		w = w*wn;
	}
	return cmplx;								// return complex number
}


double* MyIDFT(double* a, int size) {		// recursive function 
	int n = size;							// size of data given by vector a
	if(n == 1) {
		return a;							// deepest level
	}
	double arg = ((2.0 * PI)/n);			// argument of e
	complex<double> wn(cos(arg), sin(arg)); // wn - n complex square root of -1
	complex<double> w(-1.0, 0.0);
	double* a0 = (double *)malloc(n/2 * sizeof(double));// from now on we divide a on 2 matrices
	int licz = 0;							// 1 - where n are odd
	for(int v = 0; v < n; v += 2) {			// 2 - where n are even
		a0[licz] = a[v];					// n is given by:
		licz++;								// W(x) = a_0 + a_1*x + ... + a_n*x^n
	}										// A_0[x] = a_0 + a_2*x + ... + a_(n-2)*x^(n/2)-1
											// A_1[x] = a_1 + a_3*x + ... + a_(n-1)*x^(n/2)-1
	double* a1 = (double *)malloc(n/2 * sizeof(double));
	licz = 0;
	for(int h = 1; h < n; h += 2) {
		a1[licz] = a[h];
		licz++;
	}										//	end of division
	double* y0 = (double *)malloc(n/2 * sizeof(double));
	y0 = MyIDFT(a0, n/2);					// recursive getting y0 vector
	double* y1 = (double *)malloc(n/2 * sizeof(double));
	y1 = MyIDFT(a1, n/2);					// -----------|| --- y1 -------
	double* cmplx = (double *)malloc(n * sizeof(double));				// that's where ale conversion come
	for(int k = 0; k < (n/2); k++) {		// merge vector A_0 and A_1
		cmplx[k] = y0[k] + real(w*y1[k]);			// A[x] = A_0[x^2] + x*A_1[x^2]
		cmplx[k + (n/2)] = y0[k] - real(w*y1[k]);	
		w = w*wn;
	}
	return cmplx;								// return complex number
}
