
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008-2012 Simon Judd
 *
 * Email:       sirjuddington@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    HogArchive.cpp
 * Description: HogArchive, archive class to handle HOG archives
 *              from Descent and Descent II.
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
#include "HogArchive.h"
#include "SplashWindow.h"
#include <wx/filename.h>


/*******************************************************************
 * EXTERNAL VARIABLES
 *******************************************************************/
EXTERN_CVAR(Bool, wad_force_uppercase)
EXTERN_CVAR(Bool, archive_load_data)

/*******************************************************************
 * HOGARCHIVE CLASS FUNCTIONS
 *******************************************************************/

/* HogArchive::HogArchive
 * HogArchive class constructor
 *******************************************************************/
HogArchive::HogArchive() : TreelessArchive(ARCHIVE_HOG)
{
}

/* HogArchive::~HogArchive
 * HogArchive class destructor
 *******************************************************************/
HogArchive::~HogArchive()
{
}

/* HogArchive::getEntryOffset
 * Returns the file byte offset for [entry]
 *******************************************************************/
uint32_t HogArchive::getEntryOffset(ArchiveEntry* entry)
{
	// Check entry
	if (!checkEntry(entry))
		return 0;

	return (uint32_t)(int)entry->exProp("Offset");
}

/* HogArchive::setEntryOffset
 * Sets the file byte offset for [entry]
 *******************************************************************/
void HogArchive::setEntryOffset(ArchiveEntry* entry, uint32_t offset)
{
	// Check entry
	if (!checkEntry(entry))
		return;

	entry->exProp("Offset") = (int)offset;
}

/* HogArchive::getFileExtensionString
 * Gets the wxWidgets file dialog filter string for the archive type
 *******************************************************************/
string HogArchive::getFileExtensionString()
{
	return "Hog Files (*.hog)|*.hog";
}

/* HogArchive::getFormat
 * Returns the EntryDataFormat id of this archive type
 *******************************************************************/
string HogArchive::getFormat()
{
	return "archive_hog";
}

/* HogArchive::open
 * Reads hog format data from a MemChunk
 * Returns true if successful, false otherwise
 *******************************************************************/
bool HogArchive::open(MemChunk& mc)
{
	// Check data was given
	if (!mc.hasData())
		return false;

	// Check size
	size_t archive_size = mc.getSize();
	if (archive_size < 3)
		return false;

	// Check magic header (DHF for "Descent Hog File")
	if (mc[0] != 'D' || mc[1] != 'H' || mc[2] != 'F')
		return false;

	// Stop announcements (don't want to be announcing modification due to entries being added etc)
	setMuted(true);

	// Iterate through files to see if the size seems okay
	theSplashWindow->setProgressMessage("Reading hog archive data");
	size_t iter_offset = 3;
	uint32_t num_lumps = 0;
	while (iter_offset < archive_size)
	{
		// Update splash window progress
		theSplashWindow->setProgress(((float)iter_offset / (float)archive_size));

		// If the lump data goes past the end of the file,
		// the hogfile is invalid
		if (iter_offset + 17 > archive_size)
		{
			wxLogMessage("HogArchive::open: hog archive is invalid or corrupt");
			Global::error = "Archive is invalid and/or corrupt";
			setMuted(false);
			return false;
		}

		// Setup variables
		num_lumps++;
		size_t offset = iter_offset + 17;
		size_t size = READ_L32(mc, iter_offset + 13);
		char name[14] = "";
		mc.seek(iter_offset, SEEK_SET);
		mc.read(name, 13);
		name[13] = 0;

		// Create & setup lump
		ArchiveEntry* nlump = new ArchiveEntry(wxString::FromAscii(name), size);
		nlump->setLoaded(false);
		nlump->exProp("Offset") = (int)offset;
		nlump->setState(0);

		// Add to entry list
		getRoot()->addEntry(nlump);

		// Update entry size to compute next offset
		iter_offset = offset + size;
	}

	// Detect all entry types
	MemChunk edata;
	theSplashWindow->setProgressMessage("Detecting entry types");
	for (size_t a = 0; a < numEntries(); a++)
	{
		// Update splash window progress
		theSplashWindow->setProgress((((float)a / (float)num_lumps)));

		// Get entry
		ArchiveEntry* entry = getEntry(a);

		// Read entry data if it isn't zero-sized
		if (entry->getSize() > 0)
		{
			// Read the entry data
			mc.exportMemChunk(edata, getEntryOffset(entry), entry->getSize());
			entry->importMemChunk(edata);
		}

		// Detect entry type
		EntryType::detectEntryType(entry);

		// Unload entry data if needed
		if (!archive_load_data)
			entry->unloadData();

		// Set entry to unchanged
		entry->setState(0);
	}

	// Setup variables
	setMuted(false);
	setModified(false);
	announce("opened");

	theSplashWindow->setProgressMessage("");

	return true;
}

/* HogArchive::write
 * Writes the hog archive to a MemChunk
 * Returns true if successful, false otherwise
 *******************************************************************/
bool HogArchive::write(MemChunk& mc, bool update)
{
	// Determine individual lump offsets
	uint32_t offset = 3;
	ArchiveEntry* entry = NULL;
	for (uint32_t l = 0; l < numEntries(); l++)
	{
		offset += 17;
		entry = getEntry(l);
		setEntryOffset(entry, offset);
		if (update)
		{
			entry->setState(0);
			entry->exProp("Offset") = (int)offset;
		}
		offset += entry->getSize();
	}

	// Clear/init MemChunk
	mc.clear();
	mc.seek(0, SEEK_SET);
	mc.reSize(offset);

	// Write the header
	char header[3] = { 'D', 'H', 'F' };
	mc.write(header, 3);

	// Write the lumps
	for (uint32_t l = 0; l < numEntries(); l++)
	{
		entry = getEntry(l);
		mc.write(entry->getData(), entry->getSize());
	}

	// Write the directory
	for (uint32_t l = 0; l < numEntries(); l++)
	{
		entry = getEntry(l);
		char name[13] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		long size = wxINT32_SWAP_ON_BE(entry->getSize());

		for (size_t c = 0; c < entry->getName().length() && c < 13; c++)
			name[c] = entry->getName()[c];

		mc.write(name, 13);
		mc.write(&size, 4);
		mc.write(entry->getData(), entry->getSize());
	}

	return true;
}

/* HogArchive::loadEntryData
 * Loads an entry's data from the hogfile
 * Returns true if successful, false otherwise
 *******************************************************************/
bool HogArchive::loadEntryData(ArchiveEntry* entry)
{
	// Check the entry is valid and part of this archive
	if (!checkEntry(entry))
		return false;

	// Do nothing if the lump's size is zero,
	// or if it has already been loaded
	if (entry->getSize() == 0 || entry->isLoaded())
	{
		entry->setLoaded();
		return true;
	}

	// Open hogfile
	wxFile file(filename);

	// Check if opening the file failed
	if (!file.IsOpened())
	{
		wxLogMessage("HogArchive::loadEntryData: Failed to open hogfile %s", filename.c_str());
		return false;
	}

	// Seek to lump offset in file and read it in
	file.Seek(getEntryOffset(entry), wxFromStart);
	entry->importFileStream(file, entry->getSize());

	// Set the lump to loaded
	entry->setLoaded();

	return true;
}

/* HogArchive::addEntry
 * Override of Archive::addEntry to force entry addition to the root
 * directory, update namespaces if needed and rename the entry if
 * necessary to be hog-friendly (12 characters max with extension)
 *******************************************************************/
ArchiveEntry* HogArchive::addEntry(ArchiveEntry* entry, unsigned position, ArchiveTreeNode* dir, bool copy)
{
	// Check entry
	if (!entry)
		return NULL;

	// Check if read-only
	if (isReadOnly())
		return NULL;

	// Copy if necessary
	if (copy)
		entry = new ArchiveEntry(*entry);

	// Process name (must be 12 characters max)
	string name = entry->getName().Truncate(12);
	if (wad_force_uppercase) name.MakeUpper();

	// Set new hog-friendly name
	entry->setName(name);

	// Do default entry addition (to root directory)
	Archive::addEntry(entry, position);

	return entry;
}

/* HogArchive::addEntry
 * Since hog files have no namespaces, just call the other function.
 *******************************************************************/
ArchiveEntry* HogArchive::addEntry(ArchiveEntry* entry, string add_namespace, bool copy)
{
	return addEntry(entry, 0xFFFFFFFF, NULL, copy);
}

/* HogArchive::renameEntry
 * Override of Archive::renameEntry to update namespaces if needed
 * and rename the entry if necessary to be hog-friendly (twelve
 * characters max)
 *******************************************************************/
bool HogArchive::renameEntry(ArchiveEntry* entry, string name)
{
	// Check entry
	if (!checkEntry(entry))
		return false;

	// Process name (must be 12 characters max)
	name.Truncate(12);
	if (wad_force_uppercase) name.MakeUpper();

	// Do default rename
	return Archive::renameEntry(entry, name);
}

/* HogArchive::isHogArchive
 * Checks if the given data is a valid Descent hog archive
 *******************************************************************/
bool HogArchive::isHogArchive(MemChunk& mc)
{
	// Check size
	size_t size = mc.getSize();
	if (size < 3)
		return false;

	// Check magic header
	if (mc[0] != 'D' || mc[1] != 'H' || mc[2] != 'F')
		return false;

	// Iterate through files to see if the size seems okay
	size_t offset = 3;
	while (offset < size)
	{
		// Enough room for the header?
		if (offset + 17 > size)
			return false;
		// Read entry size to compute next offset
		offset += 17 + READ_L32(mc, offset + 13);
	}

	// We should end on at exactly the end of the file
	return (offset == size);
}

/* HogArchive::isHogArchive
 * Checks if the file at [filename] is a valid Descent hog archive
 *******************************************************************/
bool HogArchive::isHogArchive(string filename)
{
	// Open file for reading
	wxFile file(filename);

	// Check it opened ok
	if (!file.IsOpened())
		return false;

	// Check size
	size_t size = file.Length();
	if (size < 3)
		return false;

	// Check magic header
	char magic[3] = "";
	file.Seek(0, wxFromStart);
	file.Read(magic, 3);
	if (magic[0] != 'D' || magic[1] != 'H' || magic[2] != 'F')
		return false;

	// Iterate through files to see if the size seems okay
	size_t offset = 3;
	while (offset < size)
	{
		// Enough room for the header?
		if (offset + 17 > size)
			return false;
		// Read entry size to compute next offset
		uint32_t lumpsize;
		file.Seek(offset + 13, wxFromStart);
		file.Read(&lumpsize, 4);
		offset += 17 + wxINT32_SWAP_ON_BE(lumpsize);
	}

	// We should end on at exactly the end of the file
	return (offset == size);
}

