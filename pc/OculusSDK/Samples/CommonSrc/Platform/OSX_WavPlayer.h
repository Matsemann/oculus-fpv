/************************************************************************************

Filename    :   WavPlayer_OSX.h
Content     :   An Apple OSX audio handler.
Created     :   March 5, 2013
Authors     :   Robotic Arm Software - Peter Hoff, Dan Goodman, Bryan Croteau

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

************************************************************************************/

#ifndef OVR_WavPlayer_h
#define OVR_WavPlayer_h

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <AudioToolbox/AudioQueue.h>

#define AUDIO_BUFFERS 4

namespace OVR { namespace Platform { namespace OSX {

typedef struct AQCallbackStruct
{
    AudioQueueRef				Queue;
    UInt32						FrameCount;
    AudioQueueBufferRef			Buffers[AUDIO_BUFFERS];
    AudioStreamBasicDescription DataFormat;
    UInt32						PlayPtr;
    UInt32						SampleLen;
    unsigned char*				PCMBuffer;
} AQCallbackStruct;

class WavPlayer
{
public:
    WavPlayer(const char* fileName);
    int PlayAudio();
private:
    bool isDataChunk(unsigned char* buffer, int index);
    int getWord(unsigned char* buffer, int index);
    short getHalf(unsigned char* buffer, int index);
    void *LoadPCM(const char *filename, unsigned long *len);
    int PlayBuffer(void *pcm, unsigned long len);
    static void aqBufferCallback(void *in, AudioQueueRef inQ, AudioQueueBufferRef outQB);

    short		AudioFormat;
    short		NumChannels;
    int			SampleRate;
    int			ByteRate;
    short		BlockAlign;
    short		BitsPerSample;
    const char* FileName;
};

}}}

#endif
