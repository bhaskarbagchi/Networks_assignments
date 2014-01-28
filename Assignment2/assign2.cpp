/* Assignment 2
 * FDM with FFT
 */

//Includes
#include <iostream>
#include <cmath>
#include <vector>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <complex>

//Definitions
#define NO_OF_SINUSOIDS 5
#define NO_OF_SAMPLING_POINTS 10000
#define A_MAX 1000
#define PI 3.14

using namespace std;

//Function Prototypes
void generateWaveform(double fFrom, double* freq, double* amp, double* val, double* time);
//double* MyDFT(double* a, int size);
void FFT(double* data, int size, complex<double>* fft);
void IFFT(complex<double>* data, int size, complex<double>* fft);

//Main
int main(int arcg, char* argv[]){
  double *ampWave1, *freqWave1, *ampWave2, *freqWave2;
  double *valWave1, *timeWave1, *valWave2, *timeWave2;
  
  complex<double>* DFT1 = (complex<double>*)malloc(NO_OF_SAMPLING_POINTS * sizeof(complex<double>));
  complex<double>* IDFT1 = (complex<double>*)malloc(NO_OF_SAMPLING_POINTS * sizeof(complex<double>));
  
  ampWave1 = (double *)malloc(NO_OF_SINUSOIDS*sizeof(double));
  ampWave2 = (double *)malloc(NO_OF_SINUSOIDS*sizeof(double));
  
  freqWave1 = (double *)malloc(NO_OF_SINUSOIDS*sizeof(double));
  freqWave2 = (double *)malloc(NO_OF_SINUSOIDS*sizeof(double));
  
  valWave1 = (double *)malloc(NO_OF_SAMPLING_POINTS*sizeof(double));
  valWave2 = (double *)malloc(NO_OF_SAMPLING_POINTS*sizeof(double));
  
  timeWave1 = (double *)malloc(NO_OF_SAMPLING_POINTS*sizeof(double));
  timeWave2 = (double *)malloc(NO_OF_SAMPLING_POINTS*sizeof(double));
  
  // First band from 6kHz to 10 kHz
  generateWaveform(6.0, freqWave1, ampWave1, valWave1, timeWave1);
  // Second band from 15 kHz to 19 kHz
  generateWaveform(15.0, freqWave2, ampWave2, valWave2, timeWave2);
  cout<<"Waves generated"<<endl;
  FFT(ampWave1, NO_OF_SAMPLING_POINTS, DFT1);
  cout<<"FFT done"<<endl;
  IFFT(DFT1, NO_OF_SAMPLING_POINTS, IDFT1);
  cout<<"IFFT done"<<endl;
  
  for(int i = 0; i<NO_OF_SAMPLING_POINTS; i++){
          cout<<ampWave1[i]<<"    and    "<<real(IDFT1[i])<<endl;
  }
  //free ampWave, freqWave, valWave and timeWave
  free(ampWave1);
  free(freqWave1);
  free(ampWave2);
  free(freqWave2);
  free(valWave1);
  free(timeWave1);
  free(valWave2);
  free(timeWave2);
  return 0;
}

//Function definitions

void generateWaveform(double fFrom, double* freq, double* amp, double* val, double* time){
        int i, j;
        
        double **t, **F;
        t = (double **)malloc(NO_OF_SINUSOIDS * sizeof(double*));
        F = (double **)malloc(NO_OF_SINUSOIDS * sizeof(double*));
        
        for (i = 0; i < NO_OF_SINUSOIDS; i++)
        {
                freq[i] = fFrom + 4.0 * ((double)rand() / (RAND_MAX));
                amp[i] = ( A_MAX / NO_OF_SINUSOIDS - 1.0) * ((double)rand() / (RAND_MAX)) + 1.0;
                t[i] = (double *)malloc(NO_OF_SAMPLING_POINTS * sizeof(double));
                F[i] = (double *)malloc(NO_OF_SAMPLING_POINTS * sizeof(double));
        }
        
        for (i = 0; i < NO_OF_SINUSOIDS; i++)
        {
                for (j = 0; j < NO_OF_SAMPLING_POINTS; ++j)
                {
                        t[i][j] = (double) j * 0.00006;                         //( (double) j * 2.0 / (FREQUENCY_GENERATION * McIN_FREQUENCY));
                        F[i][j] = amp[i] * sinf(2.0 * PI * freq[i] * t[i][j]);
                }
        }
        
        for (j = 0; j < NO_OF_SAMPLING_POINTS; ++j)
        {
                time[j] = (double) j * 0.00006;                                 //( (double) j * 2.0 / (FREQUENCY_GENERATION * MIN_FREQUENCY));
                val[j] = F[0][j] + F[1][j] + F[2][j] + F[3][j] + F[4][j];
        }

        //free t and F
        for(int i = 0; i<NO_OF_SINUSOIDS; i++){
                free(t[i]);
                free(F[i]);
        }
        free(t);
        free(F);
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

void FFT(double* data, int size, complex<double>* fft){
        int n = size;
        if(n==1){
                complex<double> temp(data[0], 0.0);
                fft[0] = temp;
                return;
        }
        double arg = ((2.0 * PI) / n);
        complex<double> wn(cos(arg), sin(arg));
        complex<double> w(1.0, 0.0);
        double* a0 = (double *)malloc(n/2 * sizeof(double));
        double* a1 = (double *)malloc(n/2 * sizeof(double));
        int licz = 0;
        for(int i = 0; i<n-1; i+=2, licz++){
                a0[licz] = data[i];
                a1[licz] = data[i+1];
        }
        complex<double>* y0 = (complex<double> *)malloc(n/2 * sizeof(complex<double>));
        complex<double>* y1 = (complex<double> *)malloc(n/2 * sizeof(complex<double>));
        FFT(a0, n/2, y0);
        FFT(a1, n/2, y1);
        for(int k = 0; k < (n/2); k++){
                fft[k] = y0[k] + (w*y1[k]);
                fft[k + (n/2)] = y0[k] - (w * y1[k]);
                w *= wn;
        }
        free(y0);
        free(y1);
        free(a0);
        free(a1);
        return;
}

void IFFT(complex<double>* data, int size, complex<double>* ifft){
        int n = size;
        if(n==1){
                ifft[0] = data[0];
                return;
        }
        double arg = ((2.0 * PI) / n);
        complex<double> wn(cos(arg), sin(arg));
        complex<double> w(-1.0, 0.0);
        complex<double>* a0 = (complex<double> *)malloc(n/2 * sizeof(complex<double>));
        complex<double>* a1 = (complex<double> *)malloc(n/2 * sizeof(complex<double>));
        int licz = 0;
        for(int i = 0; i<n-1; i+=2, licz++){
                a0[licz] = data[i];
                a1[licz] = data[i+1];
        }
        complex<double>* y0 = (complex<double> *)malloc(n/2 * sizeof(complex<double>));
        complex<double>* y1 = (complex<double> *)malloc(n/2 * sizeof(complex<double>));
        IFFT(a0, n/2, y0);
        IFFT(a1, n/2, y1);
        for(int k = 0; k < (n/2); k++){
                ifft[k] = y0[k] + (w*y1[k]);
                ifft[k + (n/2)] = y0[k] - (w * y1[k]);
                w *= wn;
        }
        free(y0);
        free(y1);
        free(a0);
        free(a1);
        return;
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
