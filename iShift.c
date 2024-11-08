#include <stdio.h>
#include <string.h>
#include <portaudio.h>
#include <sndfile.h>
#include <math.h>
#include <soundpipe.h>
#include <time.h>
#include <stdlib.h>

//------------------------------------------------------------------------------------
//Constants
#define kDeviceIndex 1 //Built-in Output
#define kNumFramesPerBuffer  512
#define kGain 100.0
#define kLevel 0.1
#define kOutputFileName "PitchShift.wav"

unsigned long glength;
float gratio = 1.0;
float ginBuffer[2646000];
float pBuffer[2646000];
float writeBuffer[2646000];
int hClip = 0;
//------------------------------------------------------------------------------------
//Declare user data that holds SNDFILE and SF_INFO
//so that we can use them inside audio render callback
typedef struct SoundFile {
  SNDFILE *file;
  SF_INFO info;
} SoundFile;

typedef struct {
    sp_pshift *pshift;
    sp_diskin *diskin;
} UserData;
//------------------------------------------------------------------------------------
//Function prototypes
int createOutputFile(SoundFile *inFile, SoundFile *outFile);
int openread(SoundFile *sndFile, char *fileName);
int initPortAudio();
int closePortAudio();
void printPaDevices();
void passBuffer(float *buffer, unsigned long numFrames, float *insample);
void passBuffer2(float *buffer, unsigned long numFrames, float *insample);
void drywet(float *buffer, unsigned long numFrames, float *insample, float *dryBuffer, float ratio);
void hardclip(float *buffer, unsigned long numFrames, float *insample);
void procpitch(float *out, void *udata, sp_data *sp, long n);
void pshifter(char *fileName, int shiftval);
//------------------------------------------------------------------------------------
//Audio render callback function
int renderCallback(
  const void *input,
  void *output,
  unsigned long frameCount,
  const PaStreamCallbackTimeInfo* timeInfo,
  PaStreamCallbackFlags statusFlags,
  void *userData 
);
//------------------------------------------------------------------------------------
int main(){
  PaStream *pStream = NULL; //For port audio streaming
	PaStreamParameters outputParameters; //Parameters for output of a stream
  SoundFile sndFile; //Used for audio render callback
  SoundFile outFile;
  char fileName[20]; //empty string container for filename to play
  char command[20];
  int shiftv;
  //Open sound file for a playback
  printf("Select file to play: ");
  scanf("%s", fileName);
  printf("Select ratio(0.0-1.0): ");
  scanf("%f", &gratio);
  printf("Select ratio(-24 - 24): ");
  scanf("%i", &shiftv);
  printf("Hard Clip?(0 for no, 1 for yes): ");
  scanf("%i", &hClip);
  
   //Initialize port audio
  if(initPortAudio()) return 1;

  //Print available audio devices
  printPaDevices();
  int snderror =openread(&sndFile, fileName);
  if(snderror) return 1;
  snderror = createOutputFile(&sndFile, &outFile);
  if(snderror) return 1;
  pshifter(fileName, shiftv);



  //Configure port audio streaming setup
  memset(&outputParameters, 0, sizeof(PaStreamParameters));
  outputParameters.channelCount = sndFile.info.channels;
	outputParameters.device = kDeviceIndex;
	outputParameters.sampleFormat = paFloat32;
	outputParameters.suggestedLatency = 0.0;

  //Open port audio streaming
  PaError error = Pa_OpenStream(
    &pStream,
    NULL, //no input
    &outputParameters, //output
    sndFile.info.samplerate,
    kNumFramesPerBuffer, //frames per buffer
    paNoFlag,
    renderCallback,
    ginBuffer //User data
  );

  //Check for error in opening port audio streaming
  if(error != paNoError){
    printf("Error: Failed to open port audio stream. %s\n",Pa_GetErrorText(error));
    closePortAudio();
    return 1;
  }

  //Start port audio streaming
  error = Pa_StartStream(pStream);
  if(error != paNoError){
    printf("Error: Failed to start port audio stream. %s\n",Pa_GetErrorText(error));
    return 0;
  }
  else {
    printf("Rendering audio...\n");
    while(1){
      printf("Enter one of following command\n");
      printf("'ratio': change dry/wet ratio      \t 'shift': change pitch shift value\n");
      printf("'clip': toggle on/off hard clipping\t 'print': print selected effects and exit program\n");
      printf("'exit': exit the program\n: ");
      scanf("%s", command);
      if(strcmp(command, "ratio") == 0){
        printf("Input ratio(0.0-1.0): ");
        scanf("%f", &gratio);
      } else if(strcmp(command, "shift") == 0){
        printf("Input shift value(-24 - 24): ");
        scanf("%i", &shiftv);
        pshifter(fileName, shiftv);
      } else if(strcmp(command, "clip") == 0){
        /*printf("Hard Clip?(0 for no, 1 for yes): ");
        scanf("%i", &hClip);*/
        if(hClip == 1){
          hClip = 0;
        } else {
          hClip = 1;
        }
      } else if(strcmp(command, "print") == 0){
        printf("Printing Result to file and exit, Thank you/n");
        drywet(writeBuffer, glength, pBuffer, ginBuffer, gratio);
        if(hClip == 1){
        hardclip(writeBuffer, glength, writeBuffer);
        }
        sf_write_float(outFile.file, writeBuffer, glength);
        break;
      } else if(strcmp(command, "exit") == 0){
        printf("Exiting, Thank you for shifting\n");
        break;
      } else {
        printf("Invalid command, try again\n");
      }
    }
  }


    error = Pa_StopStream(pStream);
    if(error != paNoError){
      printf("Error: Failed to stop port audio stream. %s", Pa_GetErrorText(error));
    }
  

  //Close port audio streaming
  if(closePortAudio()) return 1;

  
  // //Clean up
  sf_close(sndFile.file);
  sf_close(outFile.file);
}










//------------------------------------------------------------------------------------
void passBuffer(float *buffer, unsigned long numFrames, float *insample){
  float inBuffer[numFrames];
  static unsigned long x = 0;
  unsigned long y;

  if ((x+1) * numFrames < glength){
    y = x * numFrames;
    x++;
  } else {
    y = x * numFrames;
    x = 0;
  }

  for (unsigned long a = 0; a < numFrames; a++){
    if(a+y <= glength){
    buffer[a] = insample[a+y];
    } else {
      buffer[a] = 0;
    }
  }

}
//------------------------------------------------------------------------------------
void passBuffer2(float *buffer, unsigned long numFrames, float *insample){
  float inBuffer[numFrames];
  static unsigned long x = 0;
  unsigned long y;

  if ((x+1) * numFrames < glength){
    y = x * numFrames;
    x++;
  } else {
    y = x * numFrames;
    x = 0;
  }

  for (unsigned long a = 0; a < numFrames; a++){
    if(a+y <= glength){
    buffer[a] = insample[a+y];
    } else {
      buffer[a] = 0;
    }
  }

}
//------------------------------------------------------------------------------------
void drywet(float *buffer, unsigned long numFrames, float *wetBuffer,float *dryBuffer, float ratio){
  for(unsigned long a = 0; a < numFrames; a++){
    buffer[a] = (1-ratio) * wetBuffer[a];
    buffer[a] += (ratio) * dryBuffer[a];
  }
}
//------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
void hardclip(float *buffer, unsigned long numFrames, float *insample){
  float clipBuffer[numFrames];

  for(unsigned long a = 0; a < numFrames; a++){
    clipBuffer[a] = insample[a] * kGain; //Amplify orignal audio data
    if(clipBuffer[a] > 1.0f){
      clipBuffer[a] = 1.0f; //Clip signal to 1
    } else if(clipBuffer[a] < - 1.0f){
      clipBuffer[a] = -1.0; //Clip signal to -1
    }
    clipBuffer[a] *= kLevel; //Adjust volume
    buffer[a] = clipBuffer[a];
  }
}
//-----------------------------------------------------------------------------------
int renderCallback(
  const void *input,
  void *output,
  unsigned long frameCount,
  const PaStreamCallbackTimeInfo* timeInfo,
  PaStreamCallbackFlags statusFlags,
  void *userData 
){
  float *inBuffer = (float *) userData;
  float *dryBuffer = (float *) userData;
  float *outBuffer = (float *) output;
  float tempBuffer[frameCount];
  float dBuffer[frameCount];
  float wBuffer[frameCount];

//process audio
  passBuffer(dBuffer, frameCount, dryBuffer);
  passBuffer2(wBuffer, frameCount, pBuffer);
  drywet(tempBuffer, frameCount, wBuffer, dBuffer, gratio);
  if(hClip == 1){
  hardclip(tempBuffer, frameCount, tempBuffer);
  }
//write processed audio to output
 for (unsigned long a = 0; a < frameCount; a++){
   outBuffer[a] = tempBuffer[a];
 }



  return 0;
}

//------------------------------------------------------------------------------------
int initPortAudio(){ //Initialize Port Audio
  PaError error = Pa_Initialize();
  if(error != paNoError){
    printf("Error: Pa_Initialize() failed with %s\n", Pa_GetErrorText(error));
    return 1;
  }
  return 0;
}
//------------------------------------------------------------------------------------
int closePortAudio(){ //Terminate Port Audio
  PaError error = Pa_Terminate();
  if(error != paNoError){
    printf("Error: Pa_Terminate() failed with %s\n",Pa_GetErrorText(error));
    return 1;
  }
  return 0;
}
//------------------------------------------------------------------------------------
void printPaDevices(){
  //Get number of port audio devices available
  PaDeviceIndex numDevices = Pa_GetDeviceCount(); 
  PaDeviceIndex curDeviceID;

  const PaDeviceInfo *pDeviceInfo; 
  const PaHostApiInfo *pHostApiInfo;

  //Iterate over each device and print out information about them
  for(curDeviceID = 0; curDeviceID < numDevices; curDeviceID++){
    pDeviceInfo = Pa_GetDeviceInfo(curDeviceID);
    pHostApiInfo = Pa_GetHostApiInfo(pDeviceInfo->hostApi);
    printf("--------------------------------------------\n");
    printf("ID: %d, Name: %s, ", curDeviceID, pDeviceInfo->name);
    printf("API name: %s\n", pHostApiInfo->name);
    printf("Max output channels: %d\t", pDeviceInfo->maxOutputChannels);
    printf("Max input channels: %d\n\n", pDeviceInfo->maxInputChannels);
  }
}

//-------------------
int openread(SoundFile *sndFile, char *fileName){
  //Initialize SF_INFO with 0s (Required for reading)
  memset(&sndFile->info, 0, sizeof(SF_INFO));

  //Open the original sound file as read mode
  sndFile->file = sf_open(fileName, SFM_READ, &sndFile->info);
  if(!sndFile->file){//Check if the file was succefully opened
    printf("Error : could not open file : %s\n", fileName);
		puts(sf_strerror(NULL));
		return 1;
  }

  //Check if the file format is in good shape
  if(!sf_format_check(&sndFile->info)){	
    sf_close(sndFile->file);
		printf("Invalid encoding\n");
		return 1;
	}

  //print out information about opened sound file
  printf("Sample rate for this file is %d\n", sndFile->info.samplerate);
	printf("A number of channels in this file is %d\n", sndFile->info.channels);
  printf("A number of frames in this file is %lld\n", sndFile->info.frames);
  printf("Duration is %f\n", (double)sndFile->info.frames / sndFile->info.samplerate);

  unsigned long bufferSize = sndFile->info.frames;
  //glength = 132300;
  //2646000
  glength = bufferSize;
  //printf("length is: %li \n", bufferSize);
  //float inBuffer[bufferSize];

  sf_read_float(sndFile->file, ginBuffer, bufferSize);

  return 0;
}

int createOutputFile(SoundFile *inFile, SoundFile *outFile){
  //Open another sound file in write mode
  outFile->file = sf_open(kOutputFileName, SFM_WRITE, &inFile->info);
  if(!outFile->file){//Check if the file was succefully opened
    printf("Error : could not open file : %s\n", kOutputFileName);
		puts(sf_strerror(NULL));
		return 1;
	}
  return 0;
}


void pshifter(char *fileName, int shiftval){
    srand(1234567);
    UserData ud;
    sp_data *sp;
    sp_create(&sp);

    sp_pshift_create(&ud.pshift);
    sp_diskin_create(&ud.diskin);    

    sp_pshift_init(sp, ud.pshift);
    *ud.pshift->shift = shiftval;
    *ud.pshift->window = 500;
    /* half window size is smoothest sounding */
    *ud.pshift->xfade = 50;
    sp_diskin_init(sp, ud.diskin, fileName);

    sp->len = glength;
    //sp_process(sp, &ud, process);
    long counter;
    counter = 0;
    //float gBuffer[glength];
    for(float x = 0; x < glength; x++){
    procpitch(pBuffer, &ud, sp, counter);
    counter++;
    }
}



//
void procpitch(float *out, void *udata, sp_data *sp, long n){
    UserData *ud = udata;
    SPFLOAT diskin = 0, pshift = 0;
    sp_diskin_compute(sp, ud->diskin, NULL, &diskin);
    sp_pshift_compute(sp, ud->pshift, &diskin, &pshift);
    out[n] = pshift;
}

