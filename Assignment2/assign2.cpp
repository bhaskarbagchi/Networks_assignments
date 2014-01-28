/* 
 * Assignment 2
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
#define NO_OF_SAMPLING_POINTS 16384
#define A_MAX 500
#define PI 3.14159265358979323846

using namespace std;

//Function Prototypes
void generateWaveform(double fFrom, double* freq, double* amp, double* val, double* time);
void FFT(double* data, int size, complex<double>* fft);
void IFFT(complex<double>* data, int size, complex<double>* fft);

//Main
int main(int arcg, char* argv[]){
  
  double *ampWave1, *ampWave2, *freqWave1, *freqWave2;
  double *valWave1, *valWave2;
  double *timeWave1, *timeWave2;
  
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
  
  FFT(valWave1, NO_OF_SAMPLING_POINTS, DFT1);
  cout<<"FFT done"<<endl;
  
  IFFT(DFT1, NO_OF_SAMPLING_POINTS, IDFT1);
  cout<<"IFFT done"<<endl;
  
  for(int i = 0; i<NO_OF_SAMPLING_POINTS; i++){
          cout<<valWave1[i]<<"          "<<real(IDFT1[i])/NO_OF_SAMPLING_POINTS<<endl;
  }
  cout<<"Printed"<<endl;
  
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
        {       freq[i] = fFrom + 4.0 * ((double)rand() / (RAND_MAX));
                amp[i] = ( A_MAX / NO_OF_SINUSOIDS ) * ((double)rand() / (RAND_MAX));
                t[i] = (double *)malloc(NO_OF_SAMPLING_POINTS * sizeof(double));
                F[i] = (double *)malloc(NO_OF_SAMPLING_POINTS * sizeof(double));
        }
        
        for (i = 0; i < NO_OF_SINUSOIDS; i++)
        {       for (j = 0; j < NO_OF_SAMPLING_POINTS; ++j)
                {       t[i][j] = (double) j * 0.00006;
                        F[i][j] = amp[i] * sinf(2.0 * PI * freq[i] * t[i][j]);
                }
        }
        
        for (j = 0; j < NO_OF_SAMPLING_POINTS; ++j)
        {       time[j] = (double) j * 0.00006;
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
        double arg = ((-2.0 * PI) / n);
        complex<double> wn(cos(arg), sin(arg));
        complex<double> w(1.0, 0.0);
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
