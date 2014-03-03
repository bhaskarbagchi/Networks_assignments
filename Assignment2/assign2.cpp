/* 
 * The assignment is a C++ code but due to the submission constraints the filename has been given the extension .c
 * As mentioned in the assignment statement statemt we are allowed to code in cpp.
 * Before compilation please change the file type to .cpp
 * Compile using $ g++ assign2.cpp -lgsl -lgslcblas -lm
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
#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>

//Definitions

//#define TEST_FREQUENCY 1
//#define TEST_QUANTIZATION 1

#define NO_OF_SINUSOIDS 5
#define SAMPLING_FREQUENCY 500000000
#define NO_OF_SAMPLING_POINTS (524288/2)                        //=No of bins
#define SAMPLING_TIME 0.000000002                               //(1/500000000)
                                                                //Therefore freq of each bin = (Sampling freq / 2) / (No of bins / 2) = 953.67 Hz
                                                                //Therefore total frequency in the band = 953.67 * 524288 = 500000000
                                                                //Total sampling time = 0.000000002*524288 = 0.001048 secs > (1/3500) time period of wave with leasat frequency
#define BIN_SIZE (2*953.67)
#define A_MAX 500
#define PI 3.14159265358979323846
#define QUANT_SAMPLING_POINTS (65536*2)
#define QUANT_SAMPLING_FREQUENCY (65200000*2)
#define QUANT_SAMPLING_TIME (0.000000016/2)

using namespace std;

//Function Prototypes
void generateWaveform(double* freq, double* amp, double* val, double* time);
void FFT(double* data, int size, complex<double>* fft);
void IFFT(complex<double>* data, int size, complex<double>* fft);
void IFFTwrapper(complex<double>* data, int size, complex<double>* fft);
void shiftAndMerges(complex<double>* DFT1, int shift1, complex<double>* DFT2, int shift2, complex<double>* shiftedFFT);
void retriveWaves(complex<double>* shiftedFFT, int shift1, complex<double>* FirstFFT, int shift2, complex<double>* SecondFFT);
double rmsError(double* wave1, double* wave2);
void addNoise(complex<double>* input, complex<double>* output, double size, double noise = 30.0);
void generateQuantizedWaveform(complex<double>* x, double* y, double S, int num_lvls, double sampling_frequency, complex<double>* qx, double* qy);
void recoverQuantized(complex<double>* noisy_quant, complex<double>* recovered_quant, int a_max, int quant_level, int size);
void interpolate(complex<double>* recovered_quant, double* quant_time, double freq_sampling, double* interpolated, double* time_interpolate);

//Main
int main(int arcg, char* argv[]){

        double *ampWave1, *ampWave2, *freqWave1, *freqWave2;
        double *valWave1, *valWave2;
        double *timeWave1, *timeWave2;
        
        FILE *fp, *fp1, *fp2, *fp3, *fp4;

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
        
        srand(time(NULL));

        generateWaveform(freqWave1, ampWave1, valWave1, timeWave1);
        generateWaveform(freqWave2, ampWave2, valWave2, timeWave2);
        cout<<"Waves generated"<<endl;

        FFT(valWave1, NO_OF_SAMPLING_POINTS, DFT1);
        cout<<"First FFT done"<<endl;

        IFFTwrapper(DFT1, NO_OF_SAMPLING_POINTS, IDFT1);
        cout<<"First IFFT done"<<endl;

        FFT(valWave2, NO_OF_SAMPLING_POINTS, DFT2);
        cout<<"Second FFT done"<<endl;

        IFFTwrapper(DFT2, NO_OF_SAMPLING_POINTS, IDFT2);
        cout<<"Second IFFT done"<<endl;
        
        fp1 = fopen("wave1.dat", "w");
        fp2 = fopen("IFFT1.dat", "w");
        fp3 = fopen("wave2.dat", "w");
        fp4 = fopen("IFFT2.dat", "w");
        for(int i = 0; i<NO_OF_SAMPLING_POINTS; i++){
                fprintf(fp1, "%g %g\n", timeWave1[i], valWave1[i]);
                fprintf(fp2, "%g %g\n", timeWave1[i], real(IDFT1[i]));
                fprintf(fp3, "%g %g\n", timeWave2[i], valWave2[i]);
                fprintf(fp4, "%g %g\n", timeWave2[i], real(IDFT2[i]));
        }
        fclose(fp1);
        fclose(fp2);
        fclose(fp3);
        fclose(fp4);

        int sh1, sh2, shift1, shift2;
        cout<<"Enter shift for 1st wave in Hz : ";
        cin>>shift1;
        cout<<"Enter shift for 2nd wave in Hz : ";
        cin>>shift2;
        sh1 = shift1/BIN_SIZE;
        sh2 = shift2/BIN_SIZE;
        
        cout<<"Shifted bin values : "<<sh1<<"    "<<sh2<<endl;
        
        //Transmitter
        complex<double>* shiftedFFT = (complex<double> *)malloc(NO_OF_SAMPLING_POINTS * sizeof(complex<double>));
        complex<double>* shiftedIFFT = (complex<double> *)malloc(NO_OF_SAMPLING_POINTS * sizeof(complex<double>));
        shiftAndMerges(DFT1, sh1, DFT2, sh2, shiftedFFT);
        IFFTwrapper(shiftedFFT, NO_OF_SAMPLING_POINTS, shiftedIFFT);
        fp = fopen("Shifted.dat", "w");
        for(int i = 0; i<NO_OF_SAMPLING_POINTS; i++){
                fprintf(fp, "%g, %g\n", timeWave1[i], real(shiftedIFFT[i]));
        }
        fclose(fp);
        
        
        //Reciever - NOISELESS CHANNEL
        complex<double>* shiftedFFTreciever = (complex<double> *)malloc(NO_OF_SAMPLING_POINTS * sizeof(complex<double>));
        complex<double>* recievedFirstFFT = (complex<double> *)malloc(NO_OF_SAMPLING_POINTS * sizeof(complex<double>));
        complex<double>* recievedSecondFFT = (complex<double> *)malloc(NO_OF_SAMPLING_POINTS * sizeof(complex<double>));
        complex<double>* recievedFirstCmplx = (complex<double> *)malloc(NO_OF_SAMPLING_POINTS * sizeof(complex<double>));
        complex<double>* recievedSecondCmplx = (complex<double> *)malloc(NO_OF_SAMPLING_POINTS * sizeof(complex<double>));
        double* e_recieved = (double *)malloc(NO_OF_SAMPLING_POINTS * sizeof(double));
        double* recieved = (double *)malloc(NO_OF_SAMPLING_POINTS * sizeof(double));
        double* recievedFirst = (double *)malloc(NO_OF_SAMPLING_POINTS * sizeof(double));
        double* recievedSecond = (double *)malloc(NO_OF_SAMPLING_POINTS * sizeof(double));
        double error1, error2;
        for(int k = 0; k<NO_OF_SAMPLING_POINTS; k++){
                recieved[k] = real(shiftedIFFT[k]);
        }
        FFT(recieved, NO_OF_SAMPLING_POINTS, shiftedFFTreciever);
        retriveWaves(shiftedFFTreciever, sh1, recievedFirstFFT, sh2, recievedSecondFFT);
        IFFTwrapper(recievedFirstFFT, NO_OF_SAMPLING_POINTS, recievedFirstCmplx);
        IFFTwrapper(recievedSecondFFT, NO_OF_SAMPLING_POINTS, recievedSecondCmplx);
        //collect only the real part
        fp1 = fopen("RecWave1.dat","w");
        fp2 = fopen("RecWave2.dat","w");
        for(int k = 0; k<NO_OF_SAMPLING_POINTS; k++){
                recievedFirst[k] = real(recievedFirstCmplx[k]);
                fprintf(fp1, "%g %g\n", timeWave1[k], recievedFirst[k]);
                recievedSecond[k] = real(recievedSecondCmplx[k]);
                fprintf(fp2, "%g %g\n", timeWave2[k], recievedSecond[k]);
        }
        fclose(fp1);
        fclose(fp2);
        //compare with real waves
        error1 = rmsError(valWave1, recievedFirst);
        error2 = rmsError(valWave2, recievedSecond);
        cout<<"The root mean squared errors are as follows:"<<endl;
        cout<<"\t\tWave1 = "<<error1<<endl;
        cout<<"\t\tWave2 = "<<error2<<endl;
        
        //for NOISY CHANNEL
        //Introduce noise
        complex<double>* noisy = (complex<double> *)malloc(NO_OF_SAMPLING_POINTS * sizeof(complex<double>));
        addNoise(shiftedIFFT, noisy, NO_OF_SAMPLING_POINTS, 100);
        //Recieve
        for(int k = 0; k<NO_OF_SAMPLING_POINTS; k++){
                e_recieved[k] = real(noisy[k]);
        }
        FFT(e_recieved, NO_OF_SAMPLING_POINTS, shiftedFFTreciever);
        retriveWaves(shiftedFFTreciever, sh1, recievedFirstFFT, sh2, recievedSecondFFT);
        IFFTwrapper(recievedFirstFFT, NO_OF_SAMPLING_POINTS, recievedFirstCmplx);
        IFFTwrapper(recievedSecondFFT, NO_OF_SAMPLING_POINTS, recievedSecondCmplx);
        //collect only the real part
        fp1 = fopen("RecWave1Noisy.dat","w");
        fp2 = fopen("RecWave2Noisy.dat","w");
        for(int k = 0; k<NO_OF_SAMPLING_POINTS; k++){
                recievedFirst[k] = real(recievedFirstCmplx[k]);
                fprintf(fp1, "%g %g\n", timeWave1[k], recievedFirst[k]);
                recievedSecond[k] = real(recievedSecondCmplx[k]);
                fprintf(fp2, "%g %g\n", timeWave2[k], recievedSecond[k]);
        }
        fclose(fp1);
        fclose(fp2);
        //compare with real waves
        error1 = rmsError(valWave1, recievedFirst);
        error2 = rmsError(valWave2, recievedSecond);
        cout<<"The root mean squared errors are as follows:"<<endl;
        cout<<"\t\tWave1 = "<<error1<<endl;
        cout<<"\t\tWave2 = "<<error2<<endl;
        
        //QUANTIZED IN NOISY CHANNEL
        int samp_freq = QUANT_SAMPLING_FREQUENCY;
        int quant_level = 30;
        int size_quant;
        size_quant = QUANT_SAMPLING_POINTS;
        cout<<"Sampling Frequency : "<< samp_freq <<endl;
        cout<<"Quantization Level : "<< quant_level<<endl;
        complex<double>* quant_val = (complex<double>*)malloc((size_quant ) * sizeof(complex<double>));
        complex<double>* noisy_quant = (complex<double>*)malloc((size_quant ) * sizeof(complex<double>));
        complex<double>* recovered_quant = (complex<double>*)malloc((size_quant ) * sizeof(complex<double>));
        double* interpolated = (double*)malloc(NO_OF_SAMPLING_POINTS * sizeof(double));
        double* time_interpolate = (double*)malloc(NO_OF_SAMPLING_POINTS * sizeof(double));
        double* quant_time = (double *)malloc((size_quant ) * sizeof(double));
        
        generateQuantizedWaveform(shiftedIFFT, timeWave1, A_MAX, quant_level, size_quant, quant_val, quant_time);
        addNoise(quant_val, noisy_quant, size_quant);
        
        recoverQuantized(noisy_quant, recovered_quant, A_MAX, quant_level, size_quant);
        interpolate(recovered_quant, quant_time, size_quant, interpolated, time_interpolate);

        double tt = rmsError(interpolated, recieved);
        cout<<" Quantized error "<< tt<< endl;
        
        FFT(interpolated, NO_OF_SAMPLING_POINTS, shiftedFFTreciever);
        retriveWaves(shiftedFFTreciever, sh1, recievedFirstFFT, sh2, recievedSecondFFT);
        IFFTwrapper(recievedFirstFFT, NO_OF_SAMPLING_POINTS, recievedFirstCmplx);
        IFFTwrapper(recievedSecondFFT, NO_OF_SAMPLING_POINTS, recievedSecondCmplx);
        //collect only the real part
        fp1 = fopen("RecWave1Quant.dat","w");
        fp2 = fopen("RecWave2Quant.dat","w");
        for(int k = 0; k<NO_OF_SAMPLING_POINTS; k++){
                recievedFirst[k] = real(recievedFirstCmplx[k]);
                fprintf(fp1, "%g %g\n", timeWave1[k], recievedFirst[k]);
                recievedSecond[k] = real(recievedSecondCmplx[k]);
                fprintf(fp2, "%g %g\n", timeWave2[k], recievedSecond[k]);
        }
        fclose(fp1);
        fclose(fp2);
        //compare with real waves
        error1 = rmsError(valWave1, recievedFirst);
        error2 = rmsError(valWave2, recievedSecond);
        cout<<"The root mean squared errors are as follows:"<<endl;
        cout<<"\t\tWave1 = "<<error1<<endl;
        cout<<"\t\tWave2 = "<<error2<<endl;

#ifdef TEST_QUANTIZATION
        cout<<"Testing quant .";
        fp3 = fopen("ErrorVsQuant1.dat", "w");
        fp4 = fopen("ErrorVsQuant2.dat", "w");
        int new_size = 524288/4;
        for(int testt = 0; testt<50; testt++){
                cout<<" .";
                //new_size/=2;
                //decide sampling frequency and array size accordingly
                //malloc quant_val_test and quant_time_test
                
                complex<double>* quant_val_test = (complex<double> *)malloc((new_size) * sizeof(complex<double>));
                double* quant_time_test = (double *)malloc((new_size) * sizeof(double));
                complex<double>* noisy_quant_test = (complex<double> *)malloc((new_size) * sizeof(complex<double>));
                complex<double>* recovered_quant_test = (complex<double> *)malloc((new_size) * sizeof(complex<double>));

                generateQuantizedWaveform(shiftedIFFT, timeWave1, A_MAX, 2*testt + 2, new_size, quant_val_test, quant_time_test);
                addNoise(quant_val_test, noisy_quant_test, new_size);
                
                recoverQuantized(noisy_quant_test, recovered_quant_test, A_MAX, 2*testt + 2, new_size);
                interpolate(recovered_quant_test, quant_time_test, new_size, interpolated, time_interpolate);
                
                FFT(interpolated, NO_OF_SAMPLING_POINTS, shiftedFFTreciever);
                retriveWaves(shiftedFFTreciever, sh1, recievedFirstFFT, sh2, recievedSecondFFT);
                IFFTwrapper(recievedFirstFFT, NO_OF_SAMPLING_POINTS, recievedFirstCmplx);
                IFFTwrapper(recievedSecondFFT, NO_OF_SAMPLING_POINTS, recievedSecondCmplx);
                for(int k = 0; k<NO_OF_SAMPLING_POINTS; k++){
                        recievedFirst[k] = real(recievedFirstCmplx[k]);
                        recievedSecond[k] = real(recievedSecondCmplx[k]);
                }
                error1 = rmsError(valWave1, recievedFirst);
                error2 = rmsError(valWave2, recievedSecond);
                fprintf(fp3, "%g %g\n", (double)2*testt + 2, error1);
                fprintf(fp4, "%g %g\n", (double)2*testt + 2, error2);
                free(quant_val_test);
                free(quant_time_test);
                free(noisy_quant_test);
                free(recovered_quant_test);
        }
        fclose(fp3);
        fclose(fp4);
        cout<<endl;
#endif

        

#ifdef TEST_FREQUENCY
        cout<<"Testing .";
        fp3 = fopen("ErrorVsOverlap1.dat", "w");
        fp4 = fopen("ErrorVsOverlap2.dat", "w");
        int shift11, shift22, sh11, sh22;
        for(int iterate = 0; iterate < 50; iterate++){
                cout<<" .";
                shift11 = 150000000;
                shift22 = shift1 -10000 + iterate * 1000;
                sh11 = shift11/BIN_SIZE;
                sh22 = shift22/BIN_SIZE;
                shiftAndMerges(DFT1, sh11, DFT2, sh22, shiftedFFT);
                IFFTwrapper(shiftedFFT, NO_OF_SAMPLING_POINTS, shiftedIFFT);
                FFT(recieved, NO_OF_SAMPLING_POINTS, shiftedFFTreciever);
                retriveWaves(shiftedFFTreciever, sh11, recievedFirstFFT, sh22, recievedSecondFFT);
                IFFTwrapper(recievedFirstFFT, NO_OF_SAMPLING_POINTS, recievedFirstCmplx);
                IFFTwrapper(recievedSecondFFT, NO_OF_SAMPLING_POINTS, recievedSecondCmplx);
                for(int k = 0; k<NO_OF_SAMPLING_POINTS; k++){
                        recievedFirst[k] = real(recievedFirstCmplx[k]);
                        recievedSecond[k] = real(recievedSecondCmplx[k]);
                }
                error1 = rmsError(valWave1, recievedFirst);
                error2 = rmsError(valWave2, recievedSecond);
                fprintf(fp3, "%g %g\n", iterate * 1000.0, error1);
                fprintf(fp4, "%g %g\n", iterate * 1000.0, error2);
        }
        fclose(fp3);
        fclose(fp4);
        cout<<endl;
#endif



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
        {       freq[i] = 3500 + (20000 - 3500) * ((double)rand() / (RAND_MAX));
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

void IFFTwrapper(complex<double>* data, int size, complex<double>* ifft){
        IFFT(data, size, ifft);
        complex<double> temp((double)size, 0.0);
        for(int i = 0; i<size; i++){
                ifft[i] /= size;
        }
        return;
}

void shiftAndMerges(complex<double>* DFT1, int shift1, complex<double>* DFT2, int shift2, complex<double>* shiftedFFT){
        int count, i, half = NO_OF_SAMPLING_POINTS/2;
        
        complex<double>* shifted1 = (complex<double> *)malloc(NO_OF_SAMPLING_POINTS * sizeof(complex<double>));
        for(i = 0; i < shift1; i++){
                complex<double> temp(0.0, 0.0);
                shifted1[i] = temp;
                shifted1[NO_OF_SAMPLING_POINTS -1 - i] = temp;
        }
        count = 0;
        for(; i<half; i++){
                shifted1[i] = DFT1[count];
                shifted1[NO_OF_SAMPLING_POINTS - 1 - i] = DFT1[NO_OF_SAMPLING_POINTS - 1 - count];
                count++;
        }
        
        complex<double>* shifted2 = (complex<double> *)malloc(NO_OF_SAMPLING_POINTS * sizeof(complex<double>));
        for(i = 0; i < shift2; i++){
                complex<double> temp(0.0, 0.0);
                shifted2[i] = temp;
                shifted2[NO_OF_SAMPLING_POINTS - 1 - i] = temp;
        }
        count = 0;
        for(; i<half; i++){
                shifted2[i] = DFT2[count];
                shifted2[NO_OF_SAMPLING_POINTS - 1 - i] = DFT2[NO_OF_SAMPLING_POINTS - 1 - count];
                count++;
        }
        
        for(int i = 0; i < NO_OF_SAMPLING_POINTS; i++){
                shiftedFFT[i] = shifted1[i] + shifted2[i];
        }
        free(shifted1);
        free(shifted2);
        return;
}

void retriveWaves(complex<double>* shiftedFFT, int shift1, complex<double>* FirstFFT, int shift2, complex<double>* SecondFFT){
        int count1, count2, i, half = NO_OF_SAMPLING_POINTS/2;
        count1 = shift1;
        count2 = shift2;
        for(i = 0; i < (20000 / BIN_SIZE) + 1; i++){
                FirstFFT[i] = shiftedFFT[count1];
                FirstFFT[NO_OF_SAMPLING_POINTS - 1 - i] = shiftedFFT[NO_OF_SAMPLING_POINTS - 1 - count1];
                count1++;
                SecondFFT[i] = shiftedFFT[count2];
                SecondFFT[NO_OF_SAMPLING_POINTS - 1 - i] = shiftedFFT[NO_OF_SAMPLING_POINTS - 1 - count2];
                count2++;
        }
        for(; i < half; i++){
                complex<double> temp(0.0, 0.0);
                FirstFFT[i] = SecondFFT[i] = temp;
                FirstFFT[NO_OF_SAMPLING_POINTS - 1 - i] = SecondFFT[NO_OF_SAMPLING_POINTS - 1 - i] = temp;
        }
        return;
}

double rmsError(double* wave1, double* wave2){
        double e = 0.0;
        for(int i = 0; i<NO_OF_SAMPLING_POINTS; i++){
                e += pow((wave1[i] - wave2[i]),2);
        }
        e /= (double)NO_OF_SAMPLING_POINTS;
        return sqrt(e);
}

void addNoise(complex<double>* input, complex<double>* output, double size, double noise){
        for(int i = 0; i<size; i++){
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

void generateQuantizedWaveform(complex<double>* x, double* y, double S, int num_lvls, double size, complex<double>* qx, double* qy){
        double m = NO_OF_SAMPLING_POINTS / size;
        double level = S / num_lvls;
        double thresh = level / 2;
        double xx, yy;
        int _xx, i, k;
        for(i = 0, k = 0; i<size; k+=m, i++){
                xx = real(x[k]);
                _xx = int(xx);
                yy = (_xx%((int)level) > thresh)? (_xx - (_xx%((int)level)) + (int) level): (_xx - (_xx%((int)level)));
                complex<double> temp((double)yy, imag(x[k]));
                qx[i] = temp;
                qy[i] = y[k];
        }
        complex<double> temp(0.0, 0.0);
        return;
}

void recoverQuantized(complex<double>* noisy_quant, complex<double>* recovered_quant, int a_max, int quant_level, int size){
        double level = a_max / quant_level;
        double thresh = level / 2;
        double xx, yy; 
        int _xx;
        for(int i = 0; i < size; i++){
                xx = real(noisy_quant[i]);
                _xx = int(xx);
                yy = (_xx%((int)level) > thresh)? (_xx - (_xx%((int)level)) + (int) level): (_xx - (_xx%((int)level)));
                complex<double> temp((double)yy, imag(noisy_quant[i]));
                recovered_quant[i] = temp;
        }
        return;
}

void interpolate(complex<double>* recovered_quant, double* quant_time, double freq_sampling, double* interpolated, double* time_interpolate){
        double* recieved = (double *)malloc(freq_sampling * sizeof(double));
        for(int i = 0; i<freq_sampling; i++){
                recieved[i] = real(recovered_quant[i]);
        }
        gsl_interp_accel *acc = gsl_interp_accel_alloc();
        const gsl_interp_type *t = gsl_interp_cspline_periodic;
        gsl_spline *spline = gsl_spline_alloc(t, freq_sampling);
        gsl_spline_init(spline, quant_time, recieved, freq_sampling);
        
        double xi, yi;
        xi = 0;
        for(int i = 0; i < (NO_OF_SAMPLING_POINTS) && xi < quant_time[(int)freq_sampling - 1]; i++){
                yi = gsl_spline_eval(spline, xi, acc);
                time_interpolate[i] = xi;
                interpolated[i] = yi;
                xi = (i+1) * SAMPLING_TIME;
        }
        gsl_spline_free(spline);
        gsl_interp_accel_free(acc);
        return;
}