Description
============

This is a fairly simple attempt at parallelizing the truecrypt decryption process so that it may be bruteforced on the GPU.

When bruteforcing a TrueCrypt container, each PRF (pseudorandom function) and EA (encryption algorithm) is tried using the provided password until the volume has been successfully decrypted; this is checked by verifying if a magic sequence is present in the volume header.

Out of the PRFs, SHA512 and WHIRLPOOL are very slow on the GPU and will cause CUDA to give an error and out of the EAs, TWOFISH is currently crashing for an unknown reason.

LAST_PRF_ID and EncryptionAlgorithms may be edited in common.h to select the PRFs and algorithms that will be tried when bruteforcing.


analysis.txt contains a comparison of the bruteforce speed on the GPU vs the CPU.


Building
========

words.txt contains 32768 passwords, the correct one is the last.
GPU/ contains code for bruteforcing the container (test.tc) on the GPU. To compile it, copy the files to a directory in the CUDA sdk samples directory and run make from there. This should compile on both Linux/Windows. On Windows you may have to increase the stack size for be.exe or it will crash.
CPU/ contains code for multithreaded bruteforcing on the CPU. sln file can be opened in VS2008.


Notes
=====

The GPU code should be run on a dedicated videocard, if run on the videocard connected to the display it may freeze/crash the system.