/************************************************************************************

Filename    :   WavPlayer_OSX.cpp
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

#include "OSX_WavPlayer.h"

namespace OVR { namespace Platform { namespace OSX {

WavPlayer::WavPlayer(const char* fileName)
{
    FileName = fileName;
}

bool WavPlayer::isDataChunk(unsigned char* buffer, int index)
{
    unsigned char a = buffer[index];
    unsigned char b = buffer[index + 1];
    unsigned char c = buffer[index + 2];
    unsigned char d = buffer[index + 3];
    return (a == 'D' || a == 'd') && (b == 'A' || b == 'a') &&
		(c == 'T' || c == 't') && (d == 'A' || d == 'a');
}

int WavPlayer::getWord(unsigned char* buffer, int index)
{
    unsigned char a = buffer[index];
    unsigned char b = buffer[index + 1];
    unsigned char c = buffer[index + 2];
    unsigned char d = buffer[index + 3];
    int result = 0;
    result |= a;
    result |= b << 8;
    result |= c << 16;
    result |= d << 24;
    return result;
}

short WavPlayer::getHalf(unsigned char* buffer, int index)
{
    unsigned char a = buffer[index];
    unsigned char b = buffer[index + 1];
    short result = 0;
    result |= a;
    result |= b << 8;
    return result;
}

void *WavPlayer::LoadPCM(const char *filename, unsigned long *len)
{
    FILE *file;
    struct stat s;
    void *pcm;

    if(stat(filename, &s))
    {
        return NULL;
    }
    *len = s.st_size;
    pcm = (void *) malloc(s.st_size);
    if(!pcm)
    {
        return NULL;
    }
    file = fopen(filename, "rb");
    if(!file)
    {
        free(pcm);
        return NULL;
    }
    fread(pcm, s.st_size, 1, file);
    fclose(file);
    return pcm;
}

int WavPlayer::PlayBuffer(void *pcmbuffer, unsigned long len)
{
    AQCallbackStruct aqc;
    UInt32 err, bufferSize;
    int i;

    aqc.DataFormat.mSampleRate = SampleRate;
    aqc.DataFormat.mFormatID = kAudioFormatLinearPCM;
    if(BitsPerSample == 16)
    {
        aqc.DataFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger
			| kAudioFormatFlagIsPacked;
    }
    aqc.DataFormat.mBytesPerPacket = NumChannels * (BitsPerSample / 8);
    aqc.DataFormat.mFramesPerPacket = 1;
    aqc.DataFormat.mBytesPerFrame = NumChannels * (BitsPerSample / 8);
    aqc.DataFormat.mChannelsPerFrame = NumChannels;
    aqc.DataFormat.mBitsPerChannel = BitsPerSample;
    aqc.FrameCount = SampleRate / 60;
    aqc.SampleLen = (UInt32)(len);
    aqc.PlayPtr = 0;
    aqc.PCMBuffer = static_cast<unsigned char*>(pcmbuffer);

    err = AudioQueueNewOutput(&aqc.DataFormat,
                              aqBufferCallback,
                              &aqc,
                              NULL,
                              kCFRunLoopCommonModes,
                              0,
                              &aqc.Queue);
    if(err)
    {
        return err;
    }

    aqc.FrameCount = SampleRate / 60;
    bufferSize = aqc.FrameCount * aqc.DataFormat.mBytesPerPacket;

    for(i = 0; i < AUDIO_BUFFERS; i++)
    {
        err = AudioQueueAllocateBuffer(aqc.Queue, bufferSize,
                                       &aqc.Buffers[i]);
        if(err)
        {
            return err;
        }
        aqBufferCallback(&aqc, aqc.Queue, aqc.Buffers[i]);
    }

    err = AudioQueueStart(aqc.Queue, NULL);
    if(err)
    {
        return err;
    }

    while(true)
    {
    }
    sleep(1);
    return 0;
}

void WavPlayer::aqBufferCallback(void *in, AudioQueueRef inQ, AudioQueueBufferRef outQB)
{
    AQCallbackStruct *aqc;
    unsigned char *coreAudioBuffer;

    aqc = (AQCallbackStruct *) in;
    coreAudioBuffer = (unsigned char*) outQB->mAudioData;

    printf("Sync: %u / %u\n", aqc->PlayPtr, aqc->SampleLen);

    if(aqc->FrameCount > 0)
    {
        outQB->mAudioDataByteSize = aqc->DataFormat.mBytesPerFrame * aqc->FrameCount;
        for(int i = 0; i < aqc->FrameCount * aqc->DataFormat.mBytesPerFrame; i++)
        {
            if(aqc->PlayPtr > aqc->SampleLen)
            {
                aqc->PlayPtr = 0;
                i = 0;
            }
            coreAudioBuffer[i] = aqc->PCMBuffer[aqc->PlayPtr];
            aqc->PlayPtr++;
        }
        AudioQueueEnqueueBuffer(inQ, outQB, 0, NULL);
    }
}

int WavPlayer::PlayAudio()
{
    unsigned long len;
    void *pcmbuffer;
    int ret;

    pcmbuffer = LoadPCM(FileName, &len);
    if(!pcmbuffer)
    {
        fprintf(stderr, "%s: %s\n", FileName, strerror(errno));
        exit(EXIT_FAILURE);
    }

    unsigned char* bytes = (unsigned char*)pcmbuffer;
    int index = 0;

    // 'RIFF'
    getWord(bytes, index);
    index += 4;
    // int Length
    getWord(bytes, index);
    index += 4;
    // 'WAVE'
    getWord(bytes, index);
    index += 4;
    // 'fmt '
    getWord(bytes, index);
    index += 4;

    // int Format Length
    int fmtLen = getWord(bytes, index);
    index += 4;
    AudioFormat = getHalf(bytes, index);
    index += 2;
    NumChannels = getHalf(bytes, index);
    index += 2;
    SampleRate = getWord(bytes, index);
    index += 4;
    ByteRate = getWord(bytes, index);
    index += 4;
    BlockAlign = getHalf(bytes, index);
    index += 2;
    BitsPerSample = getHalf(bytes, index);
    index += 2;
    index += fmtLen - 16;
    while(!isDataChunk(bytes, index))
    {
        // Any Chunk
        getWord(bytes, index);
        index += 4;
        // Any Chunk Length
        int anyChunkLen = getWord(bytes, index);
        index += 4 + anyChunkLen;
    }
    // 'data'
    getWord(bytes, index);
    index += 4;
    // int Data Length
    unsigned long dataLen = getWord(bytes, index);
    index += 4;
    unsigned char* target = &bytes[index];

    ret = PlayBuffer((void *)target, dataLen);
    free(pcmbuffer);
    return ret;
}

}}}
