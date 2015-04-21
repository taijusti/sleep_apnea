#include "k_engine.h"
#include <stdio.h>


int main()
{

int x[4] = {2,5,3,-1};
int y[4] = {5,7,-4,-1};

float out_actual;

int i;
k_engine(x,y,&out_actual);
int difference;
int total =0;
for (i=0;i<dim;i++)
{

difference = x[i]-y[i];
total = total + difference*difference;

}
 float xx = -1.0*inverse_sigma_squared*(float)total;
float out_theor = exp(xx);

if (out_theor!=out_actual)
{
	printf("%f %f fail %f\n", out_theor,xx,out_actual);
	return -1;

}
else
{
	printf("%f success %f\n", out_theor,out_actual);
	return 0;

}





}
