
#ifndef __MAP_SPECIALS_H__
#define __MAP_SPECIALS_H__

class SLADEMap;
class ArchiveEntry;

namespace MapSpecials
{
	struct sector_colour_t
	{
		int		tag;
		rgba_t	colour;
	};

	void	processMapSpecials(SLADEMap* map);
	void	processLineSpecial(MapLine* line);

	bool	getTagColour(int tag, rgba_t* colour);
	bool	tagColoursSet();
	void	updateTaggedSectors(SLADEMap* map);

	// ZDoom
	void	processZDoomMapSpecials(SLADEMap* map);
	void	processZDoomLineSpecial(MapLine* line);
	void	setupPlaneAlignSlope(MapLine* line, bool floor, bool front);
	void	processACSScripts(ArchiveEntry* entry);
}

#endif//__MAP_SPECIALS_H__
