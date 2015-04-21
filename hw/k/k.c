#include "k_engine.h"

void two_norm(int x[dim],int y[dim], int* z)
{
	int inter=0;
	int difference;
	int i;
	dimesnion:
	for (i=0;i<dim;i++)
	{
		difference = x[i]-y[i];
		inter = inter + difference*difference;

	}

	*z=inter;
}

void exponential(float i , float* o  )
{
	float inter = i*-1*inverse_sigma_squared;
	*o=expf(inter);

}
void k_engine(int x[dim],int y[dim],float* out)
{
	int interm,yy;
	//interm=two_norm(x,y,&yy);
	two_norm(x,y,&interm);
	exponential((float)interm,out);

}
