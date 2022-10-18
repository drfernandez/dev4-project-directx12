#include "Frustum.h"

Frustum::Frustum()
{
	this->corners = { 0 };
	this->planes = { 0 };
	this->attachMatrix = GW::MATH::GIdentityMatrixF;
}

Frustum::Frustum(GW::MATH::GMATRIXF matrix)
{
	attachMatrix = matrix;
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
	/*
	int result = 0;
	float distance = dot(sphere.center, plane.normal) - plane.offset;
	if (distance > sphere.radius)
	{
		result = 1;
	}
	else if (distance < -sphere.radius)
	{
		result = -1;
	}
	return result;
	*/
	int result = 0;
	float distance = 0.0f;
	//GW::MATH::GVector::DotF(sphere.d)
	return 0;
}

int Frustum::ClassifyAABBToPlane(const GW::MATH::GAABBMMF& aabb, const GW::MATH::GPLANEF& plane)
{
	return 0;
}

void Frustum::CalculateFrustum(float fov, float ar, float nd, float fd, const GW::MATH::GMATRIXF& mat)
{

}

bool Frustum::CompareSphereToFrustum(const GW::MATH::GSPHEREF& sphere)
{
	return false;
}

bool Frustum::CompareAABBToFrustum(const GW::MATH::GAABBMMF& aabb)
{
	return false;
}
