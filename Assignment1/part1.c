#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>

#define FREQUENCY_INTERPOLATION 1000
#define FREQUENCY_GENERATION 1000
#define NO_WAVEFORMS 5
#define PI 3.14159265358979323846

/* Globals */
double MAX_FREQUENCY;
double MIN_FREQUENCY;
int FREQUENCY_SAMPLING;

/* Function prototypes */
void interpolate(double* x, double* y, double* interpolatedX, double* interpolatedY);
void generateWaveform(double fmax, double Amax, double* fs, double* As, double** F, double** t);
void printWaveform(double** F, double** t);
void generateQuantizedWaveform(double* x, double* y, double S, int num_lvls, double sampling_frequency, double** qx, double** qy);
double* addNoise(double* x, double* y, double noise);
double MeanSquaredError(double* original, double* interpolated);

/* Main */
int main(int argc, char const *argv[])
{
        double fmax, Amax, noise, *As, *fs, **F, **t, *qx, *qw, *noisy_qw, sampling_frequency, *interpolatedX, *interpolatedY;
        int i, num_lvls;

        srand(time(NULL));

        printf("Enter maximum frequency (in Hz): ");
        scanf("%lf", &fmax);
        while(fmax<0){
                printf("Frequency can't be negative. Enter maximum frequency (In Hz.): ");
                scanf("%lf", &fmax);
        }

        printf("Enter maximum amplitude (max. signal strength): ");
        scanf("%lf", &Amax);
        while(Amax<0){
                printf("Amplitude can't be negative. Enter maximum amplitude: ");
                scanf("%lf", &Amax);
        }
        
        As = (double*) malloc((NO_WAVEFORMS + 1) * sizeof(double));
        fs = (double*) malloc((NO_WAVEFORMS + 1) * sizeof(double));

        F = (double**) malloc((NO_WAVEFORMS + 1) * sizeof(double*));
        t = (double**) malloc((NO_WAVEFORMS + 1) * sizeof(double*));

        for (i = 0; i <= NO_WAVEFORMS; ++i)
        {
                F[i] = (double*) malloc((FREQUENCY_GENERATION + 1) * sizeof(double));
                t[i] = (double*) malloc((FREQUENCY_GENERATION + 1) * sizeof(double));
        }
        
        interpolatedX = (double *)malloc((FREQUENCY_GENERATION + 1) * sizeof(double));
        interpolatedY = (double *)malloc((FREQUENCY_GENERATION + 1) * sizeof(double));

        generateWaveform(fmax, Amax, fs, As, F, t);
        printWaveform(F, t);

        printf("Enter maximum noise value N (Range: (-N,N)): ");
        scanf("%lf", &noise);
        while(noise<0){
                printf("Maximum noise can't be negative. Enter maximum noise value N (Range: (-N,N)): ");
                scanf("%lf", &noise);
        }
        
        
        printf("The maximum frequency of generated wave is %lf\n", MAX_FREQUENCY);

        printf("****Test*Case****\n");
        printf("Enter number of quantization levels: ");
        while(scanf("%d", &num_lvls) != EOF) {

                if(num_lvls<0){
                        printf("Quantization level must be positive value. Enter number of quantization levels (or press Ctrl-D to continue): ");
                        continue;
                }
                printf("Enter sampling frequency:");
                scanf("%lf", &sampling_frequency);
                if(sampling_frequency<0){
                        printf("Sampling frequency must be a positive quantity.\n");
                        printf("Enters the details again. Enter number of quantization levels (or press Ctrl-D to continue): ");
                        continue;
                }
                generateQuantizedWaveform(t[0], F[0], Amax, num_lvls, sampling_frequency, &qx, &qw);
                noisy_qw = addNoise(qx, qw, noise);
                interpolate(qx, noisy_qw, interpolatedX, interpolatedY);
                printf("Mean Squared Error for this sampling is %lf.\n", MeanSquaredError(F[0], interpolatedY));
                free(qw);
                free(qx);
                free(noisy_qw);
                printf("****Test*Case****\n");
                printf("Enter number of quantization levels (or press Ctrl-D to continue): ");
        }

        i = (int)(fmax) - ((int)fmax%10);
        int L = (int) sqrt(1 + ((double)Amax/noise));
        int j = 0;
        double* D;
        D = (double *)malloc(((fmax*3 - i)/20) * sizeof(double));
        for(; i<=(fmax*3); i+=20){
        	generateQuantizedWaveform(t[0], F[0], Amax, L, i, &qx, &qw);
			noisy_qw = addNoise(qx, qw, noise);
			interpolate(qx, noisy_qw, interpolatedX, interpolatedY);
        	D[j] = MeanSquaredError(F[0], interpolatedY);
			free(qw);
        	free(qx);
			free(noisy_qw);
			j++;
        }
        
        int Dindex = 1;
        double Ddiff = 0.0;
        for(i=1; i<j; i++){
                if((D[i] - D[i-1])>Ddiff){
                        Ddiff = D[i] - D[i-1];
                        Dindex = i;
                }
        }
        
        printf("\n****Estimation****\n");
        printf("The estimated maximum frequency is about %lf.\n\n", (fmax + 20 * Dindex) / 2.0 );

	for (i = 0; i <= NO_WAVEFORMS; ++i)
	{
		free(F[i]);
		free(t[i]);
	}
	free(As);free(fs);

	return 0;
}
/* Functions */
void interpolate(double* x, double* y, double* interpolatedX, double* interpolatedY) {
	int i;
	double xi, yi;
	FILE* fp;

	fp = fopen("wInterpol.dat", "w");

	gsl_interp_accel *acc = gsl_interp_accel_alloc ();
	const gsl_interp_type *t = gsl_interp_cspline_periodic;
	gsl_spline *spline = gsl_spline_alloc (t, FREQUENCY_SAMPLING);
	gsl_spline_init (spline, x, y, FREQUENCY_SAMPLING);

	fprintf (fp, "# Interpolated values\n");
	for (i = 0; i <= FREQUENCY_INTERPOLATION; i++)
	{
	  xi = (double) i * x[FREQUENCY_SAMPLING - 1] / FREQUENCY_INTERPOLATION;
	  yi = gsl_spline_eval (spline, xi, acc);
	  fprintf (fp, "%g %g\n", xi, yi);
          interpolatedX[i] = xi;
          interpolatedY[i] = yi;
	}

	gsl_spline_free (spline);
	gsl_interp_accel_free (acc);
	fclose(fp);
}

void generateWaveform(double fmax, double Amax, double* fs, double* As, double** F, double** t) {

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

void printWaveform(double** F, double** t) {
	int i, j;
	FILE* fp;
	char fname[10];

	for (i = 0; i <= NO_WAVEFORMS; ++i)
	{
		sprintf(fname, "w%d.dat", i);
		fp = fopen(fname, "w");

		for (j = 0; j <= FREQUENCY_GENERATION; ++j)
		{
			fprintf (fp, "%g %g\n", t[i][j], F[i][j]);
		}

		fclose(fp);
	}
}

void generateQuantizedWaveform(double* x, double* y, double S, int num_lvls, double sampling_frequency, double** qx, double** qy) {
	int i, j, flag;
	FILE *fp;
	fp = fopen("wQuant.dat", "w");

	double *qw, *t, *lvls, *threshold, k, m;
        FREQUENCY_SAMPLING = (sampling_frequency * 2 )/ MIN_FREQUENCY;
        
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

		fprintf (fp, "%g %g\n", t[i], qw[i]);
                i++;	
	}	

	free(threshold);
	free(lvls);
	fclose(fp);
}

double* addNoise(double* x, double* y, double noise) {
	int i;
	double *noisy_qw;
	FILE *fp;
	fp = fopen("wQuantNoisy.dat", "w");

	noisy_qw = (double*) malloc((FREQUENCY_SAMPLING + 1) * sizeof(double));

	for (i = 0; i <= FREQUENCY_SAMPLING; ++i)
	{
		if(rand() % 2) {
			noisy_qw[i] = y[i] + ( 2.0 * noise * rand() / RAND_MAX  - noise );
		}
		else {
			noisy_qw[i] = y[i];
		}

		fprintf(fp, "%g %g\n", x[i], noisy_qw[i]);
	}

	fclose(fp);
	return noisy_qw;
}

double MeanSquaredError(double* original, double* interpolated){
        double MSE = 0.0;
        int i;
        for(i=0; i<=FREQUENCY_GENERATION; i++){
                MSE+=pow((original[i]-interpolated[i]),2);
        }
        MSE/=(FREQUENCY_GENERATION+1);
        return (sqrt(MSE));
}
