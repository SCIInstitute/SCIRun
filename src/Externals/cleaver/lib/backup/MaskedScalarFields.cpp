#include "MaskedScalarFields.h"
#include <cstdlib>
#include <cstring>
#include <cmath>

namespace Cleaver
{

MaskedScalarFields::MaskedScalarFields(std::vector<ScalarField*> fields) : fields(fields)
{
    w = fields[0]->width();
    h = fields[0]->height();
    d = fields[0]->depth();
    size = w*h*d;
    p_mask = new float*[size];
    d_mask = new float*[size];
    memset(p_mask, 0, size * sizeof(float*));
    memset(d_mask, 0, size * sizeof(float*));

    change_count = 0;
}

MaskedScalarFields::~MaskedScalarFields()
{
    int index = 0;
    for(int k=0; k < d; k++)
    {
        for(int j=0; j < h; j++)
        {
            for(int i=0; i < w; i++)
            {
                // delete element change array
                if(p_mask[index])
                    delete [] p_mask[index];
                if(d_mask[index])
                    delete [] d_mask[index];
                index++;
            }
        }
    }

    // delete containing array
    delete [] p_mask;
    delete [] d_mask;

    // leave creator to destroy the fields themselves
}

float MaskedScalarFields::valueAt(const vec3 &x, int m) const
{
    return valueAt(x.x,x.y,x.z,m);
}

float MaskedScalarFields::valueAt(float x, float y, float z, int m) const
{  
    bool primal_grid = (fmod(x,1.0f) == 0 && fmod(y,1.0f) == 0 && fmod(z,1.0f) == 0);
    bool dual_grid = (fmod(x,0.5f) == 0 && fmod(y,0.5f) == 0 && fmod(z,0.5f) == 0);

    if(primal_grid)
    {
        int index = x + y*h + z*w*h;
        if(p_mask[index])
            return p_mask[index][m];
        else
            return fields[m]->valueAt(x,y,z);
    }
    else if(dual_grid)
    {
        int index = x + y*h + z*w*h;
        if(d_mask[index])
            return d_mask[index][m];
        else
            return fields[m]->valueAt(x,y,z);
    }
    else
        return fields[m]->valueAt(x,y,z);
}

void MaskedScalarFields::setNewValueAt(float value, float x, float y, float z, int m)
{
    bool primal_grid = (fmod(x,1.0f) == 0 && fmod(y,1.0f) == 0 && fmod(z,1.0f) == 0);
    bool dual_grid = (fmod(x,0.5f) == 0 && fmod(y,0.5f) == 0 && fmod(z,0.5f) == 0);

    if(primal_grid)
    {
        int index = x + y*h + z*w*h;
        if(!p_mask[index]){
            p_mask[index] = new float[m];
            change_count++;
        }

        // copy values
        for(unsigned int i=0; i < fields.size(); i++)
            p_mask[index][i] = fields[i]->valueAt(x,y,z);
        // change new value
        p_mask[index][m] = value;
    }
    else if(dual_grid)
    {
        int index = x + y*h + z*w*h;
        if(!d_mask[index]){
            d_mask[index] = new float[m];
            change_count++;
        }

        // copy values
        for(unsigned int i=0; i < fields.size(); i++)
            d_mask[index][i] = fields[i]->valueAt(x,y,z);
        // change new value
        d_mask[index][m] = value;
    }
    else
    {
        std::cerr << "DANGER: Masked field attempting to change non-lattice value" << std::endl;
        exit(0);
    }
}

}
