#pragma once

#define GATEWARE_ENABLE_MATH // Enables all Math Libraries

#include "structures.h"
#include "Gateware.h"
#include <array>

#define MAX_LINE_VERTS 20000

class DebugLinesManager
{
private:
	std::array<H2B::COLORED_VERTEX, MAX_LINE_VERTS> debugVerts;
	UINT debugVertCount;

public:
	DebugLinesManager();
	~DebugLinesManager();
	DebugLinesManager(const DebugLinesManager& copy);
	DebugLinesManager& operator=(const DebugLinesManager& copy);

	inline const UINT size() const { return debugVertCount; }
	inline const UINT capacity() const { return MAX_LINE_VERTS; }
	inline const H2B::COLORED_VERTEX* data() const { return debugVerts.data(); }
	inline const void clear() { debugVertCount = 0; }

	void AddLine(const H2B::COLORED_VERTEX& start, const H2B::COLORED_VERTEX& end);
	void AddLine(const GW::MATH::GVECTORF& start, const GW::MATH::GVECTORF& end, const GW::MATH::GVECTORF& startColor, const GW::MATH::GVECTORF& endColor);
	inline void AddLine(const GW::MATH::GVECTORF& start, const GW::MATH::GVECTORF& end, const GW::MATH::GVECTORF& color) { AddLine(start, end, color, color); }

};