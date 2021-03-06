#include <iostream>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <time.h>
#ifdef _OPENMP
#include <omp.h>
#endif
#define	INDEX(i,j,n) i+j*n

using namespace std;

void initialize(float* x, long n)
{
	for(int i=0;i<n;i++)
	{
		for(int j=0;j<n;j++)
		{
			x[INDEX(i,j,n)] = random()/(float)RAND_MAX;
		}
	}
}

void smooth(float* x, float* y, long n, float a, float b, float c)
{	
	int i=1;
	int j=1;	
	#ifdef _OPENMP
	#pragma omp for private(i,j) //schedule(dynamic,100000)		
	for(i=1; i<n-1; i++){
		for(j=1; j<n-1; j++){
			y[INDEX(i,j,n)] = a*(x[INDEX((i-1),(j-1),n)] + x[INDEX((i-1),(j+1),n)] + x[INDEX((i+1),(j-1),n)] + x[INDEX((i+1),(j+1),n)]) +
					  b*(x[INDEX((i-1),j,n)] + x[INDEX((i+1),j,n)] + x[INDEX(i,(j-1),n)] + x[INDEX(i,(j+1),n)]) + 
					  c*x[INDEX(i,j,n)];
		}
	}
	//cout << "smooth" << endl;	
	#endif
}

void count(float* array, long n, float t,long long &count)
{
	int i,j;
	#ifdef _OPENMP
	#pragma omp for private(i,j) //schedule(dynamic,100000)
	for(i=1; i<n-1; ++i){
		for(j=1; j<n-1; ++j){
			if(array[INDEX(i,j,n)] < t){ // checking if element in x is below threshold value
				count +=1;
			}
		}
	}
	//cout << "count" << endl;
	#endif
}

int main(){

	// Dummy parallel region	
	#ifdef _OPENMP
	#pragma omp parallel
	{
		printf("This is thread %d\n", omp_get_thread_num());
	}
	#endif 
	//reseting random numbers			
	srand(time(0)); 
	
	// timing variable
	clock_t start,end,time_initialize,time_smooth,time_count_x,time_count_y;
	
	// weights
	float a = .05;
	float b = .1;
	float c = .4;

	// dimension of arrays
	long n = 98306; // rows and columns
	
	// element value limit
	float t = .1;
	long long count_x = 0;
	long long count_y = 0;

	// initializing arrays
	float* x = (float*) malloc(n*n*sizeof(long long));
	float* y = (float*) malloc(n*n*sizeof(long long));

	//calling initialize 
	start = clock();
	initialize(x,n);
	end = clock();
	time_initialize = end - start;
	 
	//calling smooth
	start = clock();
	#ifdef _OPENMP
	#pragma omp parallel
	{
		smooth(x,y,n,a,b,c);
	}
	#endif
	end = clock();
	time_smooth = end-start;

	// counting elements below threshold in array x
	start = clock();
	#ifdef _OPENMP
	#pragma omp parallel
	{
		count(x,n,t,count_x);
	}
	#endif 
	end = clock();
	time_count_x = end-start;

	// counting elements below threshold in array y
	start = clock();
	#ifdef _OPENMP
	#pragma omp parallel
	{
		count(y,n,t,count_y);
	}
	#endif
	end = clock();
	time_count_y = end - start;
	
	// calculations
	long Memory = 4*n*n*pow(10,-9);
	float fraction_x = (float) count_x/(n*n);
	float fraction_y = (float) count_y/(n*n);		

	// Summary of results
	cout << "Number of elements in row/column	:: " << n << endl;
	cout << "Number of inner elements in row/column  :: " << n-2<< endl;
	cout << "Total number of elements		:: " << n*n<< endl;
	cout << "Total number of inner elements		:: " << (n-2)*(n-2)<< endl;
	cout << "Memory (GB) used per array		:: " << Memory << endl;
	cout << "Threshold				:: " << t << endl;
	cout << "Smoothing constant (a,b,c)		:: " <<a<<","<<b<<","<<c<< endl;
	cout << "Number of elements below threshold (x)  :: " << count_x << endl;
	cout << "Fraction of elements below threshold    :: " << fraction_x<< endl;
	cout << "Number of elements below threshold (y)  :: " << count_y << endl;
	cout << "Fraction of elements below threshold    :: " << fraction_y<< endl;
	cout << "Timing " <<  endl;
	cout << "Initializing x  :: " << (double)time_initialize/CLOCKS_PER_SEC << endl;
	cout << "Smoothing y     :: " << (double)time_smooth/CLOCKS_PER_SEC << endl;
	cout << "Counting x 	:: " << (double)time_count_x/CLOCKS_PER_SEC << endl;
	cout << "Counting y	:: " << (double)time_count_y/CLOCKS_PER_SEC << endl;

	// Deallocating arrays
	delete[] x;
	delete[] y;
}
