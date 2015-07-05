#include "MyFunction.h"

using namespace Cleaver;

MyFunction::MyFunction() : m_bounds(vec3::zero, vec3(30,30,30))
{
    // no allocation
}

MyFunction::~MyFunction()
{
    // no memory cleanup
}

BoundingBox MyFunction::bounds() const
{
    return m_bounds;
}

float MyFunction::valueAt(float x, float y, float z) const
{
    int dim_x = m_bounds.size.x;
    int dim_y = m_bounds.size.y;
    int dim_z = m_bounds.size.z;

    float r = 0.45*dim_x;
    float val = ((x-dim_x/2)*(x-dim_x/2))+((y-dim_y/2)*(y-dim_y/2))+(z-dim_z/2)*(z-dim_z/2) - r*r;
    return -val;
}

