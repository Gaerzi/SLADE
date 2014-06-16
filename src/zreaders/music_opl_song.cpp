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

void OPLmusicFile::OPLwriteReg(int which, uint32_t reg, uint8_t data)
{
	reg |= (which & 1) << 8;
	which >>= 1;
	if (chips[which] != NULL)
	{
		chips[which]->WriteReg(reg, data);
	}
}

/*
* Write to an operator pair. To be used for register bases of 0x20, 0x40,
* 0x60, 0x80 and 0xE0.
*/
void OPLmusicFile::OPLwriteChannel(uint32_t regbase, uint32_t channel, uint8_t data1, uint8_t data2)
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
void OPLmusicFile::OPLwriteValue(uint32_t regbase, uint32_t channel, uint8_t value)
{
	uint32_t which = channel / OPL2CHANNELS;
	uint32_t reg = regbase + (channel % OPL2CHANNELS);
	OPLwriteReg (which, reg, value);
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

void OPLmusicFile::OPLwriteInstrument(uint32_t channel, audiot_inst_t *instr)
{
	OPLwriteChannel(0x20, channel, instr->mChar,    instr->cChar);
	OPLwriteChannel(0x40, channel, instr->mScale,   instr->cScale);
	OPLwriteChannel(0x60, channel, instr->mAttack,  instr->cAttack);
	OPLwriteChannel(0x80, channel, instr->mSus,     instr->cSus);
	OPLwriteChannel(0xE0, channel, instr->mWave,    instr->cWave);
	OPLwriteValue  (0xC0, channel, 0x30);
}

void OPLmusicFile::OPLwriteInstrument(uint32_t channel, genmidi_inst_t *instr)
{
	OPLwriteChannel(0x20, channel, instr->mChar,    instr->cChar);
	OPLwriteChannel(0x40, channel, instr->mScale,   instr->cScale);
	OPLwriteChannel(0x60, channel, instr->mAttack,  instr->cAttack);
	OPLwriteChannel(0x80, channel, instr->mSus,     instr->cSus);
	OPLwriteChannel(0xE0, channel, instr->mWave,    instr->cWave);
	OPLwriteValue  (0xC0, channel, instr->nConn | 0x30);
}

/*
* Stop all sounds
*/
void OPLmusicFile::OPLshutup(void)
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
int OPLmusicFile::OPLinit()
{
	uint32_t i;

	memset(chips, 0, sizeof(chips));
	for (i = 0; i < MAXOPL2CHIPS; ++i)
	{
		OPLEmul *chip = JavaOPLCreate();
		if (chip == NULL)
		{
			break;
		}
		chips[i] = chip;
	}
	OPLchannels = i * OPL3CHANNELS;
	OPLwriteInitState();
	return i;
}

void OPLmusicFile::OPLwriteInitState()
{
	for (uint32_t i = 0; i < MAXOPL2CHIPS; ++i)
	{
		int chip = i << 1;
		OPLwriteReg(chip, 0x105, 0x01);	// enable YMF262/OPL3 mode
		OPLwriteReg(chip, 0x104, 0x00);	// disable 4-operator mode
		OPLwriteReg(chip, 0x01, 0x20);	// enable Waveform Select
		OPLwriteReg(chip, 0x0B, 0x40);	// turn off CSW mode
		OPLwriteReg(chip, 0xBD, 0x00);	// set vibrato/tremolo depth to low, set melodic mode
	}
	OPLshutup();
}

/*
* Deinitialize hardware before shutdown
*/
void OPLmusicFile::OPLdeinit(void)
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

int OPLmusicFile::releaseChannel(uint32_t slot, uint32_t killed)
{
	OPLwriteChannel(0x80, slot, 0x0F, 0x0F);  // release rate - fastest
	OPLwriteChannel(0x40, slot, 0x3F, 0x3F);  // no volume
	return slot;
}

void OPLmusicFile::OPLstopMusic()
{
	uint32_t i;
	for(i = 0; i < OPLchannels; i++)
		releaseChannel(i, 1);
}

OPLmusicFile::OPLmusicFile (FILE *file, const uint8_t *musiccache, size_t len)
{
	memset (this, 0, sizeof(*this));
	scoredata = NULL;
	NextTickIn = 0;
	LastOffset = 0;
	NumChips = MIN(*opl_numchips, 2);
	ScoreLen = len;
	ImfRate = 700;

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

	if (0 == (NumChips = OPLinit()))
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
		uint16_t v1 = READ_L16(scoredata, 8);
		uint16_t v2 = READ_L16(scoredata, 10);
		if ((v1 == 0 || v1 > 1000) && v2 == 1)
		{
			RawPlayer = DosBox1;
			SamplesPerTick = OPL_SAMPLE_RATE / 1000;
			ScoreLen = MIN(len - 24, (unsigned)READ_L32(scoredata, 16)) + 24;
		}
		else if (v1 == 2 && v2 == 0)
		{
			bool okay = true;
			if (scoredata[21] != 0)
			{
				wxLogMessage("Unsupported DOSBox Raw OPL format %d", scoredata[20]);
				okay = false;
			}
			if (scoredata[22] != 0)
			{
				wxLogMessage("Unsupported DOSBox Raw OPL compression %d", scoredata[21]);
				okay = false;
			}
			if (!okay)
				goto fail;
			RawPlayer = DosBox2;
			SamplesPerTick = OPL_SAMPLE_RATE / 1000;
			int headersize = 0x1A + scoredata[0x19];
			ScoreLen = MIN(len - headersize, (unsigned)READ_L32(scoredata, 12) * 2) + headersize;
		}
		else
		{
			wxLogMessage("Unsupported DOSBox Raw OPL version %d.%d", v1, v2);
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
		SamplesPerTick = OPL_SAMPLE_RATE / ImfRate;

		score = scoredata + 6;
		// Skip track and game name
		for (int i = 2; i != 0; --i)
		{
			while (score < max && *score++ != '\0') {}
		}
		score++;	// Skip unknown uint8_t
		if (score + 8 > max)
		{ // Not enough room left for song data
			delete[] scoredata;
			scoredata = NULL;
			return;
		}
		songlen = READ_L32(score, 0);
		score += 4;
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
		OPLwriteInstrument(0, &sound->inst);
		ScoreLen = len;
		score = scoredata + sizeof(oplsound_t);
	}
	else
	{
		LOG_MESSAGE(2, "Unknown or unidentified OPL format");
		goto fail;
	}

	// This is called by the OPL player instead
	//Restart ();
}

OPLmusicFile::~OPLmusicFile ()
{
	if (scoredata != NULL)
	{
		OPLdeinit ();
		delete[] scoredata;
		scoredata = NULL;
	}
}

void OPLmusicFile::Restart ()
{
	if (scoredata == NULL)
	{
		wxLogMessage("Data was not loaded!");
		return;
	}
	OPLstopMusic ();
	MLtime = 0;
	LastOffset = 0;
	WhichChip = 0;
	switch (RawPlayer)
	{
	case RDosPlay:
		score = scoredata + 10;
		SamplesPerTick = READ_L16(scoredata, 8) / ADLIB_CLOCK_MUL;
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
		SamplesPerTick = OPL_SAMPLE_RATE / ImfRate;
		// Skip track and game name
		for (int i = 2; i != 0; --i)
		{
			while (*score++ != '\0') {}
		}
		score += 5;	// Skip unknown uint8_t and song length
		break;

	case AudioT:
		SamplesPerTick = OPL_SAMPLE_RATE / 140;
		score = scoredata + sizeof(oplsound_t);
		oplsound_t * sound = (oplsound_t*)scoredata;
		Octave = sound->octave;
		OPLwriteInstrument(0, &sound->inst);
		break;
	}
}

bool OPLmusicFile::ServiceStream (void *buff, int numbytes)
{
	float *samples1 = (float *)buff;
	int stereoshift = 1;
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
			for (i = 0; i < MAXOPL2CHIPS; ++i)
			{
				chips[i]->Update(samples1, samplesleft);
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
				if (numsamples > 0)
				{
					for (i = 0; i < MAXOPL2CHIPS; ++i)
					{
						chips[i]->Update(samples1, numsamples);
					}
					OffsetSamples(samples1, numsamples << stereoshift);
				}
				res = false;
				break;
			}
			else
			{
				prevEnded = false;
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
	uint8_t * ScoreEnd = scoredata + ScoreLen;

	switch (RawPlayer)
	{
	case RDosPlay:
		while (score < ScoreEnd)
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
				OPLwriteReg(WhichChip, reg, data);
				break;
			}
		}
		break;

	case DosBox1:
		while (score < ScoreEnd)
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
				int delay = READ_L16(score, 0) + 1;
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
			OPLwriteReg(WhichChip, reg, data);
		}
		break;

	case DosBox2:
		{
			uint8_t *to_reg = scoredata + 0x1A;
			uint8_t to_reg_size = scoredata[0x19];
			uint8_t short_delay_code = scoredata[0x17];
			uint8_t long_delay_code = scoredata[0x18];

			while (score < ScoreEnd)
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
					OPLwriteReg(which, to_reg[code], data);
				}
			}
		}
		break;

	case IMF:
		delay = 0;
		while (delay == 0 && score + 4 <= ScoreEnd)
		{
			if (*(uint32_t *)score == 0xFFFFFFFF)
			{ // This is a special value that means to end the song.
				return 0;
			}
			reg = score[0];
			data = score[1];
			delay = READ_L16(score, 2);
			score += 4;
			OPLwriteReg (0, reg, data);
		}
		return delay;
	case AudioT:
		if (score < ScoreEnd - 1)
		{
			uint8_t block = (Octave&7)<<2;
			if (!score[0])
				OPLwriteReg (0, 0xB0, block);
			else
			{
				OPLwriteReg (0, 0xA0, score[0]);
				OPLwriteReg (0, 0xB0, (block|0x20));
			}
			++score;
			return 1;
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
