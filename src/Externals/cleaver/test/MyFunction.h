#ifndef MYFUNCTION_H
#define MYFUNCTION_H

#include <Cleaver/ScalarField.h>
#include <Cleaver/BoundingBox.h>


class MyFunction : public Cleaver::ScalarField
{
public:
    MyFunction();
    ~MyFunction();

    virtual float valueAt(float x, float y, float z) const;
    virtual Cleaver::BoundingBox bounds() const;

private:
    Cleaver::BoundingBox m_bounds;
};


#endif // MYFUNCTION_H
