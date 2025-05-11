#include "audio.h"
#include <iostream>
#define STB_VORBIS_IMPLEMENTATION
#include "../external/stb/stb_vorbis.c"
#include "cleanup.h" 

ALCdevice* device;
ALCcontext* context;
ALuint source;
ALuint buffer;

short* audioData = nullptr;
int audioDataSize = 0;
int sampleRate = 0;
int playbackIndex = 0;
int channels = 0;

void initOpenAL() {
    device = alcOpenDevice(nullptr); 
    if (!device) {
        std::cerr << "Failed to open OpenAL device\n";
        return;
    }

    context = alcCreateContext(device, nullptr);
    alcMakeContextCurrent(context);

    alGenSources(1, &source);
    alGenBuffers(1, &buffer);
}

void loadAudio(const char* filename) {
    int channels;
    audioDataSize = stb_vorbis_decode_filename(filename, &channels, &sampleRate, &audioData);
    if (!audioData) {
        std::cerr << "Audio load failed.\n";
        exit(-1);
    }
    std::cout << "Audio loaded: " << audioDataSize << " samples\n" << channels << " channels, " << sampleRate << " Hz\n";

    ::channels = channels;
    ::sampleRate = sampleRate;
}

void playAudio() {
    if (channels == 0 || audioData == nullptr) {
        std::cerr << "Invalid audio data.\n";
        return;
    }

    ALenum format;
    if (channels == 1) {
        format = AL_FORMAT_MONO16;
    } else if (channels == 2) {
        format = AL_FORMAT_STEREO16;
    } else {
        std::cerr << "Unsupported channel count: " << channels << std::endl;
        return;
    }

    alBufferData(buffer, format, audioData, audioDataSize * sizeof(short), sampleRate);
    alSourcei(source, AL_BUFFER, buffer);
    alSourcePlay(source);
}



