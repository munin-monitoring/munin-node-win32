#pragma once
#include "../core/MuninNodePlugin.h"

class RegistryMuninNodePlugin : public MuninNodePluginHelper
{
public:
	/// \param sectionName The INI File section name for this plugin
	RegistryMuninNodePlugin(const std::string &sectionName);
	virtual ~RegistryMuninNodePlugin();

	virtual int GetConfig(char *buffer, int len);
	virtual int GetValues(char *buffer, int len);
	virtual bool IsLoaded() { return true; };

	static const char *SectionPrefix;

private:
	std::string m_SectionName;

	HKEY m_RootKey;
	std::string m_SubKey;
};