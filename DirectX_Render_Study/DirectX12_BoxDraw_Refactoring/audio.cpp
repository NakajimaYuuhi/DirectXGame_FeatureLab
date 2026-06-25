
#include <assert.h>
#include "audio.h"





IXAudio2*				Audio::_XAudio = NULL;
IXAudio2MasteringVoice*	Audio::_masteringVoice = NULL;


void Audio::InitMaster()
{
	// COM初期化
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	// XAudio生成
	XAudio2Create(&_XAudio, 0);

	// マスタリングボイス生成
	_XAudio->CreateMasteringVoice(&_masteringVoice);
}


void Audio::UninitMaster()
{
	_masteringVoice->DestroyVoice();
	_XAudio->Release();
	CoUninitialize();
}









void Audio::Load(const char *FileName)
{

	// サウンドデータ読込
	WAVEFORMATEX wfx = { 0 };

	{
		HMMIO hmmio = NULL;
		MMIOINFO mmioinfo = { 0 };
		MMCKINFO riffchunkinfo = { 0 };
		MMCKINFO datachunkinfo = { 0 };
		MMCKINFO mmckinfo = { 0 };
		UINT32 buflen;
		LONG readlen;


		hmmio = mmioOpen((LPSTR)FileName, &mmioinfo, MMIO_READ);
		assert(hmmio);

		riffchunkinfo.fccType = mmioFOURCC('W', 'A', 'V', 'E');
		mmioDescend(hmmio, &riffchunkinfo, NULL, MMIO_FINDRIFF);

		mmckinfo.ckid = mmioFOURCC('f', 'm', 't', ' ');
		mmioDescend(hmmio, &mmckinfo, &riffchunkinfo, MMIO_FINDCHUNK);

		if (mmckinfo.cksize >= sizeof(WAVEFORMATEX))
		{
			mmioRead(hmmio, (HPSTR)&wfx, sizeof(wfx));
		}
		else
		{
			PCMWAVEFORMAT pcmwf = { 0 };
			mmioRead(hmmio, (HPSTR)&pcmwf, sizeof(pcmwf));
			memset(&wfx, 0x00, sizeof(wfx));
			memcpy(&wfx, &pcmwf, sizeof(pcmwf));
			wfx.cbSize = 0;
		}
		mmioAscend(hmmio, &mmckinfo, 0);

		datachunkinfo.ckid = mmioFOURCC('d', 'a', 't', 'a');
		mmioDescend(hmmio, &datachunkinfo, &riffchunkinfo, MMIO_FINDCHUNK);



		buflen = datachunkinfo.cksize;
		_soundData = new unsigned char[buflen];
		readlen = mmioRead(hmmio, (HPSTR)_soundData, buflen);


		_length = readlen;
		_playLength = readlen / wfx.nBlockAlign;


		mmioClose(hmmio, 0);
	}


	// サウンドソース生成
	_XAudio->CreateSourceVoice(&_sourceVoice, &wfx);
	assert(_sourceVoice);
}


Audio::Audio()
	:CComponent("Audio")
{
}

Audio::~Audio()
{
	_sourceVoice->Stop();
	_sourceVoice->DestroyVoice();

	delete[] _soundData;
}





void Audio::Play(bool Loop)
{
	_sourceVoice->Stop();
	_sourceVoice->FlushSourceBuffers();


	// バッファ設定
	XAUDIO2_BUFFER bufinfo;

	memset(&bufinfo, 0x00, sizeof(bufinfo));
	bufinfo.AudioBytes = _length;
	bufinfo.pAudioData = _soundData;
	bufinfo.PlayBegin = 0;
	bufinfo.PlayLength = _playLength;

	// ループ設定
	if (Loop)
	{
		bufinfo.LoopBegin = 0;
		bufinfo.LoopLength = _playLength;
		bufinfo.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	_sourceVoice->SubmitSourceBuffer(&bufinfo, NULL);

/*
	float outputMatrix[4] = { 0.0f , 0.0f, 1.0f , 0.0f };
	m_SourceVoice->SetOutputMatrix(m_MasteringVoice, 2, 2, outputMatrix);
	//m_SourceVoice->SetVolume(0.1f);
*/


	// 再生
	_sourceVoice->Start();

}



