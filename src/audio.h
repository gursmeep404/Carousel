#ifndef AUDIO_H
#define AUDIO_H

#include <AL/al.h>
#include <AL/alc.h>

extern ALCdevice* device;
extern ALCcontext* context;
extern ALuint source;
extern ALuint buffer;
extern short* audioData;
extern int audioDataSize;
extern int sampleRate;
extern int playbackIndex;
extern int channels;

void initOpenAL();
void loadAudio(const char* filename);
void playAudio();
void cleanup();

#endif // AUDIO_H

