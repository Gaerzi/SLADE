
#ifndef __GMEPLAYER_H__
#define __GMEPLAYER_H__

#include "gme.h"
#include <SFML/Audio.hpp>
class wxString;
typedef wxString string;

#define GME_BUFFERSIZE 4096

class GMEPlayer : public sf::SoundStream
{
private:
	static GMEPlayer*	instance;

	Music_Emu*		emu;
	uint32_t		sample_rate;
	gme_info_t*		track_info;
	int				fill_rate;
	short*			buffer;

public:
	GMEPlayer();
	~GMEPlayer();

	// Singleton implementation
	static GMEPlayer*	getInstance()
	{
		if (!instance)
			instance = new GMEPlayer();

		return instance;
	}

	void resetPlayer()
	{
		if (instance)
		{
			delete instance;
			instance = new GMEPlayer();
		}
	}

	bool	initSound(uint32_t samplerate = 44100);
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
	bool	getTrackInfo(gme_info_t ** gme_info, int track);
	string	getInfo(int track = 0);
};

// Define for less cumbersome GMEPlayer::getInstance()
#define theGMEPlayer GMEPlayer::getInstance()

#endif//__GMEPLAYER_H__
