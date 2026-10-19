#pragma once

#include <xaudio2.h>
#include "EngineAPI.h"
#include "Object.h"
#include "Component.h"

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
	ENGINE_API static void InitMaster();
	ENGINE_API static void UninitMaster();

	ENGINE_API Audio();
	ENGINE_API virtual ~Audio();

	ENGINE_API void Load(const char *FileName);
	ENGINE_API void Play(bool Loop = false);
};
