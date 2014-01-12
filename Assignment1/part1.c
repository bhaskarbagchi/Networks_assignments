#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>

#define FREQUENCY_INTERPOLATION 500
#define FREQUENCY_GENERATION 500
#define FREQUENCY_SAMPLING 500
#define NO_WAVEFORMS 5
#define PI 3.14159265358979323846

/* Globals */
double MAX_FREQUENCY;
double MIN_FREQUENCY;


/* Function prototypes */
void generatePlot(FILE* fp, double* x, double* y, int N);
void generateWaveform(double fmax, double Amax, double* fs, double* As, double** F, double** t);
void printWaveform(double** F, double** t);
double* generateQuantizedWaveform(double* x, double* y, double S, int num_lvls);
double* addNoise(double* x, double* y, double noise);
/* Main */
int main(int argc, char const *argv[])
{
	double fmax, Amax, noise, *As, *fs, **F, **t, *qw, *noisy_qw;
	int i, num_lvls;

	printf("Enter maximum frequency (in Hz): ");
	scanf("%lf", &fmax);

	printf("Enter maximum amplitude (max. signal strength): ");
	scanf("%lf", &Amax);

	As = (double*) malloc((NO_WAVEFORMS + 1) * sizeof(double));
	fs = (double*) malloc((NO_WAVEFORMS + 1) * sizeof(double));

	F = (double**) malloc((NO_WAVEFORMS + 1) * sizeof(double*));
	t = (double**) malloc((NO_WAVEFORMS + 1) * sizeof(double*));

	for (i = 0; i <= NO_WAVEFORMS; ++i)
	{
		F[i] = (double*) malloc((FREQUENCY_GENERATION + 1) * sizeof(double));
		t[i] = (double*) malloc((FREQUENCY_GENERATION + 1) * sizeof(double));
	}

	generateWaveform(fmax, Amax, fs, As, F, t);
	printWaveform(F, t);

	printf("Enter maximum noise value N (Range: (-N,N)): ");
	scanf("%lf", &noise);

	printf("Enter number of quantization levels: ");
	while(scanf("%d", &num_lvls) != EOF) {

		qw = generateQuantizedWaveform(t[0], F[0], Amax, num_lvls);
		noisy_qw = addNoise(t[0], qw, noise);

		free(qw);
		free(noisy_qw);
		printf("Enter number of quantization levels (or press Ctrl-D to exit): ");
	}

	for (i = 0; i <= NO_WAVEFORMS; ++i)
	{
		free(F[i]);
		free(t[i]);
	}
	free(As);free(fs);

	return 0;
}
/* Functions*/
void generatePlot(FILE* fp, double* x, double* y, int N) {
	gsl_interp_accel *acc = gsl_interp_accel_alloc ();
	const gsl_interp_type *t = gsl_interp_cspline_periodic; 
	gsl_spline *spline = gsl_spline_alloc (t, N);

	int i; double xi, yi;

	fprintf (fp, "# Discrete values\n");
	for (i = 0; i < N; i++)
	{
	  fprintf (fp, "%g %g\n", x[i], y[i]);
	}

	fprintf (fp, "# Interpolated values\n");
	gsl_spline_init (spline, x, y, N);

	for (i = 0; i <= FREQUENCY_INTERPOLATION; i++)
	{
	  xi = (1 - i / FREQUENCY_INTERPOLATION) * x[0] + (i / FREQUENCY_INTERPOLATION) * x[N-1];
	  yi = gsl_spline_eval (spline, xi, acc);
	  fprintf (fp, "%g %g\n", xi, yi);
	}

	gsl_spline_free (spline);
	gsl_interp_accel_free (acc);
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

double* generateQuantizedWaveform(double* x, double* y, double S, int num_lvls) {
	int i, j, flag;
	FILE *fp;
	fp = fopen("wQuant.dat", "w");

	double *qw, *lvls, *threshold;

	qw = (double*) malloc((FREQUENCY_SAMPLING + 1) * sizeof(double));
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

	for (i = 0; i <= FREQUENCY_SAMPLING; ++i)
	{
		flag = 1;
		for (j = 0; j < num_lvls - 1; ++j) {
			if( y[i] < threshold[j] ) {
				qw[i] = lvls[j];
				flag = 0;
				break;
			}
		}
		if(flag) {
			qw[i] = lvls[num_lvls - 1];
		}

		fprintf (fp, "%g %g\n", x[i], qw[i]);	
	}	

	free(threshold);
	free(lvls);
	fclose(fp);

	return qw;
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