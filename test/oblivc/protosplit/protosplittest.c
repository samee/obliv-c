#include "protosplittest.h"

void parallelize(split_fn fn, void * output1, void * output2, void * output3, uint32_t * input, int leneach, void * pd1, void* pd2) {
	#pragma omp parallel num_threads(3)
	{
		//OpenMP seems to get along with obliv-c just fine, so long as obliv-c only uses the master thread.
		#pragma omp master
		{
			fn(output1, input, leneach, NULL);
		}
		

		#pragma omp single
		{
			#pragma omp task
			fn(output2, &input[leneach], leneach, pd1);

			#pragma omp task
			fn(output3, &input[2*leneach], leneach, pd2);
		}
	}
}