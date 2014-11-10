#ifndef __AUDIOTAGS_H__
#define	__AUDIOTAGS_H__

#include "Main.h"

namespace Audio
{
	string		getID3Tag(MemChunk& mc);
	string		getVorbisTag(MemChunk& mc);
}

#endif // __AUDIOTAGS_H__