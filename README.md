# iShift User's manual



***

- This software relies on three external libraries that need to be installed in addition to the standard libraries: **Libsndfile**, **Portaudio**, and **Soundpipe**. Portaudio and Libsndfile can be installed via Homebrew, while Soundpipe must be compiled and installed by cloning the GitHub repository and following the instructions on the repository page: https://github.com/SeesePlusPlus/soundpipe.

- The code runs with an easy question/answer UI on the command line, users only need to type in easy parameters to run the code. The basic operation of this software starting from running the code is, input file to pitch shift then input all parameters requested from the screen, adjusting parameters live on the audio stream and then chose to print the output or exit the software.

# **Operation**
* To run the iShift code, put the iShift and the audio file that need to be shifted in the same folder.
Open terminal with the directory pointing to the folder of the code and type the following.
```clang iShift.c -lsndfile -lsoundpipe -lportaudio```
./a.out

* After the code starts running, we have to input some of the initial parameters before the stream starts running and we can hear sound

* Input audio file name, (NOTE:Audio file have to also be in the same folder as the iShift.c code), the name also have to include extension eg. a.wav, audio.aiff
Pitch: change pitch up/down in semitone (-24 to 24)
ratio : change dry/wet ratio 0.0-1.0, 0 is wet and 1 is dry
Hard clip: define if users want hard clipping on the output, 1 for on and 0 for off

* After the stream of audio starts, iShift has multiple options to adjust the parameters, exit the code, or print the results. The menu are as followings:
ratio : change the dry wet ratio of the dry and pithched signal
shift : change the value of pitch shifting from -24 to 24
clip : toggle hard clip on/off
print : print the result pitch shifting audio into an audio file name PitchShift.wav and exit the software
exit : exit the software without printing anything.

1. NOTE: the menu will also show up on the terminal for users to see while the stream audio is running.

1. NOTE: in the case of an error about audio output not correct or audio not playing from the speaker you want to, please refer to "Audio MIDI setup" of your system and change the value of KDeviceIndex inside the code. Each audio input/output device will have its labeled ID number, the top will be starting from ID:0 and going up as the device list goes down. For example, if your headphone device is on number 4 of the setup list, the ID value need to be 3 inside the code for KDeviceIndex.


## Developers
[tingting129](https://github.com/tingting129)

[pkhao75](https://github.com/pkhao75)
