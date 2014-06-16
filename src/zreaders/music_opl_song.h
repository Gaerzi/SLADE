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


//#ifndef OPL_MUS_PLAYER_H
//#define OPL_MUS_PLAYER_H

#define ADLIB_CLOCK_MUL		24.0
#define OPL2CHANNELS		9
#define OPL3CHANNELS		OPL2CHANNELS * 2
#define MAXOPL2CHIPS		8

class OPLmusicFile
{
public:
	OPLmusicFile(FILE *file, const uint8_t *musiccache, size_t len);
	~OPLmusicFile();

	uint8_t *score;
	uint8_t *scoredata;

	int		OPLinit();
	void	OPLdeinit();

	void	OPLwriteReg(int which, uint32_t reg, uint8_t data);
	void	OPLwriteChannel(uint32_t regbase, uint32_t channel, uint8_t data1, uint8_t data2);
	void	OPLwriteValue(uint32_t regbase, uint32_t channel, uint8_t value);
	void	OPLwriteInstrument(uint32_t channel, struct genmidi_inst_t *instr);
	void	OPLwriteInstrument(uint32_t channel, struct audiot_inst_t *instr);
	void	OPLshutup(void);
	void	OPLwriteInitState();

	class OPLEmul *chips[MAXOPL2CHIPS];
	uint32_t OPLchannels;

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