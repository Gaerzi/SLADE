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
#include "Tokenizer.h"
#include "ArchiveManager.h"

/*******************************************************************
 * VARIABLES
 *******************************************************************/
CVAR(Int, imf_rate, 700, CVAR_SAVE)
OPLPlayer*	OPLPlayer::instance = NULL;


// The OPL emulator outputs to floating point, but SMFL is only 
// compatible with 16-bit ints, so we need to convert them.
inline int16_t to_int16(float f)
{
	int32_t tmp = f * 32000;
	if (tmp >  32767) tmp =  32767;
	if (tmp < -32767) tmp = -32767;
    return (int16_t)tmp;
}


OPLPlayer::OPLPlayer() : sf::SoundStream()
{
	emu = NULL;
	buffer = new float[OPL_BUFFERSIZE];
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
	data.importMem(mc.getData(), mc.getSize());

	emu = new OPLmusicFile(NULL, mc.getData(), mc.getSize());
	emu->SetImfRate(findImfRate(mc.crc()));
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
	float* fbuffer = (float*)buffer;
	int16_t* ibuffer = (int16_t*)buffer;

	bool res = emu ? emu->ServiceStream(fbuffer, OPL_BUFFERSIZE*sizeof(float)) : false;

	for (size_t i = 0; i < OPL_BUFFERSIZE; ++i)
	{
		ibuffer[i] = to_int16(fbuffer[i]);
	}

	data.sampleCount = OPL_BUFFERSIZE;
	data.samples     = ibuffer;

	return res;
}

void OPLPlayer::onSeek(sf::Time timeOffset)
{
	setPosition(timeOffset.asMilliseconds());
}

int OPLPlayer::findImfRate(size_t crc32)
{
	ArchiveEntry * imf_list = theArchiveManager->programResourceArchive()->entryAtPath("imfrates.txt");
	if (imf_list)
	{
		Tokenizer tz;
		string token;
		int rate;
		unsigned long val;
		tz.openMem(&(imf_list->getMCData()), "imfrates.txt");
		do
		{
			token = tz.getToken();
			rate = tz.getInteger();
			if (token.ToULong(&val, 16) && val == crc32)
				return rate;
		} while (token.length());
	}
	return imf_rate;
}

/* OPLPlayer::getInfo
 * Returns a string containing info about the current track. Since
 * only IMF files may contain metadata, an empty string is returned
 * for other formats. There are three kinds of metadata that may be
 * found in an IMF file:
 * 1. MUSE metadata (track name and source path)
 * 2. Martin Fernandez's format header (track and game name)
 * 3. Adam Nielsen's footer (title, author, comment, and program)
 * In addition, we retrieve the update rate.
 */
string OPLPlayer::getInfo()
{
	string info = wxEmptyString;
	if (emu && emu->getRawPlayer() == 1)
	{
		string track1, track2, track3;
		string source, game;
		string author, comment, program;
		size_t offset;
		size_t pos = 6;
		size_t size = data.getSize();
		if (size > pos && data[pos] != 0)
		{
			while (pos < size && data[pos] != 0)
				track1 += data[pos++];
		} else ++pos;
		if (size > pos && data[pos] != 0)
		{
			while (pos < size && data[pos] != 0)
				game += data[pos++];
		} else ++pos;
		++pos; // unknown byte
		offset = READ_L32(data, pos);
		if (offset != 0)
		{
			offset += pos + 4u;
			while (offset +13 < size)
			{
				// Nielsen footer
				if (data[offset] == 0x1A)
				{
					++offset;
					string * track;
					if (track1.IsEmpty()) track = &track1;
					else if (track2.IsEmpty()) track = &track2;
					else track = &track3;
					while (offset < size && data[offset] != 0)
						track->Append(data[offset++]);
					++offset;
					while (offset < size && data[offset] != 0)
						author.Append(data[offset++]);
					++offset;
					while (offset < size && data[offset] != 0)
						comment.Append(data[offset++]);
					++offset;
					while (offset < size && data[offset] != 0)
						program.Append(data[offset++]);
					++offset;
				}
				// MUSE footer
				else if (offset + 88 <= size)
				{
					offset += 2;
					string * track;
					if (track1.IsEmpty()) track = &track1;
					else if (track2.IsEmpty()) track = &track2;
					else track = &track3;
					track->Append((char*)(&data[offset]), 16);
					source.Append((char*)(&data[offset+16]), 64);
					offset += 86;
				}
			}
		}
		if (track1.length())
		{
			info = S_FMT("Track name: %s", track1);
			if (track2.length())
				info += S_FMT(" / %s", track2);
			if (track3.length())
				info += S_FMT(" / %s", track3);
			info += "\n";
		}
		else
			info = "Unidentified song\n";

		if (game.length())
			info += S_FMT("Game: %s\n", game);
		if (author.length())
			info += S_FMT("Author(s): %s\n", author);
		if (source.length())
			info += S_FMT("Source: %s\n", source);
		if (program.length())
			info += S_FMT("Tagged with: %s\n", program);
		if (comment.length())
			info += S_FMT("\"%s\"\n", comment);
		
		info += S_FMT("IMF update rate: %u\n", emu->GetImfRate());
	}
	return info;
}
