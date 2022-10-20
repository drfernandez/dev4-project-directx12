#include "Frustum.h"

Frustum::Frustum()
{
	this->corners = { 0 };
	this->planes = { 0 };
	this->attachMatrix = GW::MATH::GIdentityMatrixF;
}

Frustum::Frustum(const GW::MATH::GMATRIXF& matrix)
{
	this->corners = { 0 };
	this->planes = { 0 };
	this->attachMatrix = matrix;
}

Frustum::~Frustum()
{
	this->corners = { 0 };
	this->planes = { 0 };
	this->attachMatrix = GW::MATH::GIdentityMatrixF;
}

Frustum::Frustum(const Frustum& copy)
{
	*this = copy;
}

Frustum& Frustum::operator=(const Frustum& copy)
{
	if (this != &copy)
	{
		this->corners = copy.corners;
		this->planes = copy.planes;
		this->attachMatrix = copy.attachMatrix;
	}
	return *this;
}

GW::MATH::GPLANEF Frustum::CalculatePlane(GW::MATH::GVECTORF a, GW::MATH::GVECTORF b, GW::MATH::GVECTORF c)
{	
	GW::MATH::GVECTORF normal = { 0 };
	GW::MATH::GVECTORF ba = { 0 };
	GW::MATH::GVECTORF ca = { 0 };
	float distance = 0.0f;
	GW::MATH::GVector::SubtractVectorF(b, a, ba);
	GW::MATH::GVector::SubtractVectorF(c, a, ca);
	GW::MATH::GVector::CrossVector3F(ba, ca, normal);
	GW::MATH::GVector::NormalizeF(normal, normal);
	GW::MATH::GVector::DotF(a, normal, distance);

	GW::MATH::GPLANEF plane = { 0 };
	plane.data = normal;
	plane.distance = distance;
	return plane;
}

int Frustum::ClassifySphereToPlane(const GW::MATH::GSPHEREF& sphere, const GW::MATH::GPLANEF& plane)
{
	int result = 0;
	float distance = 0.0f;
	GW::MATH::GVECTORF sc = { sphere.data.x, sphere.data.y, sphere.data.z, 0 };
	GW::MATH::GVECTORF pn = { plane.data.x, plane.data.y, plane.data.z, 0 };
	GW::MATH::GVector::DotF(sc, pn, distance);
	
	if ((distance - plane.distance) > sphere.radius)
	{
		result = 1;
	}
	else if ((distance - plane.distance) < -sphere.radius)
	{
		result = -1;
	}
	return result;
}

int Frustum::ClassifyAABBToPlane(const GW::MATH::GAABBMMF& aabb, const GW::MATH::GPLANEF& plane)
{
	GW::MATH::GVECTORF center = { 0 };
	GW::MATH::GVector::AddVectorF(aabb.max, aabb.min, center);
	GW::MATH::GVector::ScaleF(center, 0.5f, center);
	GW::MATH::GVECTORF extents = { 0 };
	GW::MATH::GVector::SubtractVectorF(aabb.max, center, extents);
	GW::MATH::GVECTORF absnormal = { fabs(plane.data.x), fabs(plane.data.y), fabs(plane.data.z), 0.0f };
	float radius = 0.0f;
	GW::MATH::GVector::DotF(absnormal, extents, radius);
	GW::MATH::GSPHEREF sphere = { center.x, center.y, center.z, radius };
	return ClassifySphereToPlane(sphere, plane);
}

void Frustum::Create(float fov, float ar, float nd, float fd, const GW::MATH::GMATRIXF& mat)
{
	attachMatrix = mat;

	float nearHeight = 2.0f * tanf(fov * 0.5f) * nd;
	float farHeight = 2.0f * tanf(fov * 0.5f) * fd;
	float nearWidth = nearHeight * ar;
	float farWidth = farHeight * ar;

	GW::MATH::GVECTORF nearCenter = { 0 };
	GW::MATH::GVECTORF farCenter = { 0 };
	GW::MATH::GVECTORF scaledDistance = { 0 };

	// nearCenter
	GW::MATH::GVector::ScaleF(attachMatrix.row3, nd, scaledDistance);
	GW::MATH::GVector::AddVectorF(attachMatrix.row4, scaledDistance, nearCenter);
	// farCenter
	GW::MATH::GVector::ScaleF(attachMatrix.row3, fd, scaledDistance);
	GW::MATH::GVector::AddVectorF(attachMatrix.row4, scaledDistance, farCenter);

	// Calculate the corners of the frustum (manual calculation)
	// NTL, NTR, NBL, NBR
	// NTL = nc - (attachMatrix.xaxis * (nearWidth * 0.5)) + (attachMatrix.yaxis * (nearHeight * 0.5));
	GW::MATH::GVector::ScaleF(attachMatrix.row1, nearWidth * 0.5, scaledDistance);
	GW::MATH::GVector::SubtractVectorF(nearCenter, scaledDistance, corners[static_cast<int>(CORNERS::NTL)]);
	GW::MATH::GVector::ScaleF(attachMatrix.row2, nearHeight * 0.5, scaledDistance);
	GW::MATH::GVector::AddVectorF(corners[static_cast<int>(CORNERS::NTL)], scaledDistance, corners[static_cast<int>(CORNERS::NTL)]);
	// NTR = NTL + (attachMatrix.xaxis * nearWidth)
	GW::MATH::GVector::ScaleF(attachMatrix.row1, nearWidth, scaledDistance);
	GW::MATH::GVector::AddVectorF(corners[static_cast<int>(CORNERS::NTL)], scaledDistance, corners[static_cast<int>(CORNERS::NTR)]);
	// NBR = NTR - (attachMatrix.yaxis * nearHeight)
	GW::MATH::GVector::ScaleF(attachMatrix.row2, nearHeight, scaledDistance);
	GW::MATH::GVector::SubtractVectorF(corners[static_cast<int>(CORNERS::NTR)], scaledDistance, corners[static_cast<int>(CORNERS::NBR)]);
	// NBL = NBR - (attachMatrix.xaxis * nearWidth)
	GW::MATH::GVector::ScaleF(attachMatrix.row1, nearWidth, scaledDistance);
	GW::MATH::GVector::SubtractVectorF(corners[static_cast<int>(CORNERS::NBR)], scaledDistance, corners[static_cast<int>(CORNERS::NBL)]);

	// FTL, FTR, FBL, FBR
	// FTL = fc - (attachMatrix.xaxis * (farWidth * 0.5)) + (attachMatrix.yaxis * (farHeight * 0.5));
	GW::MATH::GVector::ScaleF(attachMatrix.row1, farWidth * 0.5, scaledDistance);
	GW::MATH::GVector::SubtractVectorF(farCenter, scaledDistance, corners[static_cast<int>(CORNERS::FTL)]);
	GW::MATH::GVector::ScaleF(attachMatrix.row2, farHeight * 0.5, scaledDistance);
	GW::MATH::GVector::AddVectorF(corners[static_cast<int>(CORNERS::FTL)], scaledDistance, corners[static_cast<int>(CORNERS::FTL)]);
	// NTR = NTL + (attachMatrix.xaxis * farWidth)
	GW::MATH::GVector::ScaleF(attachMatrix.row1, farWidth, scaledDistance);
	GW::MATH::GVector::AddVectorF(corners[static_cast<int>(CORNERS::FTL)], scaledDistance, corners[static_cast<int>(CORNERS::FTR)]);
	// NBR = NTR - (attachMatrix.yaxis * farHeight)
	GW::MATH::GVector::ScaleF(attachMatrix.row2, farHeight, scaledDistance);
	GW::MATH::GVector::SubtractVectorF(corners[static_cast<int>(CORNERS::FTR)], scaledDistance, corners[static_cast<int>(CORNERS::FBR)]);
	// NBL = NBR - (attachMatrix.xaxis * farWidth)
	GW::MATH::GVector::ScaleF(attachMatrix.row1, farWidth, scaledDistance);
	GW::MATH::GVector::SubtractVectorF(corners[static_cast<int>(CORNERS::FBR)], scaledDistance, corners[static_cast<int>(CORNERS::FBL)]);

	// Calculate the planes for the frustum
	planes[static_cast<int>(PLANE_SIDES::BACK)]		= CalculatePlane(corners[static_cast<int>(CORNERS::FTL)], corners[static_cast<int>(CORNERS::FTR)], corners[static_cast<int>(CORNERS::FBR)]);
	planes[static_cast<int>(PLANE_SIDES::FRONT)]	= CalculatePlane(corners[static_cast<int>(CORNERS::NTR)], corners[static_cast<int>(CORNERS::NTL)], corners[static_cast<int>(CORNERS::NBL)]);
	planes[static_cast<int>(PLANE_SIDES::LEFT)]		= CalculatePlane(corners[static_cast<int>(CORNERS::NTL)], corners[static_cast<int>(CORNERS::FTL)], corners[static_cast<int>(CORNERS::NBL)]);
	planes[static_cast<int>(PLANE_SIDES::RIGHT)]	= CalculatePlane(corners[static_cast<int>(CORNERS::FTR)], corners[static_cast<int>(CORNERS::NTR)], corners[static_cast<int>(CORNERS::FBR)]);
	planes[static_cast<int>(PLANE_SIDES::TOP)]		= CalculatePlane(corners[static_cast<int>(CORNERS::NTL)], corners[static_cast<int>(CORNERS::NTR)], corners[static_cast<int>(CORNERS::FTL)]);
	planes[static_cast<int>(PLANE_SIDES::BOTTOM)]	= CalculatePlane(corners[static_cast<int>(CORNERS::NBL)], corners[static_cast<int>(CORNERS::FBL)], corners[static_cast<int>(CORNERS::FBR)]);

}

bool Frustum::CompareSphereToFrustum(const GW::MATH::GSPHEREF& sphere)
{
	bool result = true;
	for (size_t i = 0; i < planes.size(); i++)
	{
		if (ClassifySphereToPlane(sphere, planes[i]) == -1)
		{
			result = false;
			break;
		}
	}
	return result;
}

bool Frustum::CompareAABBToFrustum(const GW::MATH::GAABBMMF& aabb)
{
	bool result = true;
	for (size_t i = 0; i < planes.size(); i++)
	{
		if (ClassifyAABBToPlane(aabb, planes[i]) == -1)
		{
			result = false;
			break;
		}
	}
	return result;
}
