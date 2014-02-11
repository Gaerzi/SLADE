/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008-2014 Simon Judd
 *
 * Email:       sirjuddington@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    OPLPlayer.cpp
 * Description: OPLPlayer class, a singleton class that handles
 *              playback of files through the OPL emulator.
 *              Mostly derived from ZDoom code.
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
#include "OPLPlayer.h"

/*******************************************************************
 * VARIABLES
 *******************************************************************/
OPLPlayer*	OPLPlayer::instance = NULL;

OPLPlayer::OPLPlayer() : sf::SoundStream()
{
	emu = NULL;
	buffer = new short[OPL_BUFFERSIZE];
}

OPLPlayer::~OPLPlayer()
{
	stop();
	delete[] buffer;
}

bool OPLPlayer::initSound(uint32_t samplerate)
{
	sf::SoundStream::initialize(2, samplerate);

	return true;
}

bool OPLPlayer::openData(MemChunk &mc, int subsong, int * num_tracks)
{
	stop();
	*num_tracks = 1;

	emu = new OPLmusicFile(NULL, mc.getData(), mc.getSize());
	emu->Restart();

	initSound();
	return true;
}

bool OPLPlayer::play(int track)
{
	if (emu)
	{
		if (sf::SoundStream::getStatus() != sf::SoundStream::Paused)
		{
			// Sound must not be running when operating on emulator
			sf::SoundStream::stop();

			// Start track
			emu->Restart();
			
			// Calculate track length
			// ???
		}
		
		sf::SoundStream::play();
	}
	return true;
}

bool OPLPlayer::pause()
{
	sf::SoundStream::pause();
	return true;
}

bool OPLPlayer::stop()
{
	sf::SoundStream::stop();
	delete emu;
	emu = NULL;
	return true;
}

bool OPLPlayer::isPlaying()
{
	return (sf::SoundSource::getStatus() == sf::SoundSource::Playing);
}

int OPLPlayer::getPosition()
{
	return emu ? emu->GetPosition() : 0;
}

bool OPLPlayer::setPosition(int pos)
{
	if (emu)
	{
		return emu->SetPosition(pos);
	}
	return false;
}

int OPLPlayer::getLength()
{
	return emu ? emu->GetLength() : 0;
}

bool OPLPlayer::setVolume(int volume)
{
	sf::SoundSource::setVolume(volume);
	return true;
}

bool OPLPlayer::onGetData(sf::SoundStream::Chunk &data)
{
	emu->ServiceStreamI(buffer, OPL_BUFFERSIZE*2);

	data.sampleCount = OPL_BUFFERSIZE;
	data.samples     = buffer;

	return true;
}

void OPLPlayer::onSeek(sf::Time timeOffset)
{
	setPosition(timeOffset.asMilliseconds());
}