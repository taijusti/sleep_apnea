#include "../k/k_inc.h"
#include "../common/common.h"
#include <stdio.h>

float randFloat(void) {
	return (float)rand() / 100.0;
}

 float sw_k(data_t * point1, data_t * point2) {
	 int i;
	 float difference;
	 float result = 0;
	 for (i=0;i<DIMENSIONS;i++)
	 {
		 difference = point1->dim[i]-point2->dim[i];
		 result = result + difference * difference;

	 }
	 result = result*-1*inverse_sigma_squared;
	 return expf(result);
}

int main()
{

//int i[4] = {2,5,3,-1};

		data_t point1;
		data_t point2;
		data_t data[ELEMENTS];
		float k1[ELEMENTS];
		float k2[ELEMENTS];
		unsigned short i, j;
		unsigned int errors = 0;

		// randomly generate point1
		for (i = 0; i < DIMENSIONS; i++) {
			point1.dim[i] = randFloat();
		}

		// randomly generate point2
		for (i = 0; i < DIMENSIONS; i++) {
			point2.dim[i] = randFloat();
		}

		// randomly generate some data
		for (i = 0; i < ELEMENTS; i++) {
			for (j = 0; j < DIMENSIONS; j++) {
				data[i].dim[j] = randFloat();
			}
		}



		// call the module being tested
		k(&point1, &point2, data, k1, k2);

		// test to make sure every k value lines up with the kernel function
		for (i = 0; i < ELEMENTS; i++) {
			if (k1[i] != sw_k(&point1, data + i)) {
				printf("k1 error. Expected: %f Actual: %f\n", sw_k(&point1, data + i), k1[i]);
				errors++;
			}

			if (k2[i] != sw_k(&point2, data + i)) {
				printf("k2 error. Expected: %f Actual: %f\n", sw_k(&point2, data + i), k2[i]);
				errors++;
			}
		}

		if (errors != 0) {
			printf("TEST_FAILED!\n");
		}
		else {
			printf("TEST PASSED!\n");
		}
		return errors;

}
