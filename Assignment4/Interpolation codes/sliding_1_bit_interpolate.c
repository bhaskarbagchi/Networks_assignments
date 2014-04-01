#include <stdio.h>

#define P_MIN 10
#define P_MAX 26

#define FRAME_SZ_MIN 32
#define FRAME_SZ_MAX 1024

double Spline_evaluation(double x_in, double y_in);

int main(int argc, char *argv[])
{
    double P_diff, FRAME_SZ_diff;

    printf("\n");

    P_diff =  (double)(P_MAX - P_MIN) / 100;
    FRAME_SZ_diff = (double) (FRAME_SZ_MAX - FRAME_SZ_MIN) / 100;

    double p, frame_sz;

    for (p = P_MIN; p <= P_MAX; p += P_diff)
    {
        for (frame_sz = FRAME_SZ_MIN; frame_sz <= FRAME_SZ_MAX; frame_sz += FRAME_SZ_diff)
        {
            printf("%g\t%g\t%g\n", p, frame_sz, Spline_evaluation(p, frame_sz));
        }
    }

    return 0;
}
double Spline_evaluation(double x_in, double y_in)
{
    double tx [] = {1.0000000000000000E+01, 1.0000000000000000E+01, 1.0000000000000000E+01, 2.6000000000000000E+01, 2.6000000000000000E+01, 2.6000000000000000E+01};
    double ty [] = {3.2000000000000000E+01, 3.2000000000000000E+01, 3.2000000000000000E+01, 1.0240000000000000E+03, 1.0240000000000000E+03, 1.0240000000000000E+03};
    double coeff [] = {2.8287602360463256E+02, 4.1929645496255816E+03, 7.8376522637672015E+03, 2.8071417079056249E+02, 3.5857065115448754E+03, 9.0914358298293428E+03, 2.4379856883815913E+02, 4.4196659304639425E+03, 8.1753822993312260E+03};
    int nx = 6;
    int ny = 6;
    int kx = 2;
    int ky = 2;

    double h [] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    double hh [] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    double w_x [] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    double w_y [] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

    int i, j, li, lj, lx, ky1, nky1, ly, i1, j1, l2;
    double f, temp;

    int kx1 = kx+1;
    int nkx1 = nx-kx1;
    int l = kx1;
    int l1 = l+1;

    while ((x_in >= tx[l1-1]) && (l != nkx1))
    {
        l = l1;
        l1 = l+1;
    }
    
    h[0] = 1.0;
    for (j = 1; j < kx+1; j++)
    {
        for (i = 0; i < j; i++)
        {
            hh[i] = h[i];
        }
        h[0] = 0.0;
        for (i = 0; i < j; i++)
        {
            li = l+i;
            lj = li-j;
            if (tx[li] != tx[lj])
            {
                f = hh[i] / (tx[li] - tx[lj]);
                h[i] = h[i] + f * (tx[li] - x_in);
                h[i+1] = f * (x_in - tx[lj]);
            }
            else
            {
                h[i+1-1] = 0.0;
            }
        }
    }
    
    lx = l-kx1;
    for (j = 0; j < kx1; j++)
    {
        w_x[j] = h[j];
    }

    ky1 = ky+1;
    nky1 = ny-ky1;
    l = ky1;
    l1 = l+1;

    while ((y_in >= ty[l1-1]) && (l != nky1))
    {
        l = l1;
        l1 = l+1;
    }
    
    h[0] = 1.0;
    for (j = 1; j < ky+1; j++)
    {
        for (i = 0; i < j; i++)
        {
            hh[i] = h[i];
        }
        h[0] = 0.0;
        for (i = 0; i < j; i++)
        {
            li = l+i;
            lj = li-j;
            if (ty[li] != ty[lj])
            {
                f = hh[i] / (ty[li] - ty[lj]);
                h[i] = h[i] + f * (ty[li] - y_in);
                h[i+1] = f * (y_in - ty[lj]);
            }
            else
            {
                h[i+1-1] = 0.0;
            }
        }
    }

    ly = l-ky1;
    for (j = 0; j < ky1; j++)
    {
        w_y[j] = h[j];
    }

    l = lx*nky1;
    for (i1 = 0; i1 < kx1; i1++)
    {
        h[i1] = w_x[i1];
    }
        
    l1 = l+ly;
    temp = 0.0;
    for (i1 = 0; i1 < kx1; i1++)
    {
        l2 = l1;
        for (j1 = 0; j1 < ky1; j1++)
        {
            l2 = l2+1;
            temp = temp + coeff[l2-1] * h[i1] * w_y[j1];
        }
        l1 = l1+nky1;
    }
        
    return temp;
}
