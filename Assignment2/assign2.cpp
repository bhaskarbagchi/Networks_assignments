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
#define NO_OF_SAMPLING_POINTS 524288                            //=No of bins
#define SAMPLING_TIME 0.000000002                               //(1/500000000)
                                                                //Therefore freq of each bin = (Sampling freq / 2) / (No of bins / 2) = 953.67 Hz
                                                                //Therefore total frequency in the band = 953.67 * 524288 = 500000000
                                                                //Total sampling time = 0.000000002*524288 = 0.001048 secs > (1/3500) time period of wave with leasat frequency
#define A_MAX 500
#define PI 3.14159265358979323846

using namespace std;

//Function Prototypes
void generateWaveform(double* freq, double* amp, double* val, double* time);
void FFT(double* data, int size, complex<double>* fft);
void IFFT(complex<double>* data, int size, complex<double>* fft);
void shiftAndMerges(complex<double>* DFT1, int shift1, complex<double>* DFT2, int shift2, complex<double>* shiftedFFT);
void retriveWaves(complex<double>* shiftedFFT, int shift1, complex<double>* FirstFFT, int shift2, complex<double>* SecondFFT);
double rmsError(double* wave1, double* wave2);
void addNoise(complex<double>* input, complex<double>* output, double noise = 30.0);
void generateQuantizedWaveform(double* x, double* y, double S, int num_lvls, double sampling_frequency, double** qx, double** qy, int pflag);
void interpolate(double* x, double* y, double* interpolatedX, double* interpolatedY, int pflag);

//Main
int main(int arcg, char* argv[]){

        double *ampWave1, *ampWave2, *freqWave1, *freqWave2;
        double *valWave1, *valWave2;
        double *timeWave1, *timeWave2;

        complex<double>* DFT1 = (complex<double>*)malloc(NO_OF_SAMPLING_POINTS * sizeof(complex<double>));
        complex<double>* IDFT1 = (complex<double>*)malloc(NO_OF_SAMPLING_POINTS * sizeof(complex<double>));
        complex<double>* DFT2 = (complex<double>*)malloc(NO_OF_SAMPLING_POINTS * sizeof(complex<double>));
        complex<double>* IDFT2 = (complex<double>*)malloc(NO_OF_SAMPLING_POINTS * sizeof(complex<double>));

        ampWave1 = (double *)malloc(NO_OF_SINUSOIDS*sizeof(double));
        ampWave2 = (double *)malloc(NO_OF_SINUSOIDS*sizeof(double));

        freqWave1 = (double *)malloc(NO_OF_SINUSOIDS*sizeof(double));
        freqWave2 = (double *)malloc(NO_OF_SINUSOIDS*sizeof(double));

        valWave1 = (double *)malloc(NO_OF_SAMPLING_POINTS*sizeof(double));
        valWave2 = (double *)malloc(NO_OF_SAMPLING_POINTS*sizeof(double));

        timeWave1 = (double *)malloc(NO_OF_SAMPLING_POINTS*sizeof(double));
        timeWave2 = (double *)malloc(NO_OF_SAMPLING_POINTS*sizeof(double));

        generateWaveform(freqWave1, ampWave1, valWave1, timeWave1);
        generateWaveform(freqWave2, ampWave2, valWave2, timeWave2);
        cout<<"Waves generated"<<endl;

        FFT(valWave1, NO_OF_SAMPLING_POINTS, DFT1);
        cout<<"First FFT done"<<endl;

        IFFT(DFT1, NO_OF_SAMPLING_POINTS, IDFT1);
        cout<<"First IFFT done"<<endl;

        FFT(valWave2, NO_OF_SAMPLING_POINTS, DFT2);
        cout<<"Second FFT done"<<endl;

        IFFT(DFT2, NO_OF_SAMPLING_POINTS, IDFT2);
        cout<<"Second IFFT done"<<endl;

        //for(int i = (NO_OF_SAMPLING_POINTS/10); i<NO_OF_SAMPLING_POINTS; i++){
                //for(int j = (NO_OF_SAMPLING_POINTS/10); j<NO_OF_SAMPLING_POINTS; j++){
                        int i, j, shift1, shift2;
                        cout<<"Enter shift for 1st wave in Hz : ";
                        cin>>shift1;
                        cout<<"Enter shift for 2nd wave in Hz : ";
                        cin>>shift2;
                        i = shift1/953.67;
                        j = shift2/953.67;
                        
                        //Transmitter
                        complex<double>* shiftedFFT = (complex<double> *)malloc(NO_OF_SAMPLING_POINTS * sizeof(complex<double>));
                        complex<double>* shiftedIFFT = (complex<double> *)malloc(NO_OF_SAMPLING_POINTS * sizeof(complex<double>));
                        shiftAndMerges(DFT1, i, DFT2, j, shiftedFFT);
                        IFFT(shiftedFFT, NO_OF_SAMPLING_POINTS, shiftedIFFT);
                        
                        //Reciever - NOISELESS CHANNEL
                        complex<double>* shiftedFFTreciever = (complex<double> *)malloc(NO_OF_SAMPLING_POINTS * sizeof(complex<double>));
                        complex<double>* recievedFirstFFT = (complex<double> *)malloc(NO_OF_SAMPLING_POINTS * sizeof(complex<double>));
                        complex<double>* recievedSecondFFT = (complex<double> *)malloc(NO_OF_SAMPLING_POINTS * sizeof(complex<double>));
                        complex<double>* recievedFirstCmplx = (complex<double> *)malloc(NO_OF_SAMPLING_POINTS * sizeof(complex<double>));
                        complex<double>* recievedSecondCmplx = (complex<double> *)malloc(NO_OF_SAMPLING_POINTS * sizeof(complex<double>));
                        double* recieved = (double *)malloc(NO_OF_SAMPLING_POINTS * sizeof(double));
                        double* recievedFirst = (double *)malloc(NO_OF_SAMPLING_POINTS * sizeof(double));
                        double* recievedSecond = (double *)malloc(NO_OF_SAMPLING_POINTS * sizeof(double));
                        for(int k = 0; k<NO_OF_SAMPLING_POINTS; k++){
                                recieved[k] = real(shiftedIFFT[i]);
                        }
                        FFT(recieved, NO_OF_SAMPLING_POINTS, shiftedFFTreciever);
                        retriveWaves(shiftedFFTreciever, i, recievedFirstFFT, j, recievedSecondFFT);
                        IFFT(recievedFirstFFT, NO_OF_SAMPLING_POINTS, recievedFirstCmplx);
                        IFFT(recievedSecondFFT, NO_OF_SAMPLING_POINTS, recievedSecondCmplx);
                        //collect only the real part
                        for(int k = 0; k<NO_OF_SAMPLING_POINTS; k++){
                                recievedFirst[k] = real(recievedFirstCmplx[i]);
                                recievedSecond[k] = real(recievedSecondCmplx[i]);
                        }
                        //compare with real waves
                        double error1 = rmsError(valWave1, recievedFirst);
                        double error2 = rmsError(valWave2, recievedSecond);
                        cout<<"The root mean squared errors are as follows:"<<endl;
                        cout<<"\t\tWave1 = "<<error1<<endl;
                        cout<<"\t\tWave2 = "<<error2<<endl;
                        
                        //for NOISY CHANNEL
                        //Introduce noise
                        complex<double>* noisy = (complex<double> *)malloc(NO_OF_SAMPLING_POINTS * sizeof(complex<double>));
                        addNoise(shiftedIFFT, noisy, 100);
                        //Recieve
                        for(int k = 0; k<NO_OF_SAMPLING_POINTS; k++){
                                recieved[k] = real(noisy[i]);
                        }
                        FFT(recieved, NO_OF_SAMPLING_POINTS, shiftedFFTreciever);
                        retriveWaves(shiftedFFTreciever, i, recievedFirstFFT, j, recievedSecondFFT);
                        IFFT(recievedFirstFFT, NO_OF_SAMPLING_POINTS, recievedFirstCmplx);
                        IFFT(recievedSecondFFT, NO_OF_SAMPLING_POINTS, recievedSecondCmplx);
                        //collect only the real part
                        for(int k = 0; k<NO_OF_SAMPLING_POINTS; k++){
                                recievedFirst[k] = real(recievedFirstCmplx[i]);
                                recievedSecond[k] = real(recievedSecondCmplx[i]);
                        }
                        //compare with real waves
                        error1 = rmsError(valWave1, recievedFirst);
                        error2 = rmsError(valWave2, recievedSecond);
                        cout<<"The root mean squared errors are as follows:"<<endl;
                        cout<<"\t\tWave1 = "<<error1<<endl;
                        cout<<"\t\tWave2 = "<<error2<<endl;
                        
                        
                        free(shiftedFFT);
                        free(shiftedIFFT);
                        free(shiftedFFTreciever);
                        free(recievedFirstFFT);
                        free(recievedSecondFFT);
                        free(recievedFirstCmplx);
                        free(recievedSecondCmplx);
                        free(recieved);
                        free(recievedFirst);
                        free(recievedSecond);
                //}
        //}

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

void generateWaveform(double* freq, double* amp, double* val, double* time){
        int i, j;
        
        double **t, **F;
        t = (double **)malloc(NO_OF_SINUSOIDS * sizeof(double*));
        F = (double **)malloc(NO_OF_SINUSOIDS * sizeof(double*));
        
        for (i = 0; i < NO_OF_SINUSOIDS; i++)
        {       freq[i] = 3.50 + (20.0 - 3.50) * ((double)rand() / (RAND_MAX));
                amp[i] = ( A_MAX / NO_OF_SINUSOIDS ) * ((double)rand() / (RAND_MAX));
                t[i] = (double *)malloc(NO_OF_SAMPLING_POINTS * sizeof(double));
                F[i] = (double *)malloc(NO_OF_SAMPLING_POINTS * sizeof(double));
        }
        
        for (i = 0; i < NO_OF_SINUSOIDS; i++)
        {       for (j = 0; j < NO_OF_SAMPLING_POINTS; ++j)
                {       t[i][j] = (double) j * SAMPLING_TIME;
                        F[i][j] = amp[i] * sinf(2.0 * PI * freq[i] * t[i][j]);
                }
        }
        
        for (j = 0; j < NO_OF_SAMPLING_POINTS; ++j)
        {       time[j] = (double) j * SAMPLING_TIME;
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

void shiftAndMerges(complex<double>* DFT1, int shift1, complex<double>* DFT2, int shift2, complex<double>* shiftedFFT){
        int min = (shift1 < shift2)? shift1: shift2;
        int max = (shift1 > shift2)? shift1: shift2;
        int i = 0, j = 0, k = 0;
        for(i = 0; i<min; i++){
                complex<double> temp(0.0, 0.0);
                shiftedFFT[i] = temp;
        }
        if(shift1 < shift2){
                for(; i<max; i++){
                        shiftedFFT[i] = DFT1[j];
                        j++;
                }
                for(; i< NO_OF_SAMPLING_POINTS; i++){
                        shiftedFFT[i] = /*DFT1[j] +*/ DFT2[k];
                        //j++;
                        k++;
                }
                return;
        }
        else{
                for(; i<max; i++){
                        shiftedFFT[i] = DFT2[k];
                        k++;
                }
                for(; i< NO_OF_SAMPLING_POINTS; i++){
                        shiftedFFT[i] = DFT1[j];// + DFT2[k];
                        j++;// k++;
                }
                return;
        }
}

void retriveWaves(complex<double>* shiftedFFT, int shift1, complex<double>* FirstFFT, int shift2, complex<double>* SecondFFT){
        int min = (shift1 < shift2)? shift1: shift2;
        int max = (shift1 > shift2)? shift1: shift2;
        int i = 0, j = 0, k = 0;
        if(shift1<shift2){
                for(i = shift1; i<shift2; i++){
                        FirstFFT[j] = shiftedFFT[i];
                        j++;
                }
                for(; i<NO_OF_SAMPLING_POINTS; i++){
                        SecondFFT[k] = shiftedFFT[i];
                        k++;
                }
                for(; j<NO_OF_SAMPLING_POINTS; j++){
                        complex<double> temp(0.0, 0.0);
                        FirstFFT[j] = temp;
                }
                for(; k<NO_OF_SAMPLING_POINTS; k++){
                        complex<double> temp(0.0, 0.0);
                        SecondFFT[k] = temp;
                }
                return;
        }
        else{
                for(i = shift2; i<shift1; i++){
                        SecondFFT[k] = shiftedFFT[i];
                        k++;
                }
                for(; i<NO_OF_SAMPLING_POINTS; i++);{
                        FirstFFT[j] = shiftedFFT[i];
                        j++;
                }
                for(; j<NO_OF_SAMPLING_POINTS; j++){
                        complex<double> temp(0.0, 0.0);
                        FirstFFT[j] = temp;
                }
                for(; k<NO_OF_SAMPLING_POINTS; k++){
                        complex<double> temp(0.0, 0.0);
                        SecondFFT[k] = temp;
                }
                return;
        }
}

double rmsError(double* wave1, double* wave2){
        double e = 0.0;
        for(int i = 0; i<NO_OF_SAMPLING_POINTS; i++){
                e += pow((wave1[i] - wave2[i]),2);
        }
        e /= (double)NO_OF_SAMPLING_POINTS;
        return sqrt(e);
}

void addNoise(complex<double>* input, complex<double>* output, double noise){
        for(int i = 0; i<NO_OF_SAMPLING_POINTS; i++){
                if(rand() %2 == 0){
                        complex<double> temp((2* noise * (double) rand()/ RAND_MAX - noise), 0.0);
                        output[i] = input[i] + temp;
                }
                else{
                        output[i] = input[i];
                }
        }
        return;
}

/*
void interpolate(double* x, double* y, double* interpolatedX, double* interpolatedY, int pflag) {
        int i;
        double xi, yi;
        FILE* fp;

        if(pflag) fp = fopen("wInterpol.dat", "w");

        gsl_interp_accel *acc = gsl_interp_accel_alloc ();
        const gsl_interp_type *t = gsl_interp_cspline_periodic;
        gsl_spline *spline = gsl_spline_alloc (t, FREQUENCY_SAMPLING + 1);
        gsl_spline_init (spline, x, y, FREQUENCY_SAMPLING + 1);

        if(pflag) fprintf (fp, "# Interpolated values\n");
        for (i = 0; i <= FREQUENCY_INTERPOLATION; i++)
        {
                xi = (double) i * x[FREQUENCY_SAMPLING] / FREQUENCY_INTERPOLATION;
                yi = gsl_spline_eval (spline, xi, acc);
                if(pflag) fprintf (fp, "%g %g\n", xi, yi);
                interpolatedX[i] = xi;
                interpolatedY[i] = yi;
        }

        gsl_spline_free (spline);
        gsl_interp_accel_free (acc);
        if(pflag) fclose(fp);
}

void generateQuantizedWaveform(double* x, double* y, double S, int num_lvls, double sampling_frequency, double** qx, double** qy, int pflag) {
        int i, j, flag;
        FILE *fp;
        if(pflag) fp = fopen("wQuant.dat", "w");

        double *qw, *t, *lvls, *threshold, k, m;
        FREQUENCY_SAMPLING = (sampling_frequency * 2 ) / MIN_FREQUENCY + 1;
        
        *qy = qw = (double*) malloc((FREQUENCY_SAMPLING + 1) * sizeof(double));
        *qx = t = (double*) malloc((FREQUENCY_SAMPLING + 1) * sizeof(double));
        lvls = (double*) malloc(num_lvls * sizeof(double));
        threshold = (double*) malloc((num_lvls - 1) * sizeof(double));

        for (i = 0; i < num_lvls; ++i)
        {
                lvls[i] = (double) i * 2.0 * S / (num_lvls - 1) - S;
        }

        for (i = 0; i < num_lvls - 1; ++i)
        {
                threshold[i] = (lvls[i] + lvls[i + 1]) / 2;
        }

        m = (double) FREQUENCY_INTERPOLATION / FREQUENCY_SAMPLING;
        for (k = 0, i = 0; k <= FREQUENCY_INTERPOLATION; k = k + m)
        {
                flag = 1;
                for (j = 0; j < num_lvls - 1; ++j) {
                        if( y[(int)k] < threshold[j] ) {
                                qw[i] = lvls[j];
                                t[i] = x[(int)k];
                                flag = 0;
                                break;
                        }
                }
                if(flag) {
                        qw[i] = lvls[num_lvls - 1];
                        t[i] = x[(int)k];
                }

                if(pflag) fprintf (fp, "%g %g\n", t[i], qw[i]);
                i++;
        }

        if(t[i-1] < x[FREQUENCY_INTERPOLATION]) {
                flag = 1;
                for (j = 0; j < num_lvls - 1; ++j) {
                        if( y[FREQUENCY_INTERPOLATION] < threshold[j] ) {
                                qw[FREQUENCY_SAMPLING] = lvls[j];
                                t[FREQUENCY_SAMPLING] = x[FREQUENCY_INTERPOLATION];
                                flag = 0;
                                break;
                        }
                }
                if(flag) {
                        qw[FREQUENCY_SAMPLING] = lvls[num_lvls - 1];
                        t[FREQUENCY_SAMPLING] = x[FREQUENCY_INTERPOLATION];
                }

                if(pflag) fprintf (fp, "%g %g\n", t[FREQUENCY_SAMPLING], qw[FREQUENCY_SAMPLING]);
        }
        else {
                FREQUENCY_SAMPLING--;
        }

        free(threshold);
        free(lvls);
        if(pflag) fclose(fp);
}*/
