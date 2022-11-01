#include "debuglinesmanager.h"

DebugLinesManager::DebugLinesManager()
{
	memset(debugVerts.data(), 0, sizeof(H2B::COLORED_VERTEX) * MAX_LINE_VERTS);
	debugVertCount = 0;
}

DebugLinesManager::~DebugLinesManager()
{
	memset(debugVerts.data(), 0, sizeof(H2B::COLORED_VERTEX) * MAX_LINE_VERTS);
	debugVertCount = 0;
}

DebugLinesManager::DebugLinesManager(const DebugLinesManager& copy)
{
	*this = copy;
}

DebugLinesManager& DebugLinesManager::operator=(const DebugLinesManager& copy)
{
	if (this != &copy)
	{
		this->debugVerts = copy.debugVerts;
		this->debugVertCount = copy.debugVertCount;
	}
	return *this;
}

void DebugLinesManager::AddLine(const H2B::COLORED_VERTEX& start, const H2B::COLORED_VERTEX& end)
{
	if (size() < capacity())
	{
		debugVerts[debugVertCount] = start; debugVertCount++;
		debugVerts[debugVertCount] = end; debugVertCount++;
	}
}

void DebugLinesManager::AddLine(const GW::MATH::GVECTORF& start, const GW::MATH::GVECTORF& end, const GW::MATH::GVECTORF& startColor, const GW::MATH::GVECTORF& endColor)
{
	H2B::COLORED_VERTEX s = { start, startColor };
	H2B::COLORED_VERTEX e = { end, endColor };

	AddLine(s, e);
}
