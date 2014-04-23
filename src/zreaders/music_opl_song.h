#ifndef MUSIC_OPL_SONG_H
#define MUSIC_OPL_SONG_H
// This file contains both muslib.h and opl_mus_player.h
#include "Main.h"


/*******************************************************************
 * STRUCTS
 *******************************************************************/

// These structs are derived from the Wolf3D source code
#pragma pack(1)
struct audiot_inst_t
{
	uint8_t	mChar,cChar,
			mScale,cScale,
			mAttack,cAttack,
			mSus,cSus,
			mWave,cWave,
			nConn,
			unused[5];
};

struct oplsound_t
{
	uint32_t		length;
	uint16_t		priority;
	audiot_inst_t	inst;
	uint8_t			octave;
};

// This struct is derived from specs at http://doomwiki.org/wiki/GENMIDI
/*
0 	1 	Modulator Characteristics (tremolo / vibrato / sustain / KSR / multi)
1 	1 	Modulator Attack rate / decay rate
2 	1 	Modulator Sustain level / release rate
3 	1 	Modulator Waveform select
4 	1 	Modulator Key scale level
5 	1 	Modulator Output level
6 	1 	Feedback
7 	1 	Carrier Characteristics
8 	1 	Carrier Attack rate / decay rate
9 	1 	Carrier Sustain level / release rate
10 	1 	Carrier Waveform select
11 	1 	Carrier Key scale level
12 	1 	Carrier Output level
13 	1 	Unused
14 	2 	Base note offset. This is used to offset the MIDI note values. Several of the
		GENMIDI instruments have a base note offset of -12, causing all notes to be 
		offset down by one octave. 
*/
struct genmidi_inst_t
{
	uint8_t	mChar,
			mAttack,
			mSus,
			mScale,
			mWave,
			mOutput,
			nConn,
			cChar,
			cAttack,
			cSus,
			cScale,
			cWave,
			cOutput,
			nUnused;
	int16_t nOffset;
};
#pragma pack()

// These two arrays are also taken from Wolf3D source code
// They are here for reference only, not used in code.
// This table maps channel numbers to carrier and modulator op cells
//static	uint8_t			carriers[9] =  { 3, 4, 5,11,12,13,19,20,21},
//						modifiers[9] = { 0, 1, 2, 8, 9,10,16,17,18},
// This table maps percussive voice numbers to op cells
//						pcarriers[5] = {19,0xff,0xff,0xff,0xff},
//						pmodifiers[5] = {16,17,18,20,21};

/*
 *	Name:		Main header include file
 *	Project:	MUS File Player Library
 *	Version:	1.75
 *	Author:		Vladimir Arnost (QA-Software)
 *	Last revision:	Mar-9-1996
 *	Compiler:	Borland C++ 3.1, Watcom C/C++ 10.0
 *
 */

/* From muslib175.zip/README.1ST:

1.1 - Disclaimer of Warranties
------------------------------

#ifdef LAWYER

THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.

#else

Use this software at your own risk.

#endif


1.2 - Terms of Use
------------------

This library may be used in any freeware or shareware product free of
charge. The product may not be sold for profit (except for shareware) and
should be freely available to the public. It would be nice of you if you
credited me in your product and notified me if you use this library.

If you want to use this library in a commercial product, contact me
and we will make an agreement. It is a violation of the law to make money
of this product without prior signing an agreement and paying a license fee.
This licence will allow its holder to sell any products based on MUSLib,
royalty-free. There is no need to buy separate licences for different
products once the licence fee is paid.


1.3 - Contacting the Author
---------------------------

Internet (address valid probably until the end of year 1998):
  xarnos00@dcse.fee.vutbr.cz

FIDO:
  2:423/36.2

Snail-mail:

  Vladimir Arnost
  Ceska 921
  Chrudim 4
  537 01
  CZECH REPUBLIC

Voice-mail (Czech language only, not recommended; weekends only):

  +42-455-2154
*/

//#ifndef __MUSLIB_H_
//#define __MUSLIB_H_

/* Global Definitions */

/* From MLOPL_IO.CPP */
#define OPL2CHANNELS	9
#define OPL3CHANNELS	18
#define MAXOPL2CHIPS	8

struct OPLio {
	virtual ~OPLio();

	void	OPLwriteChannel(uint32_t regbase, uint32_t channel, uint8_t data1, uint8_t data2);
	void	OPLwriteValue(uint32_t regbase, uint32_t channel, uint8_t value);
	void	OPLwriteInstrument(uint32_t channel, struct genmidi_inst_t *instr);
	void	OPLwriteInstrument(uint32_t channel, struct audiot_inst_t *instr);
	void	OPLshutup(void);
	void	OPLwriteInitState();

	virtual int		OPLinit();
	virtual void	OPLdeinit();
	virtual void	OPLwriteReg(int which, uint32_t reg, uint8_t data);

	class OPLEmul *chips[MAXOPL2CHIPS];
	uint32_t OPLchannels;
};

//#endif // __MUSLIB_H_

//#ifndef OPL_MUS_PLAYER_H
//#define OPL_MUS_PLAYER_H

#define ADLIB_CLOCK_MUL			24.0

class OPLmusicFile
{
public:
	OPLmusicFile(FILE *file, const uint8_t *musiccache, size_t len);
	~OPLmusicFile();

	uint8_t *score;
	uint8_t *scoredata;
	OPLio *io;

	uint32_t MLtime;

	void OPLstopMusic();

	bool ServiceStream(void *buff, int numbytes);

	void Restart();

	int  GetLength() { return ScoreLen; }
	int  GetPosition() { return score - scoredata; }
	bool SetPosition(int);
	void SetImfRate(int rate) { if (rate) ImfRate = rate; }
	int  GetImfRate() { return ImfRate; }

protected:
	void OffsetSamples(float *buff, int count);

	double NextTickIn;
	double SamplesPerTick;
	int ImfRate;
	int NumChips;
	double LastOffset;
	uint8_t Octave;	// Octave, used by AudioT format;

	int releaseChannel(uint32_t slot, uint32_t killed);

	OPLmusicFile() {}
	int PlayTick();

	enum RawPlayers
	{
		RDosPlay,
		IMF,
		DosBox1,
		DosBox2,
		AudioT,
	} RawPlayer;
	int ScoreLen;
	int WhichChip;

public:
	RawPlayers getRawPlayer() { return RawPlayer; }
};

//#endif // OPL_MUS_PLAYER_H
#endif // MUSIC_OPL_SONG_H