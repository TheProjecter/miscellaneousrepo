#include "Tcdefs.h"

#ifndef TC_WINDOWS_BOOT
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include "EncryptionThreadPool.h"
#endif

#include "Password.h"
#include <assert.h>

extern int CheckVolumeHeaderPassword (BOOL bBoot, char *encryptedHeader, Password *password);

#define THREADS 4
#define MAX_PW 32768

CRITICAL_SECTION CriticalSection;
char header[512];
Password pw;
unsigned int threads_done = 0;

typedef struct {
	Password* pw;
	unsigned int count;
} thread_arg;

unsigned int __stdcall thread_func(void *arg) {
    unsigned int i;
	thread_arg* targ = (thread_arg*)arg;

	for (i = 0; i < targ->count; i++) {
		int ret = CheckVolumeHeaderPassword(0, header, &targ->pw[i]);
		if (!ret)
			printf("success, password is %s\n", targ->pw[i].Text);
	}

    EnterCriticalSection(&CriticalSection); 
    printf("done\n");
    threads_done++;
    LeaveCriticalSection(&CriticalSection);

    return 0;
}

int main() {
    FILE* fp = fopen("test.tc", "r");
    __int64 ctr1 = 0, ctr2 = 0, freq = 0;
    unsigned int ret, i, j, k;
	Password* passwords[THREADS];
	const char *pws = "words.txt";
	char buffer[MAX_PASSWORD + 1];
	thread_arg* args;

    fgets(header, 512, fp);
    fclose(fp);

    fp = fopen(pws, "r");
	if (fp == NULL)
	{
		perror ("cannot open password file\n");
		exit(-1);
	}

	args = (thread_arg*)malloc(sizeof(thread_arg) * THREADS);
	for (i = 0; i < THREADS; i++) {
		passwords[i] = (Password*)malloc(sizeof(Password) * MAX_PW);
		args[i].pw = passwords[i];
	}


	i = 0;
	while (!feof(fp)) {
		int len;
		fgets (buffer, MAX_PASSWORD + 1, fp);
		len = strlen(buffer);
		// strip new line
		if (buffer[len - 2] == '\r') {
			buffer[len - 2] = 0;
			len -= 2;
		} else if (buffer[len - 1] == '\n') {
			buffer[len - 1] = 0;
			len--;
		}

		strcpy((char*)passwords[i % THREADS][i / THREADS].Text, buffer);
		passwords[i % THREADS][i / THREADS].Length = len;
		i++;

		if (i == MAX_PW) break;
    }
    fclose(fp);
	printf("read %d passwords\n", i);

	for (j = 0; j < THREADS; j++) {
		args[j].count = i / THREADS;
	}

	for (j = i % THREADS; j > 0; )
		for (k = 0; k < THREADS; j--, k++)
		{
			if (!j) break;
			args[k].count++;
		}

    // Initialize the critical section one time only.
    if (!InitializeCriticalSectionAndSpinCount(&CriticalSection, 
        0x80000400) ) 
        return;


    QueryPerformanceCounter((LARGE_INTEGER *)&ctr1);


    for (i = 0; i < THREADS; i++)
        _beginthreadex(NULL, 0, thread_func, (void*)&args[i], 0, &ret);

    while (threads_done < THREADS)
        Sleep(20);

	QueryPerformanceCounter((LARGE_INTEGER *)&ctr2);
	QueryPerformanceFrequency((LARGE_INTEGER *)&freq);

    printf("elapsed: %g\n", ((ctr2 - ctr1) * 1.0 / freq));

    getchar();

    // Release resources used by the critical section object.
    DeleteCriticalSection(&CriticalSection);

    return 0;
}