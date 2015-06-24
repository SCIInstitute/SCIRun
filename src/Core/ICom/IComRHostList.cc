/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
   University of Utah.

   
   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/

// IComRHostList.cc

#include <Core/ICom/IComRHostList.h>

namespace SCIRun {

void IComRHostPattern::insert(std::string pattern)
{

    size_t starpos = 0;
    size_t numstar = 0;

    // Count the number of stars
    if (pattern[0] =='*') numstar++;
    while( (starpos = pattern.find('*',(starpos+1))) < pattern.size()) numstar++;
    std::vector<size_t> starposvec(numstar);
    
    numstar = 0;
    if (pattern[0] == '*') starposvec[numstar++] = 0;
    while( (starpos = pattern.find('*',(starpos+1))) < pattern.size()) starposvec[numstar++] = starpos;
    
    if (starposvec.size() == 0)
    {
        start_ = pattern;
        return;
    }
    
    if (starposvec[0] == 0)
    {
        start_ = "";
    }
    else
    {
        start_ = pattern.substr(0,starposvec[0]);
    }
    
    if (starposvec[starposvec.size()-1] == (pattern.size()-1))
    {
        end_ = "";
    }
    else
    {
        end_ = pattern.substr(starposvec[starposvec.size()-1]+1);
    }
    
    inbetween_.resize(starposvec.size()-1);
    for (size_t p = 0; p < (starposvec.size()-1); p++)
    {
        inbetween_[p] = pattern.substr(starposvec[p]+1,starposvec[p+1]);
    }
    return;
}


bool IComRHostPattern::compare(std::string host)
{
    if (start_.size() > 0) if (host.substr(0,start_.size()) != start_) return(false);
    if (end_.size() > 0) if (host.substr(host.size() - end_.size()) !=  end_) return(false);
    std::string rem = host.substr(start_.size(),host.size() - end_.size());
    size_t patloc;
    for (size_t p =0; p < inbetween_.size(); p++)
    {
        if ( (patloc = rem.find(inbetween_[p])) < rem.size())
        {
            rem = rem.substr(patloc+inbetween_[p].size());
        }
        else
        {
            return(false);
        }
    }
    return(true);
}



void IComRHostList::insert(std::string patternlist)
{
    // analyse comma/space separated list

    //size_t numpatterns;
    
    size_t patternstart = 0;
    size_t patternstop = 0;
    size_t strpos = 0;
    size_t numpat = 0;
    
    while (strpos < patternlist.size())
    {
        for (; strpos < patternlist.size(); strpos++) { if ((patternlist[strpos] != ' ')&&(patternlist[strpos] != '\t')&&(patternlist[strpos] != ',')) break;}
        patternstart = strpos;
        for (; strpos < patternlist.size(); strpos++) if ((patternlist[strpos] == ' ')||(patternlist[strpos] == '\t')||(patternlist[strpos] == ',')) break;
        patternstop = strpos;
        if (patternstop > (patternstart+1)) numpat++;
    }

    patterns_.resize(numpat);
    
    numpat = 0;
    while (strpos < patternlist.size())
    {
        for (; strpos < patternlist.size(); strpos++) { if ((patternlist[strpos] != ' ')&&(patternlist[strpos] != '\t')&&(patternlist[strpos] != ',')) break;}
        patternstart = strpos;
        for (; strpos < patternlist.size(); strpos++) if ((patternlist[strpos] == ' ')||(patternlist[strpos] == '\t')||(patternlist[strpos] == ',')) break;
        patternstop = strpos;
        patterns_[numpat++].insert(patternlist.substr(patternstart,patternstop));
    }

    return;
}

bool IComRHostList::compare(IComAddress &address)
{
    //We cannot allow invalid addresses to be used
    if (!address.isvalid()) return(false);
    
    // In case no patterns are defined assume that 
    if (patterns_.size() == 0) return(true);
    
    if (!address.isinternal())
    {
        for (int p=0; p < (int)patterns_.size(); p++)
        {
            if(patterns_[p].compare(address.getinetname())) return(true);
            if(patterns_[p].compare(address.getipname())) return(true); 
        }
    }
    else
    {
        // internal addresses are always allowed
        return(true);
    }    
    
    return(false);
}

}

