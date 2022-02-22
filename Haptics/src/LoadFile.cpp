#include "LoadFile.h"

LoadFile::LoadFile(const char* filePath){

	// -----  Open file to read  ----------//
	fp = fopen(filePath, "rb");
	if (fp == NULL)
		std:: cout << " FIle not loaded" << std::endl;

	// ----  Check if file is Wave  ------ //
	fread(type,sizeof(char),4,fp);
	if (type[0] != 'R' || type[1] != 'I' || type[2] != 'F' || type[3] != 'F')
		LoadFile::endWithError ("NOT RIFF");

	fread(&size,sizeof(DWORD),1,fp);
	fread(type, sizeof(char),4,fp);
	if (type[0] != 'W' || type[1] != 'A' || type[2] != 'V' || type[3] != 'E')
		LoadFile::endWithError ("NOT WAVE");

	fread(type, sizeof(char),4,fp);
	if (type[0] != 'f' || type[1] != 'm' || type[2] != 't' || type[3] != ' ')
		LoadFile::endWithError ("NOT fmt");
	// ------------------------------------//

	// ------ read the chunk size (not important) ---------//
	fread (&chunkSize, sizeof(DWORD) , 1 , fp);

	// -------- store header information of the wave file ----------//
	fread(&formatType, sizeof(short) , 1 , fp);
	fread(&channels, sizeof(short) , 1 , fp);
	fread(&sampleRate, sizeof(DWORD) , 1 , fp);
	fread(&avgBytesPerSec, sizeof(DWORD) , 1 , fp);
	fread(&bytesPerSample, sizeof(short) , 1 , fp);
	fread(&bitsPerSample, sizeof(short) , 1 , fp);
	// ------------------------------------------------------------//

	// ----------- check and store the size of data ---------------//
	fread(type,sizeof(char),4,fp);
	if(type[0] != 'd' || type[1] != 'a' || type[2] != 't' || type[3] != 'a')
		LoadFile::endWithError("Data Missing");
	fread(&dataSize,sizeof(DWORD),1,fp);
	// -----------------------------------------------------------//

	// ------------ store data ---------------------//
	buf = new unsigned char[dataSize];
	fread(buf, sizeof(BYTE), dataSize,fp);
	// --------------------------------------------//
}

void LoadFile::endWithError(char* msg)
{
	// delete pointer 
	std::cout << msg << std::endl;
	system ("PAUSE");
}


LoadFile::~LoadFile(){
	if (buf != NULL)
		delete buf;

	fclose(fp); 
}

