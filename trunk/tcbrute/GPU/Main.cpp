#include <cutil_inline.h>
#include <cuda_runtime_api.h>
#include <stdint.h>
#include "common.h"
#include "Multithreading.h"

typedef struct {
    //Device id
    int device;

	PTHREAD_RESULT result;
	char *header;
	Password** passwords;

	int rounds;
	int per_round;

	int find_round;
} TGPUplan;

extern "C" void launch_reduceKernel_ripemd160(char* d_EncryptedHeader, Password* d_Input, PTHREAD_DATA d_Output, int BLOCK_N, int THREAD_N);
extern "C" void launch_reduceKernel_sha512(char* d_EncryptedHeader, Password* d_Input, PTHREAD_DATA d_Output, int BLOCK_N, int THREAD_N);
extern "C" void launch_reduceKernel_sha1(char* d_EncryptedHeader, Password* d_Input, PTHREAD_DATA d_Output, int BLOCK_N, int THREAD_N);
extern "C" void launch_reduceKernel_whirlpool(char* d_EncryptedHeader, Password* d_Input, PTHREAD_DATA d_Output, int BLOCK_N, int THREAD_N);
extern "C" void launch_reduceKernel_final(char *d_EncryptedHeader, PTHREAD_DATA d_Input, PTHREAD_RESULT d_Output, int BLOCK_N, int THREAD_N);

////////////////////////////////////////////////////////////////////////////////
// Data configuration
////////////////////////////////////////////////////////////////////////////////
#define MAX_GPU_COUNT 32
#define BLOCK_N 32
#define THREAD_N 128
#define PER_ROUND (BLOCK_N * THREAD_N)
#define PASSWORD_SETS 100 // maximum allocated password sets

////////////////////////////////////////////////////////////////////////////////
// GPU thread
////////////////////////////////////////////////////////////////////////////////
static CUT_THREADPROC solverThread(TGPUplan *plan){
    int i;
	PTHREAD_DATA d_Data;
	PTHREAD_DATA h_Data;

	char* d_EncryptedHeader;
	Password* d_Input;
	PTHREAD_RESULT d_Result;

    //Set device
    cutilSafeCall( cudaSetDevice(plan->device) );

	cutilSafeCall( cudaMalloc((void**)&d_Data, PER_ROUND * sizeof(THREAD_DATA)) );

    cutilSafeCall( cudaMalloc((void**)&d_EncryptedHeader, TC_VOLUME_HEADER_EFFECTIVE_SIZE * sizeof(char)) );
	cutilSafeCall( cudaMalloc((void**)&d_Input, PER_ROUND * sizeof(Password)) );
	cutilSafeCall( cudaMalloc((void**)&d_Result, sizeof(THREAD_RESULT)) );

    //Copy input data from CPU
	cutilSafeCall( cudaMemcpy(d_EncryptedHeader, plan->header, TC_VOLUME_HEADER_EFFECTIVE_SIZE * sizeof(char), cudaMemcpyHostToDevice) );
	cutilSafeCall( cudaMemcpy(d_Result, plan->result, sizeof(THREAD_RESULT), cudaMemcpyHostToDevice) );

	for (i = 0; i < plan->rounds; i++) {
		// load passwords for this round
		cutilSafeCall( cudaMemcpy(d_Input, plan->passwords[plan->device * plan->rounds + i],
			plan->per_round * sizeof(Password), cudaMemcpyHostToDevice) );

		// launch the 5 kernels for each set of passwords
		launch_reduceKernel_ripemd160(d_EncryptedHeader, d_Input, d_Data, BLOCK_N, THREAD_N);
		cutilCheckMsg("reduceKernel_ripemd160() execution failed.\n");/*
		launch_reduceKernel_sha512(d_EncryptedHeader, d_Input, d_Data, BLOCK_N, THREAD_N);
		cutilCheckMsg("reduceKernel_sha512() execution failed.\n");*/
		launch_reduceKernel_sha1(d_EncryptedHeader, d_Input, d_Data, BLOCK_N, THREAD_N);
		cutilCheckMsg("reduceKernel_sha1() execution failed.\n");
		launch_reduceKernel_whirlpool(d_EncryptedHeader, d_Input, d_Data, BLOCK_N, THREAD_N);
		cutilCheckMsg("reduceKernel_whirlpool() execution failed.\n");
		launch_reduceKernel_final(d_EncryptedHeader, d_Data, d_Result, BLOCK_N, THREAD_N);
		cutilCheckMsg("reduceKernel_final() execution failed.\n");

		//Read back GPU results
		cutilSafeCall( cudaMemcpy(plan->result, d_Result, sizeof(THREAD_RESULT), cudaMemcpyDeviceToHost) );
		// TODO: this solver thread should synchronize with other threads, (i.e. lock on critical section), and use a variable to
		// check if any other threads have already found the password, if so stop them too
		if (plan->result->tid != -1) {
			// password found, stop thread
			plan->find_round = i;
			break;
		}
	}

	cutilSafeMalloc(h_Data = (PTHREAD_DATA)malloc(sizeof(THREAD_DATA)));
	cutilSafeCall( cudaMemcpy(h_Data, d_Data, sizeof(THREAD_DATA), cudaMemcpyDeviceToHost) );

    //Shut down this GPU
    cutilSafeCall( cudaFree(d_Data) );
    cutilSafeCall( cudaFree(d_EncryptedHeader) );
    cutilSafeCall( cudaFree(d_Input) );
    cutilSafeCall( cudaFree(d_Result) );
	free(h_Data);

    CUT_THREADEND;
}

////////////////////////////////////////////////////////////////////////////////
// Program main
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv){
    char header[TC_VOLUME_HEADER_EFFECTIVE_SIZE];
	char* container = "test.tc";
	char* pws = "words.txt";
	Password** passwords;
	cutilSafeMalloc(passwords = (Password**)malloc(sizeof(Password*) * PASSWORD_SETS));
	int i, s = -1, p = 0, pw_count;
	char buffer[MAX_PASSWORD + 1];

    int GPU_N;
    unsigned int hTimer;

    //Solver config
    TGPUplan      plan[MAX_GPU_COUNT];

    //OS thread ID
    CUTThread threadID[MAX_GPU_COUNT];

    // read the header from the TC container
	// TODO: add command line arguments (for now this works)
    FILE* fp = fopen(container, "r");
	if (fp == NULL)
	{
		perror ("cannot open TC container\n");
		exit(-1);
	}
    fgets(header, TC_VOLUME_HEADER_EFFECTIVE_SIZE, fp);
    fclose(fp);

    // read passwords from file
	// TODO: should do optimal parallelization for any number of passwords (i.e. read the file, count passwords,
	// then determine how much each set will have based on THREADS and GPU_N;
	// 1. per_gpu = (pw_count / gpu_n);
	// 2. get closest product of two powers of two (will be thread and block counts)
	// 3. diff = per_gpu - closest; for diff threads, do 2 passwords instead of 1, so dk should be multidimensional, so should passwords

	// thread count needs to be power of two, so does block count

	// for now i'll just try with 32768 passwords,16k passwords per gpu (2 rounds)
	// the password we're looking for should be the last one
    fp = fopen(pws, "r");
	if (fp == NULL)
	{
		perror ("cannot open password file\n");
		exit(-1);
	}
	while (!feof(fp)) {
		if (p == 0 || p == PER_ROUND) {
			// move on to next set
			p = 0;
			s++;
		}

		if (!p) {
			cutilSafeMalloc(passwords[s] = (Password*)malloc(sizeof(Password) * PER_ROUND));
		}

		fgets (buffer, MAX_PASSWORD + 1, fp);
		int len = strlen(buffer);
		// strip new line
		if (buffer[len - 2] == '\r') {
			buffer[len - 2] = 0;
			len -= 2;
		} else if (buffer[len - 1] == '\n') {
			buffer[len - 1] = 0;
			len--;
		}

		strcpy((char*)passwords[s][p].Text, buffer);
		passwords[s][p].Length = len;
		p++;
    }
    fclose(fp);
	pw_count = s * PER_ROUND + p;
	printf("read %d passwords\n", pw_count);

    cutilCheckError(cutCreateTimer(&hTimer));

    cutilSafeCall(cudaGetDeviceCount(&GPU_N));
    if(GPU_N > MAX_GPU_COUNT) GPU_N = MAX_GPU_COUNT;
    printf("CUDA-capable device count: %i\n", GPU_N);
	for (i = 0; i < GPU_N; i++) {
		cutilSafeMalloc(plan[i].result = (PTHREAD_RESULT)malloc(sizeof(THREAD_RESULT)));
		plan[i].result->tid = -1;
		plan[i].device = i;
		plan[i].header = header;
		plan[i].passwords = passwords;
		plan[i].rounds = pw_count / (PER_ROUND * GPU_N);
		plan[i].per_round = PER_ROUND;
	}

    //Start timing and compute on GPU(s)
    printf("Computing with %d GPU's...\n", GPU_N);
    cutilCheckError(cutResetTimer(hTimer));
    cutilCheckError(cutStartTimer(hTimer));
    for(i = 0; i < GPU_N; i++)
    {
            threadID[i] = cutStartThread((CUT_THREADROUTINE)solverThread, (void *)(plan + i));
    }
    cutWaitForThreads(threadID, GPU_N);

	// check if password was found
	int found = 0;
	for (i = 0; i < GPU_N; i++) {
		if (plan[i].result->tid != -1) {
			printf("success, password = %s, ea = %d, mode = %d, prf = %d\n", passwords[i * plan[i].rounds + plan[i].find_round][plan[i].result->tid].Text,
				plan[i].result->ea, plan[i].result->mode, plan[i].result->prf);
			found = 1;
			break;
		}
	}
	if (!found) {
		printf("password bruteforcing failed\n");
	}

    cutilCheckError(cutStopTimer(hTimer));
    printf("  GPU Processing time: %f (ms)\n\n", cutGetTimerValue(hTimer));

    // Cleanup and shutdown
    printf("Shutting down...\n");
    cutilCheckError(cutDeleteTimer(hTimer));

	for (i = 0; i <= s; i++)
		free(passwords[i]);

	free(passwords);

	for (i = 0; i < GPU_N; i++)
		free(plan[i].result);

    cudaThreadExit();
    cutilExit(argc, argv);
}
