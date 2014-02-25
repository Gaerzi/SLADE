/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008-2014 Simon Judd
 *
 * Email:       sirjuddington@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    GMEPlayer.cpp
 * Description: GMEPlayer class, a singleton class that handles
 *              playback of files through the Game Music Emu
 *              library. Can only play one file at a time
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *******************************************************************/


/*******************************************************************
 * INCLUDES
 *******************************************************************/
#include "Main.h"
#include "GMEPlayer.h"
#include "Compression.h"

#define RETURN_ERR(expr) \
	do {\
		gme_err_t err = (expr);\
		if (err)\
		{\
			wxLogMessage("GME error: %s", err);\
			return false;\
		}\
	} while ( 0 )

#define GZIP_SIGNATURE 0x1F8B0800
CVAR(Bool, gme_debug, false, 0)
bool do_debug_gme() { return gme_debug; }
/*******************************************************************
 * VARIABLES
 *******************************************************************/
GMEPlayer*	GMEPlayer::instance = NULL;

GMEPlayer::GMEPlayer() : sf::SoundStream()
{
	emu = NULL;
	track_info = NULL;
	fill_rate = 45;
	buffer = new short[GME_BUFFERSIZE];
	sample_rate = 44100;
}

GMEPlayer::~GMEPlayer()
{
	stop();
	gme_free_info(track_info);
	delete[] buffer;
}

bool GMEPlayer::initSound(uint32_t samplerate)
{
	sample_rate = samplerate;
	sf::SoundStream::initialize(2, sample_rate);

	return true;
}

bool GMEPlayer::openData(MemChunk &mc, int subsong, int * num_tracks)
{
	stop();
	// Detect GZipped content and uncompress if needed
	MemChunk* data = &mc;
	MemChunk decomp;
	if (mc.getSize() > 4 && READ_B32(mc, 0) == GZIP_SIGNATURE)
	{
		data = &decomp;
		Compression::GZipInflate(mc, decomp);
	}
	RETURN_ERR(gme_open_data(data->getData(), data->getSize(), &emu, sample_rate));
	if (num_tracks && emu && gme_type_multitrack(gme_type(emu)))
		*num_tracks = gme_track_count(emu);

	initSound();
	return true;
}

bool GMEPlayer::play(int track)
{
	if (emu)
	{
		if (sf::SoundStream::getStatus() != sf::SoundStream::Paused)
		{
			gme_free_info(track_info);
			track_info = NULL;
			RETURN_ERR(gme_track_info(emu, &track_info, track));

			// Sound must not be running when operating on emulator
			sf::SoundStream::stop();
			RETURN_ERR(gme_start_track(emu, track));
			
			// Calculate track length
			if (track_info->length <= 0)
				track_info->length = track_info->intro_length + track_info->loop_length * 2;
			
			if ( track_info->length <= 0 )
				track_info->length = (long) (150000);
			gme_set_fade(emu, track_info->length);
		}
		
		sf::SoundStream::play();
	}
	return true;
}

bool GMEPlayer::getTrackInfo(gme_info_t ** gme_info, int track)
{
	RETURN_ERR(gme_track_info(emu, gme_info, track));
	return true;
}

string GMEPlayer::getInfo(int track)
{
	string info = wxEmptyString;
	gme_info_t * gme_info = NULL;
	if (emu && getTrackInfo(&gme_info, track))
	{
		if (gme_info->song[0])
			info = S_FMT("Track name: %s\n", gme_info->song);
		else
			info = "Unidentified song\n";
		if (gme_info->game[0])
			info += S_FMT("Game: '%s'\n", gme_info->game);
		if (gme_info->system[0])
			info += S_FMT("System: %s\n", gme_info->system);
		if (gme_info->author[0])
			info += S_FMT("Author(s): %s\n", gme_info->author);
		if (gme_info->copyright[0])
			info += S_FMT("(c) %s\n", gme_info->copyright);
		if (gme_info->dumper[0])
			info += S_FMT("Dumped by: %s\n", gme_info->dumper);
		if (gme_info->comment[0])
			info += S_FMT("\"%s\"\n", gme_info->comment);
		gme_free_info(gme_info);
	}
	return info;
}

bool GMEPlayer::pause()
{
	sf::SoundStream::pause();
	return true;
}

bool GMEPlayer::stop()
{
	sf::SoundStream::stop();
	gme_delete(emu);
	emu = NULL;
	return true;
}

bool GMEPlayer::isPlaying()
{
	return (sf::SoundSource::getStatus() == sf::SoundSource::Playing);
}

int GMEPlayer::getPosition()
{
	return emu ? gme_tell(emu) : 0;
}

bool GMEPlayer::setPosition(int pos)
{
	if (emu)
	{
		RETURN_ERR(gme_seek(emu, pos));
		return true;
	}
	return false;
}

int GMEPlayer::getLength()
{
	return track_info ? track_info->length : 0;
}

bool GMEPlayer::setVolume(int volume)
{
	sf::SoundSource::setVolume(volume);
	return true;
}

bool GMEPlayer::onGetData(sf::SoundStream::Chunk &data)
{
	RETURN_ERR(gme_play(emu, GME_BUFFERSIZE, buffer));

	data.sampleCount = GME_BUFFERSIZE;
	data.samples     = buffer;

	return true;
}

void GMEPlayer::onSeek(sf::Time timeOffset)
{
	setPosition(timeOffset.asMilliseconds());
}