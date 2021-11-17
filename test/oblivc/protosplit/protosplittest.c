#include "protosplittest.h"

void parallelize(split_fn fn, void * output, uint32_t * input, size_t leneach, size_t threads, void * pds) {
	omp_set_dynamic(0);
	omp_set_num_threads(threads);
	#pragma omp parallel
	{
		#pragma omp single
		{
			size_t ii;
			for (ii = 0; ii < threads; ii++){
				#pragma omp task
				fn(output, input, leneach, ii, pds);	
			}
		}
	}
}