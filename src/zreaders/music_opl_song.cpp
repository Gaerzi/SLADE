/*******************************************************************
 * INCLUDES
 *******************************************************************/
#include "music_opl_song.h"
#include "music_opl3.h"
#include "MathStuff.h"
#include "m_swap.h"
#include "tarray.h"

/*******************************************************************
 * VARIABLES
 *******************************************************************/
#define HALF_PI (PI*0.5)
CVAR(Bool, opl_singlevoice, 0, 0)
CVAR(Int, opl_numchips, 2, CVAR_SAVE)
CVAR(Int, imf_rate, 700, CVAR_SAVE)
//EXTERN_CVAR(Int, opl_core)


/*
*	Name:		Low-level OPL2/OPL3 I/O interface
*	Project:	MUS File Player Library
*	Version:	1.64
*	Author:		Vladimir Arnost (QA-Software)
*	Last revision:	Mar-1-1996
*	Compiler:	Borland C++ 3.1, Watcom C/C++ 10.0
*
*/

/*
* Revision History:
*
*	Aug-8-1994	V1.00	V.Arnost
*		Written from scratch
*	Aug-9-1994	V1.10	V.Arnost
*		Added stereo capabilities
*	Aug-13-1994	V1.20	V.Arnost
*		Stereo capabilities made functional
*	Aug-24-1994	V1.30	V.Arnost
*		Added Adlib and SB Pro II detection
*	Oct-30-1994	V1.40	V.Arnost
*		Added BLASTER variable parsing
*	Apr-14-1995	V1.50	V.Arnost
*              Some declarations moved from adlib.h to deftypes.h
*	Jul-22-1995	V1.60	V.Arnost
*		Ported to Watcom C
*		Simplified WriteChannel() and WriteValue()
*	Jul-24-1995	V1.61	V.Arnost
*		DetectBlaster() moved to MLMISC.C
*	Aug-8-1995	V1.62	V.Arnost
*		Module renamed to MLOPL_IO.C and functions renamed to OPLxxx
*		Mixer-related functions moved to module MLSBMIX.C
*	Sep-8-1995	V1.63	V.Arnost
*		OPLwriteReg() routine sped up on OPL3 cards
*	Mar-1-1996	V1.64	V.Arnost
*		Cleaned up the source
*/



OPLio::~OPLio()
{
}

void OPLio::SetClockRate(double samples_per_tick)
{
}

void OPLio::WriteDelay(int ticks)
{
}

void OPLio::OPLwriteReg(int which, uint32_t reg, uint8_t data)
{
	if (1)//IsOPL3)
	{
		reg |= (which & 1) << 8;
		which >>= 1;
	}
	if (chips[which] != NULL)
	{
		chips[which]->WriteReg(reg, data);
	}
}

/*
* Write to an operator pair. To be used for register bases of 0x20, 0x40,
* 0x60, 0x80 and 0xE0.
*/
void OPLio::OPLwriteChannel(uint32_t regbase, uint32_t channel, uint8_t data1, uint8_t data2)
{
	// Same as modifiers[9] array
	static const uint32_t op_num[OPL2CHANNELS] = {
		0x00, 0x01, 0x02, 0x08, 0x09, 0x0A, 0x10, 0x11, 0x12};

	uint32_t which = channel / OPL2CHANNELS;
	uint32_t reg = regbase + op_num[channel % OPL2CHANNELS];
	OPLwriteReg (which, reg, data1);
	OPLwriteReg (which, reg+3, data2);
}

/*
* Write to channel a single value. To be used for register bases of
* 0xA0, 0xB0 and 0xC0.
*/
void OPLio::OPLwriteValue(uint32_t regbase, uint32_t channel, uint8_t value)
{
	uint32_t which = channel / OPL2CHANNELS;
	uint32_t reg = regbase + (channel % OPL2CHANNELS);
	OPLwriteReg (which, reg, value);
}

static uint16_t frequencies[] =
{
	0x133, 0x133, 0x134, 0x134, 0x135, 0x136, 0x136, 0x137, 0x137, 0x138, 0x138, 0x139,
	0x139, 0x13a, 0x13b, 0x13b, 0x13c, 0x13c, 0x13d, 0x13d, 0x13e, 0x13f, 0x13f, 0x140,
	0x140, 0x141, 0x142, 0x142, 0x143, 0x143, 0x144, 0x144, 0x145, 0x146, 0x146, 0x147,
	0x147, 0x148, 0x149, 0x149, 0x14a, 0x14a, 0x14b, 0x14c, 0x14c, 0x14d, 0x14d, 0x14e,
	0x14f, 0x14f, 0x150, 0x150, 0x151, 0x152, 0x152, 0x153, 0x153, 0x154, 0x155, 0x155,
	0x156, 0x157, 0x157, 0x158, 0x158, 0x159, 0x15a, 0x15a, 0x15b, 0x15b, 0x15c, 0x15d,
	0x15d, 0x15e, 0x15f, 0x15f, 0x160, 0x161, 0x161, 0x162, 0x162, 0x163, 0x164, 0x164,
	0x165, 0x166, 0x166, 0x167, 0x168, 0x168, 0x169, 0x16a, 0x16a, 0x16b, 0x16c, 0x16c,
	0x16d, 0x16e, 0x16e, 0x16f, 0x170, 0x170, 0x171, 0x172, 0x172, 0x173, 0x174, 0x174,
	0x175, 0x176, 0x176, 0x177, 0x178, 0x178, 0x179, 0x17a, 0x17a, 0x17b, 0x17c, 0x17c,
	0x17d, 0x17e, 0x17e, 0x17f, 0x180, 0x181, 0x181, 0x182, 0x183, 0x183, 0x184, 0x185,
	0x185, 0x186, 0x187, 0x188, 0x188, 0x189, 0x18a, 0x18a, 0x18b, 0x18c, 0x18d, 0x18d,
	0x18e, 0x18f, 0x18f, 0x190, 0x191, 0x192, 0x192, 0x193, 0x194, 0x194, 0x195, 0x196,
	0x197, 0x197, 0x198, 0x199, 0x19a, 0x19a, 0x19b, 0x19c, 0x19d, 0x19d, 0x19e, 0x19f,
	0x1a0, 0x1a0, 0x1a1, 0x1a2, 0x1a3, 0x1a3, 0x1a4, 0x1a5, 0x1a6, 0x1a6, 0x1a7, 0x1a8,
	0x1a9, 0x1a9, 0x1aa, 0x1ab, 0x1ac, 0x1ad, 0x1ad, 0x1ae, 0x1af, 0x1b0, 0x1b0, 0x1b1,
	0x1b2, 0x1b3, 0x1b4, 0x1b4, 0x1b5, 0x1b6, 0x1b7, 0x1b8, 0x1b8, 0x1b9, 0x1ba, 0x1bb,
	0x1bc, 0x1bc, 0x1bd, 0x1be, 0x1bf, 0x1c0, 0x1c0, 0x1c1, 0x1c2, 0x1c3, 0x1c4, 0x1c4,
	0x1c5, 0x1c6, 0x1c7, 0x1c8, 0x1c9, 0x1c9, 0x1ca, 0x1cb, 0x1cc, 0x1cd, 0x1ce, 0x1ce,
	0x1cf, 0x1d0, 0x1d1, 0x1d2, 0x1d3, 0x1d3, 0x1d4, 0x1d5, 0x1d6, 0x1d7, 0x1d8, 0x1d8,
	0x1d9, 0x1da, 0x1db, 0x1dc, 0x1dd, 0x1de, 0x1de, 0x1df, 0x1e0, 0x1e1, 0x1e2, 0x1e3,
	0x1e4, 0x1e5, 0x1e5, 0x1e6, 0x1e7, 0x1e8, 0x1e9, 0x1ea, 0x1eb, 0x1ec, 0x1ed, 0x1ed,
	0x1ee, 0x1ef, 0x1f0, 0x1f1, 0x1f2, 0x1f3, 0x1f4, 0x1f5, 0x1f6, 0x1f6, 0x1f7, 0x1f8,
	0x1f9, 0x1fa, 0x1fb, 0x1fc, 0x1fd, 0x1fe, 0x1ff, 0x200,
	
	0x201, 0x201, 0x202, 0x203, 0x204, 0x205, 0x206, 0x207, 0x208, 0x209, 0x20a, 0x20b, 0x20c, 0x20d, 0x20e, 0x20f,
	0x210, 0x210, 0x211, 0x212, 0x213, 0x214, 0x215, 0x216, 0x217, 0x218, 0x219, 0x21a, 0x21b, 0x21c, 0x21d, 0x21e,
	
	0x21f, 0x220, 0x221, 0x222, 0x223, 0x224, 0x225, 0x226, 0x227, 0x228, 0x229, 0x22a, 0x22b, 0x22c, 0x22d, 0x22e,
	0x22f, 0x230, 0x231, 0x232, 0x233, 0x234, 0x235, 0x236, 0x237, 0x238, 0x239, 0x23a, 0x23b, 0x23c, 0x23d, 0x23e,
	
	0x23f, 0x240, 0x241, 0x242, 0x244, 0x245, 0x246, 0x247, 0x248, 0x249, 0x24a, 0x24b, 0x24c, 0x24d, 0x24e, 0x24f,
	0x250, 0x251, 0x252, 0x253, 0x254, 0x256, 0x257, 0x258, 0x259, 0x25a, 0x25b, 0x25c, 0x25d, 0x25e, 0x25f, 0x260,

	0x262, 0x263, 0x264, 0x265, 0x266, 0x267, 0x268, 0x269, 0x26a, 0x26c, 0x26d, 0x26e, 0x26f, 0x270, 0x271, 0x272,
	0x273, 0x275, 0x276, 0x277, 0x278, 0x279, 0x27a, 0x27b, 0x27d, 0x27e, 0x27f, 0x280, 0x281, 0x282, 0x284, 0x285,
	
	0x286, 0x287, 0x288, 0x289, 0x28b, 0x28c, 0x28d, 0x28e, 0x28f, 0x290, 0x292, 0x293, 0x294, 0x295, 0x296, 0x298,
	0x299, 0x29a, 0x29b, 0x29c, 0x29e, 0x29f, 0x2a0, 0x2a1, 0x2a2, 0x2a4, 0x2a5, 0x2a6, 0x2a7, 0x2a9, 0x2aa, 0x2ab,
	
	0x2ac, 0x2ae, 0x2af, 0x2b0, 0x2b1, 0x2b2, 0x2b4, 0x2b5, 0x2b6, 0x2b7, 0x2b9, 0x2ba, 0x2bb, 0x2bd, 0x2be, 0x2bf,
	0x2c0, 0x2c2, 0x2c3, 0x2c4, 0x2c5, 0x2c7, 0x2c8, 0x2c9, 0x2cb, 0x2cc, 0x2cd, 0x2ce, 0x2d0, 0x2d1, 0x2d2, 0x2d4,

	0x2d5, 0x2d6, 0x2d8, 0x2d9, 0x2da, 0x2dc, 0x2dd, 0x2de, 0x2e0, 0x2e1, 0x2e2, 0x2e4, 0x2e5, 0x2e6, 0x2e8, 0x2e9,
	0x2ea, 0x2ec, 0x2ed, 0x2ee, 0x2f0, 0x2f1, 0x2f2, 0x2f4, 0x2f5, 0x2f6, 0x2f8, 0x2f9, 0x2fb, 0x2fc, 0x2fd, 0x2ff,
	
	0x300, 0x302, 0x303, 0x304, 0x306, 0x307, 0x309, 0x30a, 0x30b, 0x30d, 0x30e, 0x310, 0x311, 0x312, 0x314, 0x315,
	0x317, 0x318, 0x31a, 0x31b, 0x31c, 0x31e, 0x31f, 0x321, 0x322, 0x324, 0x325, 0x327, 0x328, 0x329, 0x32b, 0x32c,
	
	0x32e, 0x32f, 0x331, 0x332, 0x334, 0x335, 0x337, 0x338, 0x33a, 0x33b, 0x33d, 0x33e, 0x340, 0x341, 0x343, 0x344,
	0x346, 0x347, 0x349, 0x34a, 0x34c, 0x34d, 0x34f, 0x350, 0x352, 0x353, 0x355, 0x357, 0x358, 0x35a, 0x35b, 0x35d,

	0x35e, 0x360, 0x361, 0x363, 0x365, 0x366, 0x368, 0x369, 0x36b, 0x36c, 0x36e, 0x370, 0x371, 0x373, 0x374, 0x376,
	0x378, 0x379, 0x37b, 0x37c, 0x37e, 0x380, 0x381, 0x383, 0x384, 0x386, 0x388, 0x389, 0x38b, 0x38d, 0x38e, 0x390,
	
	0x392, 0x393, 0x395, 0x397, 0x398, 0x39a, 0x39c, 0x39d, 0x39f, 0x3a1, 0x3a2, 0x3a4, 0x3a6, 0x3a7, 0x3a9, 0x3ab,
	0x3ac, 0x3ae, 0x3b0, 0x3b1, 0x3b3, 0x3b5, 0x3b7, 0x3b8, 0x3ba, 0x3bc, 0x3bd, 0x3bf, 0x3c1, 0x3c3, 0x3c4, 0x3c6,
	
	0x3c8, 0x3ca, 0x3cb, 0x3cd, 0x3cf, 0x3d1, 0x3d2, 0x3d4, 0x3d6, 0x3d8, 0x3da, 0x3db, 0x3dd, 0x3df, 0x3e1, 0x3e3,
	0x3e4, 0x3e6, 0x3e8, 0x3ea, 0x3ec, 0x3ed, 0x3ef, 0x3f1, 0x3f3, 0x3f5, 0x3f6, 0x3f8, 0x3fa, 0x3fc, 0x3fe, 0x36c
};

/*
* Write frequency/octave/keyon data to a channel
* [RH] This is totally different from the original MUS library code
* but matches exactly what DMX does. I haven't a clue why there are 284
* special bytes at the beginning of the table for the first few notes.
* That last byte in the table doesn't look right, either, but that's what
* it really is.
*/
void OPLio::OPLwriteFreq(uint32_t channel, uint32_t note, uint32_t pitch, uint32_t keyon)
{
	int octave = 0;
	int j = (note << 5) + pitch;

	if (j < 0)
	{
		j = 0;
	}
	else if (j >= 284)
	{
		j -= 284;
		octave = j / (32*12);
		if (octave > 7)
		{
			octave = 7;
		}
		j = (j % (32*12)) + 284;
	}
	int i = frequencies[j] | (octave << 10);

	OPLwriteValue (0xA0, channel, (uint8_t)i);
	OPLwriteValue (0xB0, channel, (uint8_t)(i>>8)|(keyon<<5));
}

/*
* Adjust volume value (register 0x40)
*/
inline uint32_t OPLio::OPLconvertVolume(uint32_t data, uint32_t volume)
{
	static uint8_t volumetable[128] = {
		0,   1,   3,   5,   6,   8,  10,  11,
		13,  14,  16,  17,  19,  20,  22,  23,
		25,  26,  27,  29,  30,  32,  33,  34,
		36,  37,  39,  41,  43,  45,  47,  49,
		50,  52,  54,  55,  57,  59,  60,  61,
		63,  64,  66,  67,  68,  69,  71,  72,
		73,  74,  75,  76,  77,  79,  80,  81,
		82,  83,  84,  84,  85,  86,  87,  88,
		89,  90,  91,  92,  92,  93,  94,  95,
		96,  96,  97,  98,  99,  99, 100, 101,
		101, 102, 103, 103, 104, 105, 105, 106,
		107, 107, 108, 109, 109, 110, 110, 111,
		112, 112, 113, 113, 114, 114, 115, 115,
		116, 117, 117, 118, 118, 119, 119, 120,
		120, 121, 121, 122, 122, 123, 123, 123,
		124, 124, 125, 125, 126, 126, 127, 127};

	return 0x3F - (((0x3F - data) *
		(uint32_t)volumetable[volume <= 127 ? volume : 127]) >> 7);

}

uint32_t OPLio::OPLpanVolume(uint32_t volume, int pan)
{
	if (pan >= 0)
		return volume;
	else
		return (volume * (pan + 64)) / 64;
}

/*
* Write volume data to a channel
*/
void OPLio::OPLwriteVolume(uint32_t channel, struct OPL2instrument *instr, uint32_t volume)
{
	if (instr != 0)
	{
		OPLwriteChannel(0x40, channel, ((instr->feedback & 1) ?
			OPLconvertVolume(instr->level_1, volume) : instr->level_1) | instr->scale_1,
			OPLconvertVolume(instr->level_2, volume) | instr->scale_2);
	}
}

/*
* Write pan (balance) data to a channel
*/
void OPLio::OPLwritePan(uint32_t channel, struct OPL2instrument *instr, int pan)
{
	if (instr != 0)
	{
		uint8_t bits;
		if (pan < -36) bits = 0x10;		// left
		else if (pan > 36) bits = 0x20;	// right
		else bits = 0x30;			// both

		OPLwriteValue(0xC0, channel, instr->feedback | bits);

		// Set real panning if we're using emulated chips.
		int chanper = /*IsOPL3 ?*/ OPL3CHANNELS /*: OPL2CHANNELS*/;
		int which = channel / chanper;
		if (chips[which] != NULL)
		{
			// This is the MIDI-recommended pan formula. 0 and 1 are
			// both hard left so that 64 can be perfectly center.
			// (Note that the 'pan' passed to this function is the
			// MIDI pan position, subtracted by 64.)
			double level = (pan <= -63) ? 0 : (pan + 64 - 1) / 126.0;
			chips[which]->SetPanning(channel % chanper,
				(float)cos(HALF_PI * level), (float)sin(HALF_PI * level));
		}
	}
}

/*
* Write an instrument to a channel
*
* Instrument layout:
*
*   Operator1  Operator2  Descr.
*    data[0]    data[7]   reg. 0x20 - tremolo/vibrato/sustain/KSR/multi
*    data[1]    data[8]   reg. 0x60 - attack rate/decay rate
*    data[2]    data[9]   reg. 0x80 - sustain level/release rate
*    data[3]    data[10]  reg. 0xE0 - waveform select
*    data[4]    data[11]  reg. 0x40 - key scale level
*    data[5]    data[12]  reg. 0x40 - output level (bottom 6 bits only)
*          data[6]        reg. 0xC0 - feedback/AM-FM (both operators)
*/
void OPLio::OPLwriteInstrument(uint32_t channel, OPL2instrument *instr)
{
	OPLwriteChannel(0x40, channel, 0x3F, 0x3F);		// no volume
	OPLwriteChannel(0x20, channel, instr->trem_vibr_1, instr->trem_vibr_2);
	OPLwriteChannel(0x60, channel, instr->att_dec_1,   instr->att_dec_2);
	OPLwriteChannel(0x80, channel, instr->sust_rel_1,  instr->sust_rel_2);
	OPLwriteChannel(0xE0, channel, instr->wave_1,      instr->wave_2);
	OPLwriteValue  (0xC0, channel, instr->feedback | 0x30);
}

/*
;	Stuff for the AdLib
;	Operator registers
alChar				=	20h
alScale				=	40h
alAttack			=	60h
alSus				=	80h
alWave				=	0e0h
;	Channel registers
alFreqL				=	0a0h
alFreqH				=	0b0h
alFeedCon			=	0c0h
;	Global registers
alEffects			=	0bdh
*/

void OPLio::OPLwriteInstrument(uint32_t channel, inst_t *instr)
{
	OPLwriteChannel(0x40, channel, 0x3F, 0x3F);		// no volume
	OPLwriteChannel(0x20, channel, instr->mChar,    instr->cChar);
	OPLwriteChannel(0x60, channel, instr->mAttack,  instr->cAttack);
	OPLwriteChannel(0x80, channel, instr->mSus,     instr->cSus);
	OPLwriteChannel(0xE0, channel, instr->mWave,    instr->cWave);
	//OPLwriteValue  (0xC0, channel, instr->nConn | 0x30);
}

/*
* Stop all sounds
*/
void OPLio::OPLshutup(void)
{
	uint32_t i;

	for(i = 0; i < OPLchannels; i++)
	{
		OPLwriteChannel(0x40, i, 0x3F, 0x3F);	// turn off volume
		OPLwriteChannel(0x60, i, 0xFF, 0xFF);	// the fastest attack, decay
		OPLwriteChannel(0x80, i, 0x0F, 0x0F);	// ... and release
		OPLwriteValue(0xB0, i, 0);		// KEY-OFF
	}
}

/*
* Initialize hardware upon startup
*/
int OPLio::OPLinit(uint32_t numchips, bool stereo, bool initopl3)
{
	assert(numchips >= 1 && numchips <= countof(chips));
	uint32_t i;
	//IsOPL3 = (opl_core == 1 || opl_core == 2);

	memset(chips, 0, sizeof(chips));
	//if (IsOPL3)
		numchips = (numchips + 1) >> 1;
	for (i = 0; i < numchips; ++i)
	{
		OPLEmul *chip = /*IsOPL3 ? (opl_core == 1 ? DBOPLCreate(stereo) :*/ JavaOPLCreate(stereo)/*) : YM3812Create(stereo)*/;
		if (chip == NULL)
		{
			break;
		}
		chips[i] = chip;
	}
	NumChips = i;
	OPLchannels = i * (/*IsOPL3 ?*/ OPL3CHANNELS /*: OPL2CHANNELS*/);
	OPLwriteInitState(initopl3);
	return i;
}

void OPLio::OPLwriteInitState(bool initopl3)
{
	for (uint32_t i = 0; i < NumChips; ++i)
	{
		int chip = i << 1;//(int)IsOPL3;
		if (/*IsOPL3 &&*/ initopl3)
		{
			OPLwriteReg(chip, 0x105, 0x01);	// enable YMF262/OPL3 mode
			OPLwriteReg(chip, 0x104, 0x00);	// disable 4-operator mode
		}
		OPLwriteReg(chip, 0x01, 0x20);	// enable Waveform Select
		OPLwriteReg(chip, 0x0B, 0x40);	// turn off CSW mode
		OPLwriteReg(chip, 0xBD, 0x00);	// set vibrato/tremolo depth to low, set melodic mode
	}
	OPLshutup();
}

/*
* Deinitialize hardware before shutdown
*/
void OPLio::OPLdeinit(void)
{
	for (size_t i = 0; i < countof(chips); ++i)
	{
		if (chips[i] != NULL)
		{
			delete chips[i];
			chips[i] = NULL;
		}
	}
}


#define MOD_MIN		40		/* vibrato threshold */
#define HIGHEST_NOTE 127

void OPLmusicFile::writeFrequency(uint32_t slot, uint32_t note, int pitch, uint32_t keyOn)
{
	io->OPLwriteFreq (slot, note, pitch, keyOn);
}

void OPLmusicFile::writeModulation(uint32_t slot, struct OPL2instrument *instr, int state)
{
	if (state)
		state = 0x40;	/* enable Frequency Vibrato */
	io->OPLwriteChannel(0x20, slot,
		(instr->feedback & 1) ? (instr->trem_vibr_1 | state) : instr->trem_vibr_1,
		instr->trem_vibr_2 | state);
}

uint32_t OPLmusicFile::calcVolume(uint32_t channelVolume, uint32_t channelExpression, uint32_t noteVolume)
{
	noteVolume = ((uint32_t)channelVolume * channelExpression * noteVolume) / (127*127);
	if (noteVolume > 127)
		return 127;
	else
		return noteVolume;
}

int OPLmusicFile::occupyChannel(uint32_t slot, uint32_t channel,
						 int note, int volume, struct OP2instrEntry *instrument, uint8_t secondary)
{
	struct OPL2instrument *instr;
	struct channelEntry *ch = &channels[slot];

	ch->channel = channel;
	ch->note = note;
	ch->flags = secondary ? CH_SECONDARY : 0;
	if (driverdata.channelModulation[channel] >= MOD_MIN)
		ch->flags |= CH_VIBRATO;
	ch->time = MLtime;
	if (volume == -1)
		volume = driverdata.channelLastVolume[channel];
	else
		driverdata.channelLastVolume[channel] = volume;
	ch->realvolume = calcVolume(driverdata.channelVolume[channel],
		driverdata.channelExpression[channel], ch->volume = volume);
	if (instrument->flags & FL_FIXED_PITCH)
		note = instrument->note;
	else if (channel == PERCUSSION)
		note = 60;			// C-5
	if (secondary && (instrument->flags & FL_DOUBLE_VOICE))
		ch->finetune = (instrument->finetune - 0x80) >> 1;
	else
		ch->finetune = 0;
	ch->pitch = ch->finetune + driverdata.channelPitch[channel];
	if (secondary)
		instr = &instrument->instr[1];
	else
		instr = &instrument->instr[0];
	ch->instr = instr;
	if (channel != PERCUSSION && !(instrument->flags & FL_FIXED_PITCH))
	{
		if ( (note += instr->basenote) < 0)
			while ((note += 12) < 0) {}
		else if (note > HIGHEST_NOTE)
			while ((note -= 12) > HIGHEST_NOTE) {}
	}
	ch->realnote = note;

	io->OPLwriteInstrument(slot, instr);
	if (ch->flags & CH_VIBRATO)
		writeModulation(slot, instr, 1);
	io->OPLwritePan(slot, instr, driverdata.channelPan[channel]);
	io->OPLwriteVolume(slot, instr, ch->realvolume);
	writeFrequency(slot, note, ch->pitch, 1);
	return slot;
}

int OPLmusicFile::releaseChannel(uint32_t slot, uint32_t killed)
{
	struct channelEntry *ch = &channels[slot];
	writeFrequency(slot, ch->realnote, ch->pitch, 0);
	ch->channel |= CH_FREE;
	ch->time = MLtime;
	ch->flags = CH_FREE;
	if (killed)
	{
		io->OPLwriteChannel(0x80, slot, 0x0F, 0x0F);  // release rate - fastest
		io->OPLwriteChannel(0x40, slot, 0x3F, 0x3F);  // no volume
	}
	return slot;
}

int OPLmusicFile::releaseSustain(uint32_t channel)
{
	uint32_t i;
	uint32_t id = channel;

	for(i = 0; i < io->OPLchannels; i++)
	{
		if (channels[i].channel == id && channels[i].flags & CH_SUSTAIN)
			releaseChannel(i, 0);
	}
	return 0;
}

int OPLmusicFile::findFreeChannel(uint32_t flag, uint32_t channel, uint8_t note)
{
	uint32_t i;

	uint32_t bestfit = 0;
	uint32_t bestvoice = 0;

	for (i = 0; i < io->OPLchannels; ++i)
	{
		uint32_t magic;

		magic = ((channels[i].flags & CH_FREE) << 24) |
				((channels[i].note == note &&
					channels[i].channel == channel) << 30) |
				((channels[i].flags & CH_SUSTAIN) << 28) |
				((MLtime - channels[i].time) & 0x1fffffff);
		if (magic > bestfit)
		{
			bestfit = magic;
			bestvoice = i;
		}
	}
	if ((flag & 1) && !(bestfit & 0x80000000))
	{ // No free channels good enough
		return -1;
	}
	releaseChannel (bestvoice, 1);
	return bestvoice;
}

struct OP2instrEntry *OPLmusicFile::getInstrument(uint32_t channel, uint8_t note)
{
	uint32_t instrnumber;

	if (channel == PERCUSSION)
	{
		if (note < 35 || note > 81)
			return NULL;		/* wrong percussion number */
		instrnumber = note + (128-35);
	}
	else
	{
		instrnumber = driverdata.channelInstr[channel];
	}

	if (OPLinstruments)
		return &OPLinstruments[instrnumber];
	else
		return NULL;
}


// code 1: play note
void OPLmusicFile::OPLplayNote(uint32_t channel, uint8_t note, int volume)
{
	int i;
	struct OP2instrEntry *instr;

	if (volume == 0)
	{
		OPLreleaseNote (channel, note);
		return;
	}

	if ( (instr = getInstrument(channel, note)) == NULL )
		return;

	if ( (i = findFreeChannel((channel == PERCUSSION) ? 2 : 0, channel, note)) != -1)
	{
		occupyChannel(i, channel, note, volume, instr, 0);
		if ((instr->flags & FL_DOUBLE_VOICE) && !opl_singlevoice)
		{
			if ( (i = findFreeChannel((channel == PERCUSSION) ? 3 : 1, channel, note)) != -1)
				occupyChannel(i, channel, note, volume, instr, 1);
		}
	}
}

// code 0: release note
void OPLmusicFile::OPLreleaseNote(uint32_t channel, uint8_t note)
{
	uint32_t i;
	uint32_t id = channel;
	uint32_t sustain = driverdata.channelSustain[channel];

	for(i = 0; i < io->OPLchannels; i++)
	{
		if (channels[i].channel == id && channels[i].note == note)
		{
			if (sustain < 0x40)
				releaseChannel(i, 0);
			else
				channels[i].flags |= CH_SUSTAIN;
		}
	}
}

// code 2: change pitch wheel (bender)
void OPLmusicFile::OPLpitchWheel(uint32_t channel, int pitch)
{
	uint32_t i;
	uint32_t id = channel;

	// Convert pitch from 14-bit to 7-bit, then scale it, since the player
	// code only understands sensitivities of 2 semitones.
	pitch = (pitch - 8192) * driverdata.channelPitchSens[channel] / (200 * 128) + 64;
	driverdata.channelPitch[channel] = pitch;
	for(i = 0; i < io->OPLchannels; i++)
	{
		struct channelEntry *ch = &channels[i];
		if (ch->channel == id)
		{
			ch->time = MLtime;
			ch->pitch = ch->finetune + pitch;
			writeFrequency(i, ch->realnote, ch->pitch, 1);
		}
	}
}

// code 4: change control
void OPLmusicFile::OPLchangeControl(uint32_t channel, uint8_t controller, int value)
{
	uint32_t i;
	uint32_t id = channel;

	switch (controller)
	{
	case ctrlPatch:			/* change instrument */
		OPLprogramChange(channel, value);
		break;

	case ctrlModulation:
		driverdata.channelModulation[channel] = value;
		for(i = 0; i < io->OPLchannels; i++)
		{
			struct channelEntry *ch = &channels[i];
			if (ch->channel == id)
			{
				uint8_t flags = ch->flags;
				ch->time = MLtime;
				if (value >= MOD_MIN)
				{
					ch->flags |= CH_VIBRATO;
					if (ch->flags != flags)
						writeModulation(i, ch->instr, 1);
				} else {
					ch->flags &= ~CH_VIBRATO;
					if (ch->flags != flags)
						writeModulation(i, ch->instr, 0);
				}
			}
		}
		break;

	case ctrlVolume:		/* change volume */
		driverdata.channelVolume[channel] = value;
		/* fall-through */
	case ctrlExpression:	/* change expression */
		if (controller == ctrlExpression)
		{
			driverdata.channelExpression[channel] = value;
		}
		for(i = 0; i < io->OPLchannels; i++)
		{
			struct channelEntry *ch = &channels[i];
			if (ch->channel == id)
			{
				ch->time = MLtime;
				ch->realvolume = calcVolume(driverdata.channelVolume[channel],
					driverdata.channelExpression[channel], ch->volume);
				io->OPLwriteVolume(i, ch->instr, ch->realvolume);
			}
		}
		break;

	case ctrlPan:			/* change pan (balance) */
		driverdata.channelPan[channel] = value -= 64;
		for(i = 0; i < io->OPLchannels; i++)
		{
			struct channelEntry *ch = &channels[i];
			if (ch->channel == id)
			{
				ch->time = MLtime;
				io->OPLwritePan(i, ch->instr, value);
			}
		}
		break;

	case ctrlSustainPedal:		/* change sustain pedal (hold) */
		driverdata.channelSustain[channel] = value;
		if (value < 0x40)
			releaseSustain(channel);
		break;

	case ctrlNotesOff:			/* turn off all notes that are not sustained */
		for (i = 0; i < io->OPLchannels; ++i)
		{
			if (channels[i].channel == id)
			{
				if (driverdata.channelSustain[id] < 0x40)
					releaseChannel(i, 0);
				else
					channels[i].flags |= CH_SUSTAIN;
			}
		}
		break;

	case ctrlSoundsOff:			/* release all notes for this channel */
		for (i = 0; i < io->OPLchannels; ++i)
		{
			if (channels[i].channel == id)
			{
				releaseChannel(i, 0);
			}
		}
		break;

	case ctrlRPNHi:
		driverdata.channelRPN[id] = (driverdata.channelRPN[id] & 0x007F) | (value << 7);
		break;

	case ctrlRPNLo:
		driverdata.channelRPN[id] = (driverdata.channelRPN[id] & 0x3F80) | value;
		break;

	case ctrlNRPNLo:
	case ctrlNRPNHi:
		driverdata.channelRPN[id] = 0x3FFF;
		break;

	case ctrlDataEntryHi:
		if (driverdata.channelRPN[id] == 0)
		{
			driverdata.channelPitchSens[id] = value * 100 + (driverdata.channelPitchSens[id] % 100);
		}
		break;

	case ctrlDataEntryLo:
		if (driverdata.channelRPN[id] == 0)
		{
			driverdata.channelPitchSens[id] = value + (driverdata.channelPitchSens[id] / 100) * 100;
		}
		break;
	}
}

void OPLmusicFile::OPLresetControllers(uint32_t chan, int vol)
{
	driverdata.channelVolume[chan] = vol;
	driverdata.channelExpression[chan] = 127;
	driverdata.channelSustain[chan] = 0;
	driverdata.channelLastVolume[chan] = 64;
	driverdata.channelPitch[chan] = 64;
	driverdata.channelRPN[chan] = 0x3fff;
	driverdata.channelPitchSens[chan] = 200;
}

void OPLmusicFile::OPLprogramChange(uint32_t channel, int value)
{
	driverdata.channelInstr[channel] = value;
}

void OPLmusicFile::OPLplayMusic(int vol)
{
	uint32_t i;

	for (i = 0; i < CHANNELS; i++)
	{
		OPLresetControllers(i, vol);
	}
}

void OPLmusicFile::OPLstopMusic()
{
	uint32_t i;
	for(i = 0; i < io->OPLchannels; i++)
		if (!(channels[i].flags & CH_FREE))
			releaseChannel(i, 1);
}

int OPLmusicFile::OPLloadBank (MemChunk &data)
{
	static const uint8_t masterhdr[8] = { '#','O','P','L','_','I','I','#' };
	struct OP2instrEntry *instruments;

	uint8_t filehdr[8];

	data.read (filehdr, 8);
	if (memcmp(filehdr, masterhdr, 8))
		return -2;			/* bad instrument file */
	if ( (instruments = (struct OP2instrEntry *)calloc(OP2INSTRCOUNT, OP2INSTRSIZE)) == NULL)
		return -3;			/* not enough memory */
	data.read (instruments, OP2INSTRSIZE * OP2INSTRCOUNT);
	if (OPLinstruments != NULL)
	{
		free(OPLinstruments);
	}
	OPLinstruments = instruments;
#if 0
	for (int i = 0; i < 175; ++i)
	{
		wxLogMessage ("%3d.%-33s%3d %3d %3d %d\n", i,
			(uint8_t *)data+6308+i*32,
			OPLinstruments[i].instr[0].basenote,
			OPLinstruments[i].instr[1].basenote,
			OPLinstruments[i].note,
			OPLinstruments[i].flags);
	}
#endif
	return 0;
}

void OPLmusicFile::ResetChips ()
{
	io->OPLdeinit ();
	NumChips = io->OPLinit(MIN(*opl_numchips, 2), FullPan);
}

OPLmusicFile::OPLmusicFile (FILE *file, const uint8_t *musiccache, size_t len)
{
	memset (this, 0, sizeof(*this));
	scoredata = NULL;
	NextTickIn = 0;
	LastOffset = 0;
	NumChips = MIN(*opl_numchips, 2);
	Looping = false;
	FullPan = false;
	io = NULL;
	io = new OPLio;
	ScoreLen = len;

	if (io == NULL)
	{
		return;
	}

	scoredata = new uint8_t[len];

	if (file)
	{
		if (fread (scoredata, 1, len, file) != (size_t)len)
		{
fail:		delete[] scoredata;
			scoredata = NULL;
			return;
		}
	}
	else
	{
		memcpy(scoredata, &musiccache[0], len);
	}

	if (0 == (NumChips = io->OPLinit(NumChips)))
	{
		goto fail;
	}

	// Check for RDosPlay raw OPL format
	if (((uint32_t *)scoredata)[0] == MAKE_ID('R','A','W','A') &&
			 ((uint32_t *)scoredata)[1] == MAKE_ID('D','A','T','A'))
	{
		RawPlayer = RDosPlay;
		if (*(uint16_t *)(scoredata + 8) == 0)
		{ // A clock speed of 0 is bad
			*(uint16_t *)(scoredata + 8) = 0xFFFF; 
		}
		SamplesPerTick = LittleShort(*(uint16_t *)(scoredata + 8)) / ADLIB_CLOCK_MUL;
	}
	// Check for DosBox OPL dump
	else if (((uint32_t *)scoredata)[0] == MAKE_ID('D','B','R','A') &&
		((uint32_t *)scoredata)[1] == MAKE_ID('W','O','P','L'))
	{
		if (((uint32_t *)scoredata)[2] == MAKE_ID(0,0,1,0))
		{
			RawPlayer = DosBox1;
			SamplesPerTick = OPL_SAMPLE_RATE / 1000;
			ScoreLen = MIN(len - 24, LittleLong(((uint32_t *)scoredata)[4])) + 24;
		}
		else if (((uint32_t *)scoredata)[2] == MAKE_ID(2,0,0,0))
		{
			if (scoredata[20] != 0)
			{
				wxLogMessage("Unsupported DOSBox Raw OPL format %d", scoredata[20]);
				goto fail;
			}
			if (scoredata[21] != 0)
			{
				wxLogMessage("Unsupported DOSBox Raw OPL compression %d", scoredata[21]);
				goto fail;
			}
			RawPlayer = DosBox2;
			SamplesPerTick = OPL_SAMPLE_RATE / 1000;
			int headersize = 0x1A + scoredata[0x19];
			ScoreLen = MIN(len - headersize, LittleLong(((uint32_t *)scoredata)[3]) * 2) + headersize;
		}
		else
		{
			wxLogMessage("Unsupported DOSBox Raw OPL version %d.%d", LittleShort(((uint16_t *)scoredata)[4]), LittleShort(((uint16_t *)scoredata)[5]));
			goto fail;
		}
	}
	// Check for modified IMF format (includes a header)
	else if (((uint32_t *)scoredata)[0] == MAKE_ID('A','D','L','I') &&
		     scoredata[4] == 'B' && scoredata[5] == 1)
	{
		int songlen;
		uint8_t *max = scoredata + ScoreLen;
		RawPlayer = IMF;
		SamplesPerTick = OPL_SAMPLE_RATE / imf_rate;

		score = scoredata + 6;
		// Skip track and game name
		for (int i = 2; i != 0; --i)
		{
			while (score < max && *score++ != '\0') {}
		}
		if (score < max) score++;	// Skip unknown uint8_t
		if (score + 8 > max)
		{ // Not enough room left for song data
			delete[] scoredata;
			scoredata = NULL;
			return;
		}
		songlen = LittleLong(*(uint32_t *)score);
		if (songlen != 0 && (songlen +=4) < ScoreLen - (score - scoredata))
		{
			ScoreLen = songlen + int(score - scoredata);
		}
	}
	// Check for Adlib sound effect
	else if ((len > 26) && (scoredata[22] < 8) && 
		((unsigned)(24 + READ_L32(scoredata, 0)) <= len))
	{
		RawPlayer = AudioT;
		SamplesPerTick = OPL_SAMPLE_RATE / 140;
		oplsound_t * sound = (oplsound_t*)scoredata;
		Octave = sound->octave;
		io->OPLwriteInstrument(0, &sound->inst);
		Note = false;
		ScoreLen = len - sizeof(oplsound_t);
		score = scoredata + sizeof(oplsound_t);
	}
	else
	{
		LOG_MESSAGE(2, "Unknown or unidentified OPL format");
		goto fail;
	}

	Restart ();
}

OPLmusicFile::~OPLmusicFile ()
{
	if (OPLinstruments != NULL) free(OPLinstruments);
	if (scoredata != NULL)
	{
		io->OPLdeinit ();
		delete io;
		delete[] scoredata;
		scoredata = NULL;
	}
}

bool OPLmusicFile::IsValid () const
{
	return scoredata != NULL;
}

void OPLmusicFile::SetLooping (bool loop)
{
	Looping = loop;
}

void OPLmusicFile::Restart ()
{
	if (scoredata == NULL)
	{
		wxLogMessage("Data was not loaded!");
		return;
	}
	OPLstopMusic ();
	OPLplayMusic (127);
	MLtime = 0;
	playingcount = 0;
	LastOffset = 0;
	WhichChip = 0;
	switch (RawPlayer)
	{
	case RDosPlay:
		score = scoredata + 10;
		SamplesPerTick = LittleShort(*(uint16_t *)(scoredata + 8)) / ADLIB_CLOCK_MUL;
		break;

	case DosBox1:
		score = scoredata + 24;
		SamplesPerTick = OPL_SAMPLE_RATE / 1000;
		break;

	case DosBox2:
		score = scoredata + 0x1A + scoredata[0x19];
		SamplesPerTick = OPL_SAMPLE_RATE / 1000;
		break;

	case IMF:
		score = scoredata + 6;
		SamplesPerTick = OPL_SAMPLE_RATE / imf_rate;

		// Skip track and game name
		for (int i = 2; i != 0; --i)
		{
			while (*score++ != '\0') {}
		}
		score++;	// Skip unknown uint8_t
		if (*(uint32_t *)score != 0)
		{
			score += 4;		// Skip song length
		}
		break;

	case AudioT:
		SamplesPerTick = OPL_SAMPLE_RATE / 140;
		score = scoredata + sizeof(oplsound_t);
		break;
	}
	io->SetClockRate(SamplesPerTick);
}

bool OPLmusicFile::ServiceStream (void *buff, int numbytes)
{
	float *samples1 = (float *)buff;
	int stereoshift = (int)(FullPan | /*io->IsOPL3*/1);
	int numsamples = numbytes / (sizeof(float) << stereoshift);
	bool prevEnded = false;
	bool res = true;

	memset(buff, 0, numbytes);

	while (numsamples > 0)
	{
		double ticky = NextTickIn;
		int tick_in = int(NextTickIn);
		int samplesleft = MIN(numsamples, tick_in);
		size_t i;

		if (samplesleft > 0)
		{
			for (i = 0; i < io->NumChips; ++i)
			{
				io->chips[i]->Update(samples1, samplesleft);
			}
			OffsetSamples(samples1, samplesleft << stereoshift);
			assert(NextTickIn == ticky);
			NextTickIn -= samplesleft;
			assert (NextTickIn >= 0);
			numsamples -= samplesleft;
			samples1 += samplesleft << stereoshift;
		}
		
		if (NextTickIn < 1)
		{
			int next = PlayTick();
			assert(next >= 0);
			if (next == 0)
			{ // end of song
				if (!Looping || prevEnded)
				{
					if (numsamples > 0)
					{
						for (i = 0; i < io->NumChips; ++i)
						{
							io->chips[i]->Update(samples1, samplesleft);
						}
						OffsetSamples(samples1, numsamples << stereoshift);
					}
					res = false;
					break;
				}
				else
				{
					// Avoid infinite loops from songs that do nothing but end
					prevEnded = true;
					Restart ();
				}
			}
			else
			{
				prevEnded = false;
				io->WriteDelay(next);
				NextTickIn += SamplesPerTick * next;
				assert (NextTickIn >= 0);
				MLtime += next;
			}
		}
	}
	return res;
}

bool OPLmusicFile::ServiceStreamI (int16_t *buff, int numbytes)
{
	int16_t *samples1 = buff;
	int stereoshift = (int)(FullPan | /*io->IsOPL3*/1);
	int numsamples = numbytes / (sizeof(int16_t) << stereoshift);
	bool prevEnded = false;
	bool res = true;

	memset(buff, 0, numbytes);

	while (numsamples > 0)
	{
		double ticky = NextTickIn;
		int tick_in = int(NextTickIn);
		int samplesleft = MIN(numsamples, tick_in);
		size_t i;

		if (samplesleft > 0)
		{
			for (i = 0; i < io->NumChips; ++i)
			{
				io->chips[i]->UpdateI(samples1, samplesleft);
			}
			//OffsetSamples(samples1, samplesleft << stereoshift);
			assert(NextTickIn == ticky);
			NextTickIn -= samplesleft;
			assert (NextTickIn >= 0);
			numsamples -= samplesleft;
			samples1 += samplesleft << stereoshift;
		}
		
		if (NextTickIn < 1)
		{
			int next = PlayTick();
			assert(next >= 0);
			if (next == 0)
			{ // end of song
				if (!Looping || prevEnded)
				{
					if (numsamples > 0)
					{
						for (i = 0; i < io->NumChips; ++i)
						{
							io->chips[i]->UpdateI(samples1, samplesleft);
						}
						//OffsetSamples(samples1, numsamples << stereoshift);
					}
					res = false;
					break;
				}
				else
				{
					// Avoid infinite loops from songs that do nothing but end
					prevEnded = true;
					Restart ();
				}
			}
			else
			{
				prevEnded = false;
				io->WriteDelay(next);
				NextTickIn += SamplesPerTick * next;
				assert (NextTickIn >= 0);
				MLtime += next;
			}
		}
	}
	return res;
}

void OPLmusicFile::OffsetSamples(float *buff, int count)
{
	// Three out of four of the OPL waveforms are non-negative. Depending on
	// timbre selection, this can cause the output waveform to tend toward
	// very large positive values. Heretic's music is particularly bad for
	// this. This function attempts to compensate by offseting the sample
	// data back to around the [-1.0, 1.0] range.

	double max = -1e10, min = 1e10, offset, step;
	int i, ramp, largest_at = 0;

	// Find max and min values for this segment of the waveform.
	for (i = 0; i < count; ++i)
	{
		if (buff[i] > max)
		{
			max = buff[i];
			largest_at = i;
		}
		if (buff[i] < min)
		{
			min = buff[i];
			largest_at = i;
		}
	}
	// Prefer to keep the offset at 0, even if it means a little clipping.
	if (LastOffset == 0 && min >= -1.1 && max <= 1.1)
	{
		offset = 0;
	}
	else
	{
		offset = (max + min) / 2;
		// If the new offset is close to 0, make it 0 to avoid making another
		// full loop through the sample data.
		if (fabs(offset) < 1/256.0)
		{
			offset = 0;
		}
	}
	// Ramp the offset change so there aren't any abrupt clicks in the output.
	// If the ramp is too short, it can sound scratchy. cblood2.mid is
	// particularly unforgiving of short ramps.
	if (count >= 512)
	{
		ramp = 512;
		step = (offset - LastOffset) / 512;
	}
	else
	{
		ramp = MIN(count, MAX(196, largest_at));
		step = (offset - LastOffset) / ramp;
	}
	offset = LastOffset;
	i = 0;
	if (step != 0)
	{
		for (; i < ramp; ++i)
		{
			buff[i] = float(buff[i] - offset);
			offset += step;
		}
	}
	if (offset != 0)
	{
		for (; i < count; ++i)
		{
			buff[i] = float(buff[i] - offset);
		}
	}
	LastOffset = float(offset);
}

int OPLmusicFile::PlayTick ()
{
	uint8_t reg, data;
	uint16_t delay;

	switch (RawPlayer)
	{
	case RDosPlay:
		while (score < scoredata + ScoreLen)
		{
			data = *score++;
			reg = *score++;
			switch (reg)
			{
			case 0:		// Delay
				if (data != 0)
				{
					return data;
				}
				break;

			case 2:		// Speed change or OPL3 switch
				if (data == 0)
				{
					SamplesPerTick = LittleShort(*(uint16_t *)(score)) / ADLIB_CLOCK_MUL;
					io->SetClockRate(SamplesPerTick);
					score += 2;
				}
				else if (data == 1)
				{
					WhichChip = 0;
				}
				else if (data == 2)
				{
					WhichChip = 1;
				}
				break;

			case 0xFF:	// End of song
				if (data == 0xFF)
				{
					return 0;
				}
				break;

			default:	// It's something to stuff into the OPL chip
				io->OPLwriteReg(WhichChip, reg, data);
				break;
			}
		}
		break;

	case DosBox1:
		while (score < scoredata + ScoreLen)
		{
			reg = *score++;

			if (reg == 4)
			{
				reg = *score++;
				data = *score++;
			}
			else if (reg == 0)
			{ // One-uint8_t delay
				return *score++ + 1;
			}
			else if (reg == 1)
			{ // Two-uint8_t delay
				int delay = score[0] + (score[1] << 8) + 1;
				score += 2;
				return delay;
			}
			else if (reg == 2)
			{ // Select OPL chip 0
				WhichChip = 0;
				continue;
			}
			else if (reg == 3)
			{ // Select OPL chip 1
				WhichChip = 1;
				continue;
			}
			else
			{
				data = *score++;
			}
			io->OPLwriteReg(WhichChip, reg, data);
		}
		break;

	case DosBox2:
		{
			uint8_t *to_reg = scoredata + 0x1A;
			uint8_t to_reg_size = scoredata[0x19];
			uint8_t short_delay_code = scoredata[0x17];
			uint8_t long_delay_code = scoredata[0x18];

			while (score < scoredata + ScoreLen)
			{
				uint8_t code = *score++;
				data = *score++;

				// Which OPL chip to write to is encoded in the high bit of the code value.
				int which = !!(code & 0x80);
				code &= 0x7F;

				if (code == short_delay_code)
				{
					return data + 1;
				}
				else if (code == long_delay_code)
				{
					return (data + 1) << 8;
				}
				else if (code < to_reg_size)
				{
					io->OPLwriteReg(which, to_reg[code], data);
				}
			}
		}
		break;

	case IMF:
		delay = 0;
		while (delay == 0 && score + 4 - scoredata <= ScoreLen)
		{
			if (*(uint32_t *)score == 0xFFFFFFFF)
			{ // This is a special value that means to end the song.
				return 0;
			}
			reg = score[0];
			data = score[1];
			delay = LittleShort(((uint16_t *)score)[1]);
			score += 4;
			io->OPLwriteReg (0, reg, data);
		}
		return delay;
	case AudioT:
		if (score < (scoredata + sizeof(oplsound_t) + ScoreLen))
		{
			if (!score[0])
				io->OPLwriteReg (0, 0xB1, 0);
			else
			{
				io->OPLwriteReg (0, 0xA0, score[0]);
				io->OPLwriteReg (0, 0xB0, (((Octave&7)<<2)|0x20));
			}
			++score;
		}
	}
	return 0;
}

bool OPLmusicFile::SetPosition(int pos)
{
/*	int oldpos = GetPosition();
	Restart();
	if (pos < ScoreLen)
	{
		score = scoredata + pos;
		return true;
	}
	score = scoredata + oldpos;*/
	return false;
}

OPLmusicFile::OPLmusicFile(const OPLmusicFile *source, const char *filename)
{
	ScoreLen = source->ScoreLen;
	scoredata = new uint8_t[ScoreLen];
	memcpy(scoredata, source->scoredata, ScoreLen);
	SamplesPerTick = source->SamplesPerTick;
	RawPlayer = source->RawPlayer;
	score = source->score;
	NumChips = source->NumChips;
	WhichChip = 0;
	Note = source->Note;
	Octave = source->Octave;
	if (io != NULL)
	{
		delete io;
	}
	io = new DiskWriterIO(filename);
	NumChips = io->OPLinit(NumChips);
	Restart();
}

void OPLmusicFile::Dump()
{
	int time;

	time = PlayTick();
	while (time != 0)
	{
		io->WriteDelay(time);
		time = PlayTick();
	}
}

//==========================================================================
//
// DiskWriterIO Constructor
//
//==========================================================================

DiskWriterIO::DiskWriterIO(const char *filename)
	: Filename(filename)
{
}

//==========================================================================
//
// DiskWriterIO Destructor
//
//==========================================================================

DiskWriterIO::~DiskWriterIO()
{
	OPLdeinit();
}

//==========================================================================
//
// DiskWriterIO :: OPLinit
//
//==========================================================================

int DiskWriterIO::OPLinit(uint32_t numchips, bool dontcare)
{
	// If the file extension is unknown or not present, the default format
	// is RAW. Otherwise, you can use DRO.
	if (Filename.Len() < 5 || Filename.EndsWith(".dro"))
	{
		Format = FMT_RDOS;
	}
	else
	{
		Format = FMT_DOSBOX;
	}
	File = fopen(CHR(Filename), "wb");
	if (File == NULL)
	{
		wxLogMessage("Could not open %s for writing.\n", CHR(Filename));
		return 0;
	}

	if (Format == FMT_RDOS)
	{
		fwrite("RAWADATA\0", 1, 10, File);
		NeedClockRate = true;
	}
	else
	{
		fwrite("DBRAWOPL"
			   "\0\0"		// Minor version number
			   "\1\0"		// Major version number
			   "\0\0\0\0"	// Total milliseconds
			   "\0\0\0",	// Total data
			   1, 20, File);
		if (numchips == 1)
		{
			fwrite("\0\0\0", 1, 4, File);	// Single OPL-2
		}
		else
		{
			fwrite("\2\0\0", 1, 4, File);	// Dual OPL-2
		}
		NeedClockRate = false;
	}

	TimePerTick = 0;
	TickMul = 1;
	CurTime = 0;
	CurIntTime = 0;
	CurChip = 0;
	OPLchannels = OPL2CHANNELS * numchips;
	OPLwriteInitState(false);
	return numchips;
}

//==========================================================================
//
// DiskWriterIO :: OPLdeinit
//
//==========================================================================

void DiskWriterIO::OPLdeinit()
{
	if (File != NULL)
	{
		if (Format == FMT_RDOS)
		{
			uint16_t endmark = 65535;
			fwrite(&endmark, 2, 1, File);
		}
		else
		{
			long where_am_i = ftell(File);
			uint32_t len[2];

			fseek(File, 12, SEEK_SET);
			len[0] = LittleLong(CurIntTime);
			len[1] = LittleLong(uint32_t(where_am_i - 24));
			fwrite(len, 4, 2, File);
		}
		fclose(File);
		File = NULL;
	}
}

//==========================================================================
//
// DiskWriterIO :: OPLwriteReg
//
//==========================================================================

void DiskWriterIO::OPLwriteReg(int which, uint32_t reg, uint8_t data)
{
	SetChip(which);
	if (Format == FMT_RDOS)
	{
		if (reg != 0 && reg != 2 && (reg != 255 || data != 255))
		{
			uint8_t cmd[2] = { data, uint8_t(reg) };
			fwrite(cmd, 1, 2, File);
		}
	}
	else
	{
		uint8_t cmd[3] = { 4, uint8_t(reg), data };
		fwrite (cmd + (reg > 4), 1, 3 - (reg > 4), File);
	}
}

//==========================================================================
//
// DiskWriterIO :: SetChip
//
//==========================================================================

void DiskWriterIO :: SetChip(int chipnum)
{
	assert(chipnum == 0 || chipnum == 1);

	if (chipnum != CurChip)
	{
		CurChip = chipnum;
		if (Format == FMT_RDOS)
		{
			uint8_t switcher[2] = { uint8_t(chipnum + 1), 2 };
			fwrite(switcher, 1, 2, File);
		}
		else
		{
			uint8_t switcher = chipnum + 2;
			fwrite(&switcher, 1, 1, File);
		}
	}
}

//==========================================================================
//
// DiskWriterIO :: SetClockRate
//
//==========================================================================

void DiskWriterIO::SetClockRate(double samples_per_tick)
{
	TimePerTick = samples_per_tick / OPL_SAMPLE_RATE * 1000.0;

	if (Format == FMT_RDOS)
	{
		double clock_rate;
		int clock_mul;
		uint16_t clock_word;

		clock_rate = samples_per_tick * ADLIB_CLOCK_MUL;
		clock_mul = 1;

		// The RDos raw format's clock rate is stored in a word. Therefore,
		// the longest tick that can be stored is only ~55 ms.
		while (clock_rate / clock_mul + 0.5 > 65535.0)
		{
			clock_mul++;
		}
		clock_word = uint16_t(clock_rate / clock_mul + 0.5);

		if (NeedClockRate)
		{ // Set the initial clock rate.
			clock_word = LittleShort(clock_word);
			fseek(File, 8, SEEK_SET);
			fwrite(&clock_word, 2, 1, File);
			fseek(File, 0, SEEK_END);
			NeedClockRate = false;
		}
		else
		{ // Change the clock rate in the middle of the song.
			uint8_t clock_change[4] = { 0, 2, uint8_t(clock_word & 255), uint8_t(clock_word >> 8) };
			fwrite(clock_change, 1, 4, File);
		}
	}
}

//==========================================================================
//
// DiskWriterIO :: WriteDelay
//
//==========================================================================

void DiskWriterIO :: WriteDelay(int ticks)
{
	if (ticks <= 0)
	{
		return;
	}
	if (Format == FMT_RDOS)
	{ // RDos raw has very precise delays but isn't very efficient at
	  // storing long delays.
		uint8_t delay[2];

		ticks *= TickMul;
		delay[1] = 0;
		while (ticks > 255)
		{
			ticks -= 255;
			delay[0] = 255;
			fwrite(delay, 1, 2, File);
		}
		delay[0] = uint8_t(ticks);
		fwrite(delay, 1, 2, File);
	}
	else
	{ // DosBox only has millisecond-precise delays.
		int delay;

		CurTime += TimePerTick * ticks;
		delay = int(CurTime + 0.5) - CurIntTime;
		CurIntTime += delay;
		while (delay > 65536)
		{
			uint8_t cmd[3] = { 1, 255, 255 };
			fwrite(cmd, 1, 2, File);
			delay -= 65536;
		}
		delay--;
		if (delay <= 255)
		{
			uint8_t cmd[2] = { 0, uint8_t(delay) };
			fwrite(cmd, 1, 2, File);
		}
		else
		{
			assert(delay <= 65535);
			uint8_t cmd[3] = { 1, uint8_t(delay & 255), uint8_t(delay >> 8) };
			fwrite(cmd, 1, 3, File);
		}
	}
}
