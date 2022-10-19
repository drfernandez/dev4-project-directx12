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
	distance -= plane.distance;
	if (distance > sphere.radius)
	{
		result = 1;
	}
	else if (distance < -sphere.radius)
	{
		result = 2;
	}
	return result;
}

int Frustum::ClassifyAABBToPlane(const GW::MATH::GAABBMMF& aabb, const GW::MATH::GPLANEF& plane)
{
	GW::MATH::GVECTORF center = { 0 };
	GW::MATH::GVector::AddVectorF(aabb.max, aabb.min, center);
	GW::MATH::GVECTORF extents = { 0 };
	GW::MATH::GVector::SubtractVectorF(aabb.max, center, extents);
	GW::MATH::GVECTORF absnormal = { fabs(plane.data.x), fabs(plane.data.y), fabs(plane.data.z), 0.0f };
	float radius = 0.0f;
	GW::MATH::GVector::DotF(absnormal, extents, radius);
	GW::MATH::GSPHEREF sphere = { center.x, center.y, center.z, radius };
	return ClassifySphereToPlane(sphere, plane);
}

void Frustum::CalculateFrustum(float fov, float ar, float nd, float fd, const GW::MATH::GMATRIXF& mat)
{
	float nearWidth = 0.0f;
	float nearHeight = 0.0f;
	float farWidth = 0.0f;
	float farHeight = 0.0f;

	GW::MATH::GVECTORF nearCenter = { 0 };
	GW::MATH::GVECTORF farCenter = { 0 };

	GW::MATH::GVECTORF scaledDistance = { 0 };

	GW::MATH::GVector::ScaleF(mat.row3, nd, scaledDistance);
	GW::MATH::GVector::AddVectorF(mat.row4, scaledDistance, nearCenter);
	GW::MATH::GVector::ScaleF(mat.row3, fd, scaledDistance);
	GW::MATH::GVector::AddVectorF(mat.row4, scaledDistance, farCenter);

}

bool Frustum::CompareSphereToFrustum(const GW::MATH::GSPHEREF& sphere)
{
	bool result = false;
	return result;
}

bool Frustum::CompareAABBToFrustum(const GW::MATH::GAABBMMF& aabb)
{
	bool result = false;
	return result;
}
