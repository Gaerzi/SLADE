
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008-2014 Simon Judd
 *
 * Email:       sirjuddington@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    NodeBuilders.cpp
 * Description: NodeBuilders namespace - functions for handling
 *              node builder definitions
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
#include "ArchiveManager.h"
#include "NodeBuilders.h"
#include "Parser.h"

/*******************************************************************
 * EXTERNAL VARIABLES
 *******************************************************************/
EXTERN_CVAR(String, nodebuilder_id)

/*******************************************************************
 * VARIABLES
 *******************************************************************/
namespace NodeBuilders
{
	vector<builder_t>	builders;
	builder_t			invalid;
	string				custom;
	vector<string>		builder_paths;
	vector<string>		builder_settings;
}


/*******************************************************************
 * NODEBUILDERS NAMESPACE FUNCTIONS
 *******************************************************************/

/* NodeBuilders::init
 * Loads all node builder definitions from the program resource
 *******************************************************************/
void NodeBuilders::init()
{
	// Init invalid builder
	invalid.id = "invalid";

	// Get nodebuilders configuration from slade.pk3
	Archive* archive = theArchiveManager->programResourceArchive();
	ArchiveEntry* config = archive->entryAtPath("config/nodebuilders.cfg");
	if (!config)
		return;

	// Parse it
	Parser parser;
	parser.parseText(config->getMCData(), "nodebuilders.cfg");

	// Get 'nodebuilders' block
	ParseTreeNode* root = (ParseTreeNode*)parser.parseTreeRoot()->getChild("nodebuilders");
	if (!root)
		return;

	// Go through child block
	for (unsigned a = 0; a < root->nChildren(); a++)
	{
		ParseTreeNode* n_builder = (ParseTreeNode*)root->getChild(a);

		// Parse builder block
		builder_t builder;
		builder.id = n_builder->getName();
		for (unsigned b = 0; b < n_builder->nChildren(); b++)
		{
			ParseTreeNode* node = (ParseTreeNode*)n_builder->getChild(b);

			// Option
			if (S_CMPNOCASE(node->getType(), "option"))
			{
				builder.options.push_back(node->getName());
				builder.option_desc.push_back(node->getStringValue());
			}

			// Builder name
			else if (S_CMPNOCASE(node->getName(), "name"))
				builder.name = node->getStringValue();

			// Builder command
			else if (S_CMPNOCASE(node->getName(), "command"))
				builder.command = node->getStringValue();

			// Builder executable
			else if (S_CMPNOCASE(node->getName(), "executable"))
				builder.exe = node->getStringValue();
		}
		builders.push_back(builder);
	}

	// Set builder paths
	for (unsigned a = 0; a < builder_paths.size(); a+=2)
		getBuilder(builder_paths[a]).path = builder_paths[a+1];

	// Set builder settings
	for (unsigned a = 0; a < builder_settings.size(); a+=2)
		getBuilder(builder_settings[a]).settings = builder_settings[a+1];
}

/* NodeBuilders::addBUilderPath
 * Adds [path] for [builder]
 *******************************************************************/
void NodeBuilders::addBuilderPath(string builder, string path)
{
	builder_paths.push_back(builder);
	builder_paths.push_back(path);
}

/* NodeBuilders::saveBuilderPaths
 * Writes builder paths to [file]
 *******************************************************************/
void NodeBuilders::saveBuilderPaths(wxFile& file)
{
	file.Write("nodebuilder_paths\n{\n");
	for (unsigned a = 0; a < builders.size(); a++)
		file.Write(S_FMT("\t%s \"%s\"\n", builders[a].id, builders[a].path));
	file.Write("}\n");
}

/* NodeBuilders::addBuilderSettings
 * Adds [settings] for [builder]
 *******************************************************************/
void NodeBuilders::addBuilderSettings(string builder, string settings)
{
	builder_settings.push_back(builder);
	builder_settings.push_back(settings);
}

/* NodeBuilders::saveBuilderSettings
 * Writes builder settings to [file]
 *******************************************************************/
void NodeBuilders::saveBuilderSettings(wxFile& file)
{
	file.Write("nodebuilder_settings\n{\n");
	for (unsigned a = 0; a < builders.size(); a++)
		file.Write(S_FMT("\t%s \"%s\"\n", builders[a].id, builders[a].settings));
	file.Write("}\n");
}

/* NodeBuilders::nNodeBuilders
 * Returns the number of node builders defined
 *******************************************************************/
unsigned NodeBuilders::nNodeBuilders()
{
	return builders.size();
}

/* NodeBuilders::getBuilder
 * Returns the node builder definition matching [id]
 *******************************************************************/
NodeBuilders::builder_t& NodeBuilders::getBuilder(string id)
{
	for (unsigned a = 0; a < builders.size(); a++)
	{
		if (builders[a].id == id)
			return builders[a];
	}

	return invalid;
}

/* NodeBuilders::getBuilder
 * Returns the node builder definition at [index]
 *******************************************************************/
NodeBuilders::builder_t& NodeBuilders::getBuilder(unsigned index)
{
	// Check index
	if (index >= builders.size())
		return invalid;

	return builders[index];
}

/* NodeBuilders::glvisCompatible()
 * Returns true if the current settings allow to use glvis
 *******************************************************************/
bool NodeBuilders::glvisCompatible()
{
	// First check that the glvis path is not empty
	builder_t builder = getBuilder("glvis");
	if (builder.path.IsEmpty())
		return false;

	// To use glvis, GL nodes need to have been built first. This can
	// be done with GLBSP, or depending on settings with ZDBSP.
	builder = getBuilder(nodebuilder_id);
	if (!builder.name.CmpNoCase("GLBSP") && builder.path.length())
		return true;
	else if (!builder.name.CmpNoCase("ZDBSP") && builder.path.length() && 
		builder.settings.Contains("--gl") && 
		!builder.settings.Contains("--extended") &&
		!builder.settings.Contains("--compress "))
		return true;

	return false;
}