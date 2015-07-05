#include "PaddedField.h"

namespace Cleaver
{


const float PaddedField::DefaultThickness = 2;
const float PaddedField::DefaultHighValue = +10000;
const float PaddedField::DefaultLowValue  = -10000;

PaddedField::PaddedField(const Cleaver::ScalarField *field, float thickness, float high, float low)
    : m_origi_field(field), m_thickness(thickness), m_highValue(high), m_lowValue(low)
{
    computeBounds();
}


void PaddedField::setField(const Cleaver::ScalarField *field)
{
    m_origi_field = field;
    computeBounds();
}

void PaddedField::setThickness(float thickness)
{
    m_thickness = thickness;
    computeBounds();
}

void PaddedField::setHighValue(float value)
{
    m_highValue = value;
}

void PaddedField::setLowValue(float value)
{
    m_lowValue = value;
}

void PaddedField::computeBounds()
{
    vec3 origin = m_origi_field->bounds().origin - vec3(m_thickness,m_thickness,m_thickness);
    vec3 size = m_origi_field->bounds().size + vec3(2*m_thickness,2*m_thickness,2*m_thickness);
    m_bounds = BoundingBox(origin, size);
}

}
