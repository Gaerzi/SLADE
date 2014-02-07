
#ifndef __AUDIO_ENTRY_PANEL_H__
#define __AUDIO_ENTRY_PANEL_H__

#include "EntryPanel.h"
#include <wx/mediactrl.h>

#undef Status
#include <SFML/Audio.hpp>
#ifndef NOLIBMODPLUG
#include "sfMod/sfMod.h"
#endif
#include "GMEPlayer.h"

class AudioEntryPanel : public EntryPanel
{
private:
	string	prevfile;
	int		audio_type;
	int		num_tracks;
	int		subsong;
	bool	opened;

	wxBitmapButton*	btn_play;
	wxBitmapButton*	btn_pause;
	wxBitmapButton*	btn_stop;
	wxBitmapButton*	btn_next;
	wxBitmapButton*	btn_prev;
	wxSlider*		slider_seek;
	wxSlider*		slider_volume;
	wxTimer*		timer_seek;
	wxMediaCtrl*	media_ctrl;
	wxStaticText*	txt_title;
	wxStaticText*	txt_track;

	sf::SoundBuffer*	sound_buffer;
	sf::Sound			sound;
	sf::Music			music;
#ifndef NOLIBMODPLUG
	sfmod::Mod			mod;
#endif

	enum
	{
		AUTYPE_INVALID,
		AUTYPE_SOUND,
		AUTYPE_MUSIC,
		AUTYPE_MIDI,
		AUTYPE_MEDIA,
		AUTYPE_MOD,
		AUTYPE_EMU,
	};

public:
	AudioEntryPanel(wxWindow* parent);
	~AudioEntryPanel();

	bool	loadEntry(ArchiveEntry* entry);
	bool	saveEntry();
	void	setAudioDuration(int duration);

	bool	open();
	bool	openAudio(MemChunk& audio, string filename);
	bool	openMidi(MemChunk& data, string filename);
	bool	openMod(MemChunk& data);
	bool	openEmu(MemChunk& data);
	bool	openMedia(string filename);
	void	startStream();
	void	stopStream();
	void	resetStream();

	// Events
	void	onBtnPlay(wxCommandEvent& e);
	void	onBtnPause(wxCommandEvent& e);
	void	onBtnStop(wxCommandEvent& e);
	void	onBtnPrev(wxCommandEvent& e);
	void	onBtnNext(wxCommandEvent& e);
	void	onTimer(wxTimerEvent& e);
	void	onSliderSeekChanged(wxCommandEvent& e);
	void	onSliderVolumeChanged(wxCommandEvent& e);
};

#endif//__AUDIO_ENTRY_PANEL_H__
