#ifndef MUSIC_OPL_SONG_H
#define MUSIC_OPL_SONG_H
// This file contains both muslib.h and opl_mus_player.h
#include "Main.h"


/*******************************************************************
 * STRUCTS
 *******************************************************************/

// These structs are derived from the Wolf3D source code
#pragma pack(1)
struct inst_t
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
	uint32_t	length;
	uint16_t	priority;
	inst_t		inst;
	uint8_t		octave;
};
#pragma pack()

// These two arrays are also taken from Wolf3D source code
// This table maps channel numbers to carrier and modulator op cells
static	uint8_t			carriers[9] =  { 3, 4, 5,11,12,13,19,20,21},
						modifiers[9] = { 0, 1, 2, 8, 9,10,16,17,18},
// This table maps percussive voice numbers to op cells
						pcarriers[5] = {19,0xff,0xff,0xff,0xff},
						pmodifiers[5] = {16,17,18,20,21};


// That thing outputs to floating point, but SMFL is only compatible with 16-bit ints
inline int16_t to_int16(float f)
{
	int32_t tmp = f * 32000;
	if (tmp >  32767) tmp =  32767;
	if (tmp < -32767) tmp = -32767;
    return (int16_t)tmp;
}


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

#define MLVERSION	0x0175
#define MLVERSIONSTR	"1.75"
extern char MLversion[];
extern char MLcopyright[];

#define CHANNELS	16		// total channels 0..CHANNELS-1
#define PERCUSSION	15		// percussion channel

/* MUS file header structure */
struct MUSheader {
	char		ID[4];			// identifier "MUS" 0x1A
	int16_t	scoreLen;		// score length
	int16_t	scoreStart;		// score start
	int16_t	channels;		// primary channels
	int16_t	sec_channels;	// secondary channels (??)
	int16_t instrCnt;		// used instrument count
	int16_t	dummy;
//	int16_t	instruments[...];	// table of used instruments
};

/* OPL2 instrument */
struct OPL2instrument {
/*00*/	uint8_t trem_vibr_1;	/* OP 1: tremolo/vibrato/sustain/KSR/multi */
/*01*/	uint8_t	att_dec_1;		/* OP 1: attack rate/decay rate */
/*02*/	uint8_t	sust_rel_1;		/* OP 1: sustain level/release rate */
/*03*/	uint8_t	wave_1;			/* OP 1: waveform select */
/*04*/	uint8_t	scale_1;		/* OP 1: key scale level */
/*05*/	uint8_t	level_1;		/* OP 1: output level */
/*06*/	uint8_t	feedback;		/* feedback/AM-FM (both operators) */
/*07*/	uint8_t trem_vibr_2;	/* OP 2: tremolo/vibrato/sustain/KSR/multi */
/*08*/	uint8_t	att_dec_2;		/* OP 2: attack rate/decay rate */
/*09*/	uint8_t	sust_rel_2;		/* OP 2: sustain level/release rate */
/*0A*/	uint8_t	wave_2;			/* OP 2: waveform select */
/*0B*/	uint8_t	scale_2;		/* OP 2: key scale level */
/*0C*/	uint8_t	level_2;		/* OP 2: output level */
/*0D*/	uint8_t	unused;
/*0E*/	int16_t	basenote;		/* base note offset */
};

/* OP2 instrument file entry */
struct OP2instrEntry {
/*00*/	int16_t	flags;				// see FL_xxx below
/*02*/	int8_t	finetune;			// finetune value for 2-voice sounds
/*03*/	int8_t	note;				// note # for fixed instruments
/*04*/	struct OPL2instrument instr[2];	// instruments
};

#define FL_FIXED_PITCH	0x0001		// note has fixed pitch (see below)
#define FL_UNKNOWN		0x0002		// ??? (used in instrument #65 only)
#define FL_DOUBLE_VOICE	0x0004		// use two voices instead of one


#define OP2INSTRSIZE	sizeof(struct OP2instrEntry) // instrument size (36 int8_ts)
#define OP2INSTRCOUNT	(128 + 81-35+1)	// instrument count

/* From MLOPL_IO.CPP */
#define OPL2CHANNELS	9
#define OPL3CHANNELS	18
#define MAXOPL2CHIPS	8
#define MAXCHANNELS		(OPL2CHANNELS * MAXOPL2CHIPS)


/* Channel Flags: */
#define CH_SECONDARY	0x01
#define CH_SUSTAIN		0x02
#define CH_VIBRATO		0x04		/* set if modulation >= MOD_MIN */
#define CH_FREE			0x80

struct OPLdata {
	int32_t	 channelInstr[CHANNELS];			// instrument #
	uint8_t	 channelVolume[CHANNELS];		// volume
	uint8_t	 channelLastVolume[CHANNELS];	// last volume
	int8_t	 channelPan[CHANNELS];			// pan, 0=normal
	int8_t	 channelPitch[CHANNELS];			// pitch wheel, 64=normal
	uint8_t	 channelSustain[CHANNELS];		// sustain pedal value
	uint8_t	 channelModulation[CHANNELS];	// modulation pot value
	uint16_t channelPitchSens[CHANNELS];		// pitch sensitivity, 2=default
	uint16_t channelRPN[CHANNELS];			// RPN number for data entry
	uint8_t	 channelExpression[CHANNELS];	// expression
};

struct OPLio {
	virtual ~OPLio();

	void	OPLwriteChannel(uint32_t regbase, uint32_t channel, uint8_t data1, uint8_t data2);
	void	OPLwriteValue(uint32_t regbase, uint32_t channel, uint8_t value);
	void	OPLwriteFreq(uint32_t channel, uint32_t freq, uint32_t octave, uint32_t keyon);
	uint32_t OPLconvertVolume(uint32_t data, uint32_t volume);
	uint32_t OPLpanVolume(uint32_t volume, int pan);
	void	OPLwriteVolume(uint32_t channel, struct OPL2instrument *instr, uint32_t volume);
	void	OPLwritePan(uint32_t channel, struct OPL2instrument *instr, int pan);
	void	OPLwriteInstrument(uint32_t channel, struct OPL2instrument *instr);
	void	OPLwriteInstrument(uint32_t channel, struct inst_t *instr);
	void	OPLshutup(void);
	void	OPLwriteInitState(bool initopl3);

	virtual int		OPLinit(uint32_t numchips, bool stereo=false, bool initopl3=false);
	virtual void	OPLdeinit(void);
	virtual void	OPLwriteReg(int which, uint32_t reg, uint8_t data);
	virtual void	SetClockRate(double samples_per_tick);
	virtual void	WriteDelay(int ticks);

	class OPLEmul *chips[MAXOPL2CHIPS];
	uint32_t OPLchannels;
	uint32_t NumChips;
	//bool IsOPL3;
};

struct DiskWriterIO : public OPLio
{
	DiskWriterIO(const char *filename);
	~DiskWriterIO();

	int OPLinit(uint32_t numchips, bool notused=false);
	void OPLdeinit();
	void OPLwriteReg(int which, uint32_t reg, uint8_t data);
	void SetClockRate(double samples_per_tick);
	void WriteDelay(int ticks);

	void SetChip(int chipnum);

	FILE *File;
	string Filename;
	int Format;
	bool NeedClockRate;
	double TimePerTick;		// In milliseconds
	double CurTime;
	int CurIntTime;
	int TickMul;
	int CurChip;

	enum { FMT_RDOS, FMT_DOSBOX };
};

enum MUSctrl {
    ctrlPatch = 0,
    ctrlBank,
    ctrlModulation,
    ctrlVolume,
    ctrlPan,
    ctrlExpression,
    ctrlReverb,
    ctrlChorus,
    ctrlSustainPedal,
    ctrlSoftPedal,
	ctrlRPNHi,
	ctrlRPNLo,
	ctrlNRPNHi,
	ctrlNRPNLo,
	ctrlDataEntryHi,
	ctrlDataEntryLo,

	ctrlSoundsOff,
    ctrlNotesOff,
    ctrlMono,
    ctrlPoly,
};

#define ADLIB_CLOCK_MUL			24.0

//#endif // __MUSLIB_H_

//#ifndef OPL_MUS_PLAYER_H
//#define OPL_MUS_PLAYER_H



class OPLmusicFile
{
public:
	OPLmusicFile(FILE *file, const uint8_t *musiccache, size_t len);
	OPLmusicFile(const OPLmusicFile *source, const char *filename);
	~OPLmusicFile();

	uint8_t *score;
	uint8_t *scoredata;
	int playingcount;
	OPLdata driverdata;
	OPLio *io;

	struct OP2instrEntry *OPLinstruments;

	uint32_t MLtime;

	void OPLplayNote(uint32_t channel, uint8_t note, int volume);
	void OPLreleaseNote(uint32_t channel, uint8_t note);
	void OPLpitchWheel(uint32_t channel, int pitch);
	void OPLchangeControl(uint32_t channel, uint8_t controller, int value);
	void OPLprogramChange(uint32_t channel, int value);
	void OPLresetControllers(uint32_t channel, int vol);
	void OPLplayMusic(int vol);
	void OPLstopMusic();

	int OPLloadBank (MemChunk &data);


	bool ServiceStream(void *buff, int numbytes);
	bool ServiceStreamI(int16_t *buff, int numbytes);
	void ResetChips();

	bool IsValid() const;
	void SetLooping(bool loop);
	void Restart();
	void Dump();

	int  GetLength() { return ScoreLen; }
	int  GetPosition() { return score - scoredata; }
	bool SetPosition(int);
	void SetImfRate(int rate) { if (rate) ImfRate = rate; }

protected:
	void OffsetSamples(float *buff, int count);
	void OffsetSamplesI(int16_t *buff, int count);

	double NextTickIn;
	double SamplesPerTick;
	int ImfRate;
	int NumChips;
	bool Looping;
	double LastOffset;
	bool FullPan;
	bool Note;		// 0 for note off, 1 for note on; used by AudioT format
	uint8_t Octave;	// Octave, used by AudioT format;

	/* OPL channel (voice) data */
	struct channelEntry {
		uint8_t	channel;		/* MUS channel number */
		uint8_t	note;			/* note number */
		uint8_t	flags;			/* see CH_xxx below */
		uint8_t	realnote;		/* adjusted note number */
		int8_t	finetune;		/* frequency fine-tune */
		int32_t	pitch;			/* pitch-wheel value */
		int32_t	volume;			/* note volume */
		int32_t	realvolume;		/* adjusted note volume */
		struct  OPL2instrument *instr;	/* current instrument */
		uint32_t time;			/* note start time */
	} channels[MAXCHANNELS];

	void writeFrequency(uint32_t slot, uint32_t note, int pitch, uint32_t keyOn);
	void writeModulation(uint32_t slot, struct OPL2instrument *instr, int state);
	uint32_t calcVolume(uint32_t channelVolume, uint32_t channelExpression, uint32_t noteVolume);
	int occupyChannel(uint32_t slot, uint32_t channel,
						 int note, int volume, struct OP2instrEntry *instrument, uint8_t secondary);
	int releaseChannel(uint32_t slot, uint32_t killed);
	int releaseSustain(uint32_t channel);
	int findFreeChannel(uint32_t flag, uint32_t channel, uint8_t note);
	struct OP2instrEntry *getInstrument(uint32_t channel, uint8_t note);


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
};

//#endif // OPL_MUS_PLAYER_H
#endif // MUSIC_OPL_SONG_H