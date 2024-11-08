# iShift Design Documents



***
The main idea of this project is to implement C programming language with audio. In this project, I am focusing on doing pitch shifting. The design of the product is that we not only have a pitch shifting option but also combine different parameters together. iShift has multiple options to adjust the parameters, exit the code, or print the results. I also make sure that it is a user friendly product. Users can follow the instructions inside the terminals. 


**Coding library included:**

**Portaudio** - streaming audio in realtime, so when you run the code, whatever you change it will affect the audio in realtime and Portaudio will be the one that will play that processed audio back for you through the Callback functions.

**Libsnd**- taking care of opening the audio file and writing it down into an array for processing before opening the audio stream.

**Soundpipe** - implementing a part of the example from ex_pshift code, this is the core element for the pitch shifting to work.

Inside the code consists of multiple functions, here's a quick explanation of how each function takes part inside this software.




	int createOutputFile(SoundFile *inFile, SoundFile *outFile);
Creates PitchShift.wav file ready to be written and output as a .wav file after the code is done

	int openread(SoundFile *sndFile, char *fileName);
Open the input audio file and read it into an existing array for further processing

	int initPortAudio();
Initialize PortAudio

	int closePortAudio();
Close PortAudio after everything is finish

	void printPaDevices();
Print out Audio Devices details

	void passBuffer(float *buffer, unsigned long numFrames, float *insample);
To be able to pass an array into the Callback function and have it playback properly, a function that can pass the array into the Buffer to fit its size is needed. passBuffer basically take the sample of the input array and pass it into the callback function equal to the amount of buffer size predefined, it then have an internal counter that will let the function know where it left off from the last callback call and feed the next batch of audio into the stream.

	void passBuffer2(float *buffer, unsigned long numFrames, float *insample);
Same as passBuffer but for the pitched signal

	void drywet(float *buffer, unsigned long numFrames, float *insample, float *dryBuffer, float ratio);
Adjust the dry wet ratio of the signal

	void hardclip(float *buffer, unsigned long numFrames, float *insample);
Hard clip the audio

	void procpitch(float *out, void *udata, sp_data *sp, long n);
Code for shifting audio according to input parameters

	void pshifter(char *fileName, int shiftval);
Initialize soundpipe and carry out the pitch shifting by calling the procpitch function.
