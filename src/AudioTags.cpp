
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008-2014 Simon Judd
 *
 * Email:       sirjuddington@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    AudioTags.cpp
 * Description: Functions for parsing metadata tags in audio files.
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
#include "AudioTags.h"

#pragma pack(1)
struct id3v1_t
{
	char		tag[3];	// TAG
	char		title[30];
	char		artist[30];
	char		album[30];
	char		year[4];
	char		comment[30];
	uint8_t		genre;
};

struct id3v1e_t
{
	char		tag[4];	// TAG+
	char		title[60];
	char		artist[60];
	char		album[60];
	uint8_t		speed;
	char		subgenre[30];
	char		start[6];
	char		stop[6];
};
#pragma pack()

enum id3v2_frames
{
	ID3_COM = 0x434F4D,
	ID3_TAL = 0x54414C,
	ID3_TCM = 0x54434D,
	ID3_TCO = 0x54434F,
	ID3_TCR = 0x544352,
	ID3_TOA = 0x544F41,
	ID3_TOL = 0x544F4C,
	ID3_TOT = 0x544F54,
	ID3_TP1 = 0x545031,
	ID3_TP2 = 0x545032,
	ID3_TP3 = 0x545033,
	ID3_TP4 = 0x545034,
	ID3_TRK = 0x54524B,
	ID3_TT1 = 0x545431,
	ID3_TT2 = 0x545432,
	ID3_TT3 = 0x545433,
	ID3_TXT = 0x545854,
	ID3_TYE = 0x545945,
	ID3_COMM = 0x434F4D4D,
	ID3_TALB = 0x54414C42,
	ID3_TCOM = 0x54434F4D,
	ID3_TCON = 0x54434F4E,
	ID3_TCOP = 0x54434F50,
	ID3_TDRC = 0x54445243,
	ID3_TEXT = 0x54455854,
	ID3_TIT1 = 0x54495431,
	ID3_TIT2 = 0x54495432,
	ID3_TIT3 = 0x54495433,
	ID3_TOAL = 0x544F414C,
	ID3_TOLY = 0x544F4C59,
	ID3_TOPE = 0x544F5045,
	ID3_TPE1 = 0x54504531,
	ID3_TPE2 = 0x54504532,
	ID3_TPE3 = 0x54504533,
	ID3_TPE4 = 0x54504534,
	ID3_TPOS = 0x54504F53,
	ID3_TRCK = 0x5452434B,
	ID3_TYER = 0x54594552,
};

static const char * const id3v1_genres[] =
{
	"Blues", "Classic Rock", "Country", "Dance", "Disco", "Funk",		//   0-  5
	"Grunge", "Hip-Hop", "Jazz", "Metal", "New Age", "Oldies",			//   6- 11
	"Other", "Pop", "Rhythm and Blues",	"Rap", "Reggae", "Rock",		//  12- 17
	"Techno", "Industrial",	"Alternative", "Ska", "Death Metal",		//  18- 22
	"Pranks", "Soundtrack",	"Euro-Techno", "Ambient", "Trip-Hop",		//  23- 27
	"Vocal", "Jazz & Funk",	"Fusion", "Trance", "Classical",			//  28- 32
	"Instrumental", "Acid",	"House", "Game", "Sound Clip", "Gospel",	//  33- 38
	"Noise", "Alternative Rock", "Bass", "Soul", "Punk", "Space",		//  39- 44
	"Meditative", "Instrumental Pop", "Instrumental Rock", "Ethnic",	//  45- 48
	"Gothic", "Darkwave", 	"Techno-Industrial", "Electronic",			//  49- 52
	"Pop-Folk", "Eurodance", "Dream", "Southern Rock", "Comedy",		//  53- 57
	"Cult", "Gangsta Rap", "Top 40", "Christian Rap", "Pop & Funk",		//  58- 62
	"Jungle", "Native American", "Cabaret", "New Wave", "Psychedelic",	//  63- 67
	"Rave", "Showtunes", "Trailer", "Lo-Fi", "Tribal", "Acid Punk",		//  68- 73
	"Acid Jazz", "Polka", "Retro", "Musical", "Rock & Roll",			//  74- 78
	"Hard Rock", "Folk", "Folk-Rock", "National Folk", "Swing",			//  79- 83
	"Fast-Fusion", "Bebob", "Latin", "Revival", "Celtic", "Bluegrass",	//  84- 89
	"Avantgarde", "Gothic Rock", "Progressive Rock",					//  90- 92
	"Psychedelic Rock", "Symphonic Rock", "Slow Rock", "Big Band",		//  93- 96
	"Chorus", "Easy Listening", "Acoustic", "Humour", "Speech",			//  97-101
	"Chanson", "Opera", "Chamber Music", "Sonata", "Symphony",			// 102-106
	"Booty Bass", "Primus", "Porn Groove", "Satire", "Slow Jam",		// 107-111
	"Club", "Tango", "Samba", "Folklore", "Ballad", "Power Ballad",		// 112-117
	"Rhythmic Soul", "Freestyle", "Duet", "Punk Rock", "Drum Solo",		// 118-122
	"A Cappella", "Euro-House", "Dance Hall", "Goa", "Drum & Bass",		// 123-127
	"Club-House", "Hardcore", "Terror", "Indie", "BritPop",				// 128-132
	"Afro-Punk", "Polsk Punk", "Beat", "Christian Gangsta Rap",			// 133-136
	"Heavy Metal", "Black Metal", "Crossover",							// 137-139
	"Contemporary Christian", "Christian Rock", "Merengue", "Salsa",	// 140-143
	"Thrash Metal", "Anime", "JPop", "Synthpop", "Abstract",			// 144-148
	"Art Rock", "Baroque", "Bhangra", "Big Beat", "Breakbeat",			// 149-153
	"Chillout", "Downtempo", "Dub", "EBM", "Eclectic", "Electro",		// 154-159
	"Electroclash", "Emo", "Experimental", "Garage", "Global", "IDM",	// 160-165
	"Illbient", "Industro-Goth", "Jam Band", "Krautrock", "Leftfield",	// 166-170
	"Lounge", "Math Rock", "New Romantic", "Nu-Breakz", "Post-Punk",	// 171-175
	"Post-Rock", "Psytrance", "Shoegaze", "Space Rock", "Trop Rock",	// 176-180
	"World Music", "Neoclassical", "Audiobook", "Audio Theatre",		// 181-184
	"Neue Deutsche Welle", "Podcast", "Indie Rock", "G-Funk",			// 185-188
	"Dubstep", "Garage Rock", "Psybient"								// 188-191
};

string BuildID3v2GenreString(string content)
{
	string genre = "";
	size_t i = 0;
	while (i < content.length())
	{
		if (content[i] == '(' && content[i+1] != '(')
		{
			if (content[i+1] == 'R' && content[i+2] == 'X' && content[i+3] == ')')
			{
				genre += "Remix";
				i += 4;
			}
			else if (content [i+1] == 'C' && content[i+2] == 'R' && content[i+3] == ')')
			{
				genre += "Cover";
				i += 4;
			}
			else
			{
				int index = 0;
				int j = i+1;
				while (content[j] >= '0' && content[j] <= '9' && index < 192)
				{
					index = index * 10 + (content[j] - '0');
					++j;
				}
				if (content[j] != ')')
					index = 192;
				if (index < 192)
					genre += string::FromAscii(id3v1_genres[index]);
				i = j+1;
			}
		}
		else
		{
			genre += content.Right(content.length() - i);
			i = content.length() + 1;
		}
		if (i < content.length() && content[i] == '(')
			genre += " / ";
	}
	return genre;
}

string ParseID3v1Tag(MemChunk& mc, size_t start)
{
	id3v1_t tag;
	string version, title, artist, album, comment, genre, year;
	int track = 0;

	mc.read(&tag, 128, start);
	title = string::FromAscii(tag.title, 30);
	artist = string::FromAscii(tag.artist, 30);
	album = string::FromAscii(tag.album, 30);
	comment = string::FromAscii(tag.comment, 30);
	year = string::FromAscii(tag.year, 4);
	genre = (tag.genre < 192) ? string::FromAscii(id3v1_genres[tag.genre]) : "";
	if (tag.comment[28] == 0 && tag.comment[29] != 0)
	{
		version = "ID3v1.1";
		track = tag.comment[29];
	}
	else version = "ID3v1.0";

	// Check for extended tag
	if (start > 256 && mc[start-227] == 'T' && mc[start-226] == 'A' && mc[start-225] == 'G' && mc[start-224] == '+')
	{
		id3v1e_t etag;
		version += '+';

		mc.read(&etag, 227, start - 227);
		title += string::FromAscii(etag.title, 60);
		artist += string::FromAscii(etag.artist, 60);
		album += string::FromAscii(etag.album, 60);
		genre += S_FMT(" (%s)", etag.subgenre);
	}
	string ret = version + '\n';
	if (title.length())		ret += S_FMT("Title: %s\n", title);
	if (album.length())		ret += S_FMT("Album: %s\n", album);
	if (track != 0)			ret += S_FMT("Track: %d\n", track);
	if (artist.length())	ret += S_FMT("Artist: %s\n", artist);
	if (year.length())		ret += S_FMT("Year: %s\n", year);
	if (genre.length())		ret += S_FMT("Genre: %s\n", genre);
	ret += "\n";
	return ret;
}

string ParseID3v2Tag(MemChunk& mc, size_t start)
{
	string version, title, artist, composer, copyright, album, genre, year, group, subtitle, track, comments;
	bool artists = false;

	version = S_FMT("ID3v2.%d.%d", mc[start+3], mc[start+4]);
	bool v22 = mc[start+3] < 3;

	// ID3v2.2 frame headers have a size of 6 (3 byte identifier, 3 byte size).
	// ID3v2.3 and v2.4 frame headers have a size of 10 (4 byte identifier, 4 byte size, 2 byte flags)
	size_t step = v22 ? 6 : 10;

	// Compute synchsafe size
	size_t size = (mc[start+6] << 21) + (mc[start+7] << 14) + (mc[start+8] << 7) + mc[start+9] + 10;
	size_t end = start + size;

	// ID3v2.2 tag frames have three-char identifiers.
	// We only care about a little subset of them, though.
	size_t s = start + 10;

	// Iterate through frames. The minimal size of a frame is 1 byte of data.
	while (s + step + 1 < end)
	{
		size_t fsize = v22 ? READ_B24(mc, s+3) : READ_B32(mc, s+4);

		// One byte for encoding
		size_t tsize = fsize - 1;

		// Process only text frames that aren't empty
		// Also skip flags, not gonna bother with encryption or compression
		if ((mc[s] == 'T' || (mc[s] == 'C' && mc[s+1] == 'O' && mc[s+2] == 'M'))
			&& tsize > 0 && (v22 || mc[s+8] == 0 && mc[s+9] == 0))
		{
			string content;

			// First step: retrieve the text (UTF-16 massively sucks)
			char * buffer = new char[fsize];
			mc.read(buffer, tsize, s+step+1);
			size_t frame = v22 ? READ_B24(mc, s) : READ_B32(mc, s);

			bool bomle = true;
			size_t bom = 0;
			switch (mc[s+step])
			{
			case 0: // Plain old ASCII
				content = string::From8BitData(buffer, tsize);
				break;
			case 1:	// UTF-16 with byte order mark
				{
					size_t i = 1;
					while (i < tsize - 3)
					{
						bom = i + 1;
						// Looks like stuffing garbage before the actual content is popular
						if (mc[s+step+i] == 0xFF && mc[s+step+i+1] == 0xFE && mc[s+step+i+2] != 0)
						{
							bomle = true;
							break;
						}
						if (mc[s+step+i] == 0xFE && mc[s+step+i+1] == 0xFF && mc[s+step+i+3] != 0)
						{
							bomle = false;
							break;
						}
						++i;
					}
				}
				// Fall through
			case 2:	// UTF-16 without byte order mark
				{
					// You're right, wxWidgets. Code like this is so much 
					// better than having something like wxString::FromUTF16()
					size_t u16c = (tsize - bom) / 2;
					wchar_t * wchars = new wchar_t[u16c];
					for (size_t i = 0; i < u16c; ++i)
					{
						if (bomle)
							wchars[i] = (wchar_t)READ_L16(buffer, bom+2*i);
						else
							wchars[i] = (wchar_t)READ_B16(buffer, bom+2*i);
					}
					content = string(wchars, u16c);
					delete[] wchars;
				}
				break;
			case 3:	// UTF-8
				content = string::FromUTF8(buffer, tsize);
				break;
			}
			delete[] buffer;

			// Second step: treat frame accordingly to type
			switch (frame)
			{
			case ID3_COM:	// Comments
			case ID3_COMM:	// Comments
				if (comments.length())
					comments += "\n\n";
				comments += content;
				break;
			case ID3_TAL:	// Album/Movie/Show title
			case ID3_TOT:	// Original album/movie/show title
			case ID3_TALB:	// Album/Movie/Show title
			case ID3_TOAL:	// Original album/movie/show title
				if (album.length())
					album += " / ";
				album += content;
				break;
			case ID3_TCM:	// Composer
			case ID3_TCOM:	// Composer
				composer = content;
				break;
			case ID3_TCO:	// Content type
			case ID3_TCON:	// Content type
				genre = BuildID3v2GenreString(content);
				break;
			case ID3_TCR:	// Copyright message
			case ID3_TCOP:	// Copyright message
				copyright = content;
				break;
			case ID3_TOA:	// Original artist(s)/performer(s)
			case ID3_TOL:	// Original Lyricist(s)/text writer(s)
			case ID3_TP1:	// Lead artist(s)/Lead performer(s)/Soloist(s)/Performing group
			case ID3_TP2:	// Band/Orchestra/Accompaniment
			case ID3_TP3:	// Conductor/Performer refinement
			case ID3_TP4:	// Interpreted, remixed, or otherwise modified by
			case ID3_TXT:	// Lyricist/text writer
			case ID3_TEXT:	// Lyricist/Text writer
			case ID3_TOLY:	// Original lyricist(s)/text writer(s)
			case ID3_TOPE:	// Original artist(s)/performer(s)
			case ID3_TPE1:	// Lead performer(s)/Soloist(s)
			case ID3_TPE2:	// Band/orchestra/accompaniment
			case ID3_TPE3:	// Conductor/performer refinement
			case ID3_TPE4:	// Interpreted, remixed, or otherwise modified by
				if (artist.length())
				{
					artist += " / ";
					artists = true;
				}
				artist += content;
				break;
			case ID3_TRK:	// Track number/Position in set
			case ID3_TRCK:	// Track number/Position in set
				track = content;
				break;
			case ID3_TT1:	// Content group description
			case ID3_TIT1:	// Content group description
				group = content;
				break;
			case ID3_TT2:	// Title/Songname/Content description
			case ID3_TIT2:	// Title/songname/content description
				title = content;
				break;
			case ID3_TT3:	// Subtitle/Description refinement
			case ID3_TIT3:	// Subtitle/Description refinement
				group = content;
				break;
			case ID3_TYE:	// Year
			case ID3_TYER:	// Year
				year = content;
				break;
			case ID3_TDRC:	// Recording time (precision varies between yyyy and yyyy-MM-ddTHH:mm:ss)
				year = content.Left(4);
				break;
			}
		}
		// Parsing stops when padding starts
		else if (mc[s] == 0 && fsize == 0)
			break;
		s += (fsize + step);
	}

	string ret = version + '\n';
	if (group.length())		ret += S_FMT("Group: %s\n", group);
	if (title.length())		ret += S_FMT("Title: %s\n", title);
	if (album.length())		ret += S_FMT("Album: %s\n", album);
	if (track.length())		ret += S_FMT("Track: %s\n", track);
	if (artist.length())	ret += S_FMT("Artist%s: %s\n", artists ? "s" : "", artist);
	if (copyright.length())	ret += S_FMT("Copyright \x00A9 %s\n", copyright);
	if (year.length())		ret += S_FMT("Year: %s\n", year);
	if (genre.length())		ret += S_FMT("Genre: %s\n", genre);
	if (comments.length())	ret += S_FMT("Comments:\n%s\n", comments);
	ret += "\n";
	return ret;
}

string Audio::getID3Tag(MemChunk& mc)
{
	string ret;
	// Check for empty wasted space at the beginning, since it's apparently
	// quite popular in MP3s to start with a useless blank frame.
	size_t s = 0;
	if (mc[0] == 0)
	{
		// Completely arbitrary limit to how long to seek for data.
		size_t limit = MIN(1200, mc.getSize()/16);
		while ((s < limit) && (mc[s] == 0))
			++s;
	}

	if (mc.getSize() > s+14)
	{
		// Check for ID3 header (ID3v2). Version and revision numbers cannot be FF.
		// Only the four upper flags are valid.
		while (mc.getSize() > s+14 && mc[s+0] == 'I' && mc[s+1] == 'D' && mc[s+2] == '3' &&
		        mc[s+3] != 0xFF && mc[s+4] != 0xFF && ((mc[s+5] & 0x0F) == 0) &&
		        mc[s+6] < 0x80 && mc[s+7] < 0x80 && mc[s+8] < 0x80 && mc[s+9] < 0x80)
		{
			ret += ParseID3v2Tag(mc, s);

			// Compute size. It is stored as a "synchsafe integer", that is to say,
			// a big-endian value where the highest bit of each byte is not used.
			size_t size = (mc[s+6] << 21) + (mc[s+7] << 14) + (mc[s+8] << 7) + mc[s+9] + 10;
			// If there is a footer, then add 10 more to the size
			if (mc[s+5] & 0x10) size += 10;
			// Needs to be at least that big
			if (mc.getSize() >= size + 4)
			s += size;
		}
	}
	// It's also possible to get an ID3v1 (or v1.1) tag.
	// Though normally they're at the end of the file.
	if (mc.getSize() > s+132)
	{
		// Check for ID3 header (ID3v1).
		if (mc[s+0] == 'T' && mc[s+1] == 'A' && mc[s+2] == 'G')
		{
			ret += ParseID3v1Tag(mc, s);
		}
	}
	// Look for ID3v1 tag at end of file.
	if (mc.getSize() > 132)
	{
		s = mc.getSize() - 128;
		// Check for ID3 header (ID3v1).
		if (mc[s+0] == 'T' && mc[s+1] == 'A' && mc[s+2] == 'G')
		{
			ret += ParseID3v1Tag(mc, s);
		}
	}
	return ret;
}

string Audio::getVorbisTag(MemChunk& mc)
{
	return "";
}