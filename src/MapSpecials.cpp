
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008-2014 Simon Judd
 *
 * Email:       sirjuddington@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    MapSpecials.cpp
 * Description: Various functions for processing map specials and
 *              scripts, mostly for visual effects (transparency,
 *              colours, slopes, etc.)
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
#include "SLADEMap.h"
#include "MapSpecials.h"
#include "GameConfiguration.h"
#include "Tokenizer.h"
#include <wx/colour.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
namespace MapSpecials
{
	vector<sector_colour_t> sector_colours;
}


/*******************************************************************
 * MAPSPECIALS NAMESPACE FUNCTIONS
 *******************************************************************/

/* MapSpecials::processMapSpecials
 * Process map specials, depending on the current game/port
 *******************************************************************/
void MapSpecials::processMapSpecials(SLADEMap* map)
{
	// ZDoom
	if (theGameConfiguration->currentPort() == "zdoom")
		processZDoomMapSpecials(map);
}

/* MapSpecials::applySectorColours
 * Apply sector colours as parsed from scripts
 *******************************************************************/
void MapSpecials::applySectorColours(SLADEMap* map)
{
	for (unsigned a = 0; a < sector_colours.size(); a++)
	{
		vector<MapSector*> tagged;
		map->getSectorsByTag(sector_colours[a].tag, tagged);
		wxColour wxcol(sector_colours[a].colour.r, sector_colours[a].colour.g, sector_colours[a].colour.b, 255);
		uint32_t col_int = wxcol.GetRGBA();

		for (unsigned s = 0; s < tagged.size(); s++)
			tagged[s]->setIntProperty("lightcolor", col_int);
	}
}

/* MapSpecials::processZDoomMapSpecials
 * Process ZDoom map specials, mostly to convert hexen specials to
 * UDMF counterparts
 *******************************************************************/
void MapSpecials::processZDoomMapSpecials(SLADEMap* map)
{
	// Line specials
	for (unsigned a = 0; a < map->nLines(); a++)
	{
		// Get special
		int special = map->getLine(a)->getSpecial();
		if (special == 0)
			continue;

		// Get args
		int args[5];
		for (unsigned arg = 0; arg < 5; arg++)
			args[arg] = map->getLine(a)->intProperty(S_FMT("arg%d", arg));

		// --- TranslucentLine ---
		if (special == 208)
		{
			// Get tagged lines
			vector<MapLine*> tagged;
			if (args[0] > 0)
				map->getLinesById(args[0], tagged);
			else
				tagged.push_back(map->getLine(a));

			// Get args
			double alpha = (double)args[1] / 255.0;
			string type = (args[2] == 0) ? "translucent" : "add";

			// Set transparency
			for (unsigned l = 0; l < tagged.size(); l++)
			{
				tagged[l]->setFloatProperty("alpha", alpha);
				tagged[l]->setStringProperty("renderstyle", type);

				LOG_MESSAGE(3, S_FMT("Line %d translucent: (%d) %1.2f, %s", tagged[l]->getIndex(), args[1], alpha, CHR(type)));
			}
		}
	}
}

/* MapSpecials::processACSScripts
 * Process 'OPEN' ACS scripts for various specials - sector colours,
 * slopes, etc.
 *******************************************************************/
void MapSpecials::processACSScripts(ArchiveEntry* entry)
{
	sector_colours.clear();

	Tokenizer tz;
	tz.setSpecialCharacters(";,:|={}/()");
	tz.openMem(entry->getData(), entry->getSize(), "ACS Scripts");

	string token = tz.getToken();
	while (!(token.IsEmpty() && !tz.quotedString()))
	{
		if (S_CMPNOCASE(token, "script"))
		{
			LOG_MESSAGE(3, "script found");

			tz.skipToken();	// Skip script #
			tz.getToken(&token);

			// Check for open script
			if (S_CMPNOCASE(token, "OPEN"))
			{
				LOG_MESSAGE(3, "script is OPEN");

				// Skip to opening brace
				while (token != "{")
					tz.getToken(&token);

				// Parse script
				tz.getToken(&token);
				while (token != "}")
				{
					// --- Sector_SetColor ---
					if (S_CMPNOCASE(token, "Sector_SetColor"))
					{
						// Get parameters
						vector<string> parameters;
						tz.getTokensUntil(parameters, ")");

						// Parse parameters
						long val;
						int tag = -1;
						int r = -1;
						int g = -1;
						int b = -1;
						for (unsigned a = 0; a < parameters.size(); a++)
						{
							if (parameters[a].ToLong(&val))
							{
								if (tag < 0)
									tag = val;
								else if (r < 0)
									r = val;
								else if (g < 0)
									g = val;
								else if (b < 0)
									b = val;
							}
						}

						// Check everything is set
						if (b < 0)
						{
							LOG_MESSAGE(2, "Invalid Sector_SetColor parameters");
						}
						else
						{
							sector_colour_t sc;
							sc.tag = tag;
							sc.colour.set(r, g, b, 255);
							LOG_MESSAGE(3, "Sector tag %d, colour %d,%d,%d", tag, r, g, b);
							sector_colours.push_back(sc);
						}
					}

					tz.getToken(&token);
				}
			}
		}

		tz.getToken(&token);
	}
}
