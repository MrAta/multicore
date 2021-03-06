#include <iostream>
#include <string>
#include <fstream>
#include <cstdint>
#include <vector>
//#include <cstdlib.h>
#include "wavfile.h"
#include "bp3-7.h"
#include "bs10-15.h"
#include "hp7000.h"
#include "lp8000.h"
using std::vector;
using std::cin;
using std::cout;
using std::endl;
using std::fstream;
using std::string;

typedef struct  WAV_HEADER
{
    /* RIFF Chunk Descriptor */
    uint8_t         RIFF[4];        // RIFF Header Magic header
    uint32_t        ChunkSize;      // RIFF Chunk Size
    uint8_t         WAVE[4];        // WAVE Header
    /* "fmt" sub-chunk */
    uint8_t         fmt[4];         // FMT header
    uint32_t        Subchunk1Size;  // Size of the fmt chunk
    uint16_t        AudioFormat;    // Audio format 1=PCM,6=mulaw,7=alaw,     257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
    uint16_t        NumOfChan;      // Number of channels 1=Mono 2=Sterio
    uint32_t        SamplesPerSec;  // Sampling Frequency in Hz
    uint32_t        bytesPerSec;    // bytes per second
    uint16_t        blockAlign;     // 2=16-bit mono, 4=16-bit stereo
    uint16_t        bitsPerSample;  // Number of bits per sample
    /* "data" sub-chunk */
    uint8_t         Subchunk2ID[4]; // "data"  string
    uint32_t        Subchunk2Size;  // Sampled data length
} wav_hdr;

// Function prototypes
int getFileSize(FILE* inFile);
vector <uint16_t> _dataBytes;
void readInputwavedata(const char*);
void calculateOutputDataVector(int AL, const double A[], short outData[]);
void writeOutputFile(short outData[],const char*);
int main(int argc, char* argv[])
{
    const char* filePath;
    const char* outFilePath;
    int filter;
	filePath = argv[1];
	outFilePath = argv[2];
	filter = atoi(argv[3]);
    cout << "Input wave file name: " << filePath << endl;
    cout << "Input wave file name: " << outFilePath << endl;
    cout << "Selected filter: " << filter << endl;
    cout <<"Reading "<<filePath<<" ..."<<endl;
    readInputwavedata(filePath);
    cout <<"Reading "<<filePath<<" done!"<<endl;
    cout <<"data SizeL: "<<_dataBytes.size()<<endl;    
    short *_outDataBytes = (short*)malloc(_dataBytes.size()*sizeof(int));
    cout <<"defined _outDataBytes"<<endl;
    switch(filter){
    	case 1:
    		cout << "Calculating output data vector with Filter: bp3-7" <<endl;
    		calculateOutputDataVector(AL1,A1,_outDataBytes);
    		cout << "Calculating output data vector with Filter bp3-7 done!" <<endl;
    		break;
    	case 2:
    		cout << "Calculating output data vector with Filter: bs10-15" <<endl;
    		calculateOutputDataVector(AL2,A2,_outDataBytes);
    		cout << "Calculating output data vector with Filter bs10-15 done!" <<endl;
    		break;
    	case 3:    		
    		cout << "Calculating output data vector with Filter: hp7000" <<endl;
    		calculateOutputDataVector(AL3,A3, _outDataBytes);
    		cout << "Calculating output data vector with Filter hp7000 done!" <<endl;    		
    		break;
    	case 4:
    		cout << "Calculating output data vector with Filter: lp8000" <<endl;
    		calculateOutputDataVector(AL4,A4, _outDataBytes);
    		cout << "Calculating output data vector with Filter lp8000 done!" <<endl;    		
    		break;
	}
	cout<<"Writing to "<<outFilePath<<" ..."<<endl;
	writeOutputFile(_outDataBytes,outFilePath);
	cout<<"Writing to "<<outFilePath<<" done!"<<endl;
    return 0;
}

void writeOutputFile( short outData[],const char* outFilePath){

	FILE * f = wavfile_open(outFilePath);
	if(!f) {
		cout<<"couldn't open "<<outFilePath<<" for writing: "<<errno<<endl;
		return;
	}

	wavfile_write(f,outData,_dataBytes.size());
	wavfile_close(f);	
		
}
void calculateOutputDataVector(int AL,const double A[], short outData[]){
	for(int i=0; (unsigned)i<_dataBytes.size(); i++ )
	{
		outData[i] = 0;
		for(int j=0; j<AL; j++)
		{			
			if(i - j >= 0)
				outData[i] += A[j]*_dataBytes.at(i-j);
		}
	}
	
			
}
void readInputwavedata(const char* filePath){
	
	wav_hdr wavHeader;
    int headerSize = sizeof(wav_hdr), filelength = 0;
	FILE* wavFile = fopen(filePath, "r");
    if (wavFile == nullptr)
    {
        fprintf(stderr, "Unable to open wave file: %s\n", filePath);
        return;
    }

    //Read the header
    size_t bytesRead = fread(&wavHeader, 1, headerSize, wavFile);
    cout << "Header Read " << bytesRead << " bytes." << endl;
    if (bytesRead > 0)
    {
        //Read the data
        uint16_t bytesPerSample = wavHeader.bitsPerSample / 8;      //Number     of bytes per sample
        uint64_t numSamples = wavHeader.ChunkSize / bytesPerSample; //How many samples are in the wav file?
        static const uint16_t BUFFER_SIZE = 4096;
        uint16_t* buffer = new uint16_t[BUFFER_SIZE];

        while ((bytesRead = fread(buffer, sizeof buffer[0], BUFFER_SIZE / (sizeof buffer[0]), wavFile)) > 0)
        {
            /** DO SOMETHING WITH THE WAVE DATA HERE **/
            for(int i=0; (unsigned)i < BUFFER_SIZE/(sizeof buffer[0]); i++ )
            	_dataBytes.push_back(buffer[i]);
            //cout << "Read " << bytesRead << " bytes." << endl;
        }
        delete [] buffer;
        buffer = nullptr;
        filelength = getFileSize(wavFile);

        cout << "File is                    :" << filelength << " bytes." << endl;
        cout << "RIFF header                :" << wavHeader.RIFF[0] << wavHeader.RIFF[1] << wavHeader.RIFF[2] << wavHeader.RIFF[3] << endl;
        cout << "WAVE header                :" << wavHeader.WAVE[0] << wavHeader.WAVE[1] << wavHeader.WAVE[2] << wavHeader.WAVE[3] << endl;
        cout << "FMT                        :" << wavHeader.fmt[0] << wavHeader.fmt[1] << wavHeader.fmt[2] << wavHeader.fmt[3] << endl;
        cout << "Data size                  :" << wavHeader.ChunkSize << endl;

        // Display the sampling Rate from the header
        cout << "Number of Samples              :" << numSamples << endl;
        cout << "Sampling Rate              :" << wavHeader.SamplesPerSec << endl;
        cout << "Number of bits used        :" << wavHeader.bitsPerSample << endl;
        cout << "Number of channels         :" << wavHeader.NumOfChan << endl;
        cout << "Number of bytes per second :" << wavHeader.bytesPerSec << endl;
        cout << "Data length                :" << wavHeader.Subchunk2Size << endl;
        cout << "Audio Format               :" << wavHeader.AudioFormat << endl;
        // Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM

        cout << "Block align                :" << wavHeader.blockAlign << endl;
        cout << "Data string                :" << wavHeader.Subchunk2ID[0] << wavHeader.Subchunk2ID[1] << wavHeader.Subchunk2ID[2] << wavHeader.Subchunk2ID[3] << endl;
//    	for(int i=0; i<_dataBytes.size(); i++)
//    		cout<<i<<":"<<_dataBytes.at(i)<<endl;
    }
    fclose(wavFile);

}
// find the file size
int getFileSize(FILE* inFile)
{
    int fileSize = 0;
    fseek(inFile, 0, SEEK_END);

    fileSize = ftell(inFile);

    fseek(inFile, 0, SEEK_SET);
    return fileSize;
}
