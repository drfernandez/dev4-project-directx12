#pragma once

#include "Gateware.h"
#include <array>

class Frustum
{
private:
	enum class CORNERS { NTL = 0, NTR, NBL, NBR, FTL, FTR, FBL, FBR, COUNT };
	enum class PLANE_SIDES { BACK = 0, FRONT, TOP, BOTTOM, LEFT, RIGHT, COUNT };

	GW::MATH::GPLANEF CalculatePlane(GW::MATH::GVECTORF a, GW::MATH::GVECTORF b, GW::MATH::GVECTORF c);
	int ClassifySphereToPlane(const GW::MATH::GSPHEREF& sphere, const GW::MATH::GPLANEF& plane);
	int ClassifyAABBToPlane(const GW::MATH::GAABBMMF& aabb, const GW::MATH::GPLANEF& plane);


public:
	Frustum();
	Frustum(GW::MATH::GMATRIXF matrix);
	~Frustum();
	Frustum(const Frustum& copy);
	Frustum& operator=(const Frustum& copy);

	GW::MATH::GMATRIXF attachMatrix;
	std::array<GW::MATH::GVECTORF, static_cast<size_t>(CORNERS::COUNT)> corners;
	std::array<GW::MATH::GPLANEF, static_cast<size_t>(PLANE_SIDES::COUNT)> planes;

	void CalculateFrustum(float fov, float ar, float nd, float fd, const GW::MATH::GMATRIXF& mat);
	bool CompareSphereToFrustum(const GW::MATH::GSPHEREF& sphere);
	bool CompareAABBToFrustum(const GW::MATH::GAABBMMF& aabb);
};

