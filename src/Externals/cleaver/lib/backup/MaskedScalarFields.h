//=======================================================
//
// This class allows the values of the polymorphic
// ScalarFields objects to be modified. A flat cost
// of O(n) is used to store an empty pointer list for
// each sample value. An additonal O(m) array is
// stored for each modified sample value.
//
// Any modifications to the input field are stored
// in the mask as a float array.
//
// At query time, the mask is first checked. If the
// mask is empty, then the original field is queried.
//
//=======================================================


#ifndef MASKEDSCALARFIELDS_H
#define MASKEDSCALARFIELDS_H

#include <map>
#include <vector>
#include "vec3.h"
#include "ScalarField.h"

namespace Cleaver
{

class MaskedScalarFields
{
public:

    MaskedScalarFields(std::vector<ScalarField*> fields);
    ~MaskedScalarFields();

    float valueAt(const vec3 &x, int m) const;
    float valueAt(float x, float y, float z, int m) const;

    void setNewValueAt(float value, float x, float y, float z, int m);

    int width() const { return w; }
    int height() const { return h; }
    int depth() const { return d; }
    int count() const { return fields.size(); }

    int change_count;

private:
    float **p_mask;             // primal mask
    float **d_mask;             // dual   mask
    std::map<vec3, float> mask; // possible replacement

    std::vector<ScalarField*> fields;
    int w,h,d,size;
};

}

#endif // MASKEDSCALARFIELDS_H
