#pragma once

#include <xaudio2.h>
#include "Object.h"
#include "Component.h"

//#pragma comment(lib, "winmm.lib")
class Audio : public CComponent
{
private:
	static IXAudio2*				_XAudio;
	static IXAudio2MasteringVoice*	_masteringVoice;

	IXAudio2SourceVoice*	_sourceVoice{};
	BYTE*					_soundData{};

	int						_length{};
	int						_playLength{};


public:
	static void InitMaster();
	static void UninitMaster();

	Audio();
	virtual ~Audio();

	void Load(const char *FileName);
	void Play(bool Loop = false);


};

