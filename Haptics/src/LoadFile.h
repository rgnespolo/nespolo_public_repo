// ******************* LoadFile.h  ******************************************

// ---------   Class CHECKS, READS and STORES a .WAVE file ----------------- //

#ifndef LOADFILE_H
#define LOADFILE_H


#include <Windows.h>
#include <cstdlib>
#include <list>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <sstream>

class LoadFile{
public:
	// the file path
	FILE *fp;
	// public variables that store header information of the .wave file
	char type[4];
	DWORD size, chunkSize;
	short formatType, channels;
	DWORD sampleRate, avgBytesPerSec;
	short bytesPerSample, bitsPerSample;
	DWORD dataSize;
	// variable that stores the cound data
	unsigned char* buf;

	// constructor 
	LoadFile(const char* filePath);
	// destructor
	~LoadFile();

private:
	// print error report
	void endWithError (char* msg);

};

#endif