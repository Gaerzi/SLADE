
#ifndef __OPLPLAYER_H__
#define __OPLPLAYER_H__

#include "zreaders/music_opl_song.h"
#include <SFML/Audio.hpp>

#define OPL_BUFFERSIZE 4096
#define OPL_SAMPLE_RATE 49716

class OPLPlayer : public sf::SoundStream
{
private:
	static OPLPlayer*	instance;

	OPLmusicFile*	emu;
	short*			buffer;

public:
	OPLPlayer();
	~OPLPlayer();

	// Singleton implementation
	static OPLPlayer*	getInstance()
	{
		if (!instance)
			instance = new OPLPlayer();

		return instance;
	}

	void resetPlayer()
	{
		if (instance)
		{
			delete instance;
			instance = new OPLPlayer();
		}
	}

	bool	initSound(uint32_t samplerate = OPL_SAMPLE_RATE);
	bool	openData(MemChunk &mc, int subsong = 0, int * num_tracks = NULL);
	bool	play(int track = 0);
	bool	pause();
	bool	stop();
	bool	isPlaying();
	int		getPosition();
	bool	setPosition(int pos);
	int		getLength();
	bool	setVolume(int volume);
	void	onSeek(sf::Time timeOffset);
	bool	onGetData(sf::SoundStream::Chunk& data);
};

// Define for less cumbersome GMEPlayer::getInstance()
#define theOPLPlayer OPLPlayer::getInstance()

#endif//__OPLPLAYER_H__
