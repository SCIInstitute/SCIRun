"""
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
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
"""


import matlab.engine

#input is in the form of x= matlabfunc(alpha);
def convertMatlabBlock(input):
    """ output_str is the template code. by split commands I extracted the name of input data and replace their name in output_str and then this function read every line of output_str and convert them to python code and execute it and in the end it will return the output."""
    x= str(input)
    t= x.split("=")
    out = t[0].split(",")
    num_out=len(out)
    out_scaled=""
    out_scaled=out_scaled+out[0]
    for i in range(num_out-1):
        out_scaled=out_scaled+", "+out[i+1]
    #out_scaled=out_scaled+['out_scaled'+str(i+1) for i in range(num_out)]
    y= x[x.find("(")+1:x.find(")")]
    z= t[1].split("(")
    inputs= y.split(",")
    num_in=len(inputs)
    #inputt=zeros(len(num_in))
    fild=[]
    fild=fild+['field'+str(i+1) for i in range(num_in)]
    k=""+fild[0]
    for j in range(num_in-1):
        k=k+", "+fild[j+1]
    k= z[0]+"("+k+", "+"nargout=num_out"+")"
    #k=t[1].append("nargout=num_out")
    fieldOutput=[]
    fieldOutput=fieldOutput+['fieldOutput'+str(i+1) for i in range(num_out)]
    #fieldOutput=y
    str_def1='\nimport matlab.engine\neng = matlab.engine.start_matlab()'
    str_def2=num_in*['\nfild[j]=convertfieldtomatlab(INPUT[j])']
    str_def3='\nfield_scaled= eng.tri1(fild)'
    str_def4=num_out*['\nout[i]=convertfieldtopython(field_scaled[i])\nfieldOutput[i] = out[i]']


    counter=0
    while counter<num_in:
        str_def2[counter]= str_def2[counter].replace("fild[j]",str(fild[counter]))
        str_def2[counter]= str_def2[counter].replace("INPUT[j]",str(inputs[counter]))
        counter=counter+1
    counter=0
    output_str1= str_def1
    output_str2=""
    for j in range(num_in):
        output_str2= output_str2+str_def2[j]




#print("Field is:",output_str1)


    output_str3=str_def3
    output_str3= output_str3.replace("tri1(fild)",k)
    output_str3= output_str3.replace("num_out",str(num_out))
    output_str3= output_str3.replace("num_in",str(num_in))
    output_str3= output_str3.replace("field_scaled",str(out_scaled))
    counter=0
    while counter<num_out:
        str_def4[counter]= str_def4[counter].replace("field_scaled[i]",str(out[counter]))
        str_def4[counter]= str_def4[counter].replace("num_out",str(num_out))
        str_def4[counter]= str_def4[counter].replace("num_in",str(num_in))
        str_def4[counter]= str_def4[counter].replace("out[i]",str(out[counter]))
        str_def4[counter]= str_def4[counter].replace("fieldOutput[i]",str(fieldOutput[counter]))
        counter=counter+1
        output_str4=""
    for i in range(num_out):
        output_str4=output_str4+str_def4[i]


    output_str=output_str1+output_str2+output_str3+output_str4
#print("Z is:",out)
    return output_str
def convertfieldtomatlab(field):

    if not type(field) is dict:
        raise ValueError("input is not a field type")

    new_field={}
    for key in field:
        if isinstance(field[key], (list,)):
            new_field[key] = matlab.double(field[key])
        else:
            new_field[key] = field[key]

    return new_field

def convertfieldtopython(field):

    if not type(field) is dict:
        raise ValueError("input is not a field type")

    new_field={}

    for key in field:
        if isinstance(field[key], (list,tuple,matlab.double)):
            new_field[key] = converttolist(field[key])
        elif not hasattr(field[key], "__len__"):
            new_field[key] = [field[key]]
        else:
            new_field[key] = field[key]

    return new_field

def converttolist(m_doub):

    if not isinstance(m_doub, (list,tuple,matlab.double)):
        raise ValueError("input must be a matlab matrix of doubles")

    m_list = []
    for _ in range(m_doub.size[1]):
        m_list.append(m_doub._data[_*m_doub.size[0]:_*m_doub.size[0]+m_doub.size[0]].tolist())

    m_l = [[row[i] for row in m_list] for i in range(len(m_list[0]))]

    if len(m_l)==1 and isinstance(m_l[0], (list,)):
        m_l=m_l[0]

    return m_l
