/*
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
*/


#include <stdio.h>
#include <math.h>
#include <Core/Math/fft.h>

#ifndef M_PI
#define M_PI 3.1415926535898
#endif

#ifndef M_PI
#define M_PI 3.1415926535898
#endif

void pdata(char* title, float* data, int n, int stride)
{
    int i;
    printf("%s:\t", title);
    stride*=2;
    for(i=0;i<n;i++){
	printf("%g %g ", data[0], data[1]);
	data+=stride;
    }
    printf("\n");
}

void pdata2(char* title, float* data, int n)
{
    int i,j;
    printf("%s:\n", title);
    for(i=0;i<n;i++){
	for(j=0;j<n;j++){
	    printf("%g %g ", data[0], data[1]);
	    data+=2;
	}
	printf("\n");
    }
    printf("\n");
}

void fft1d_stride_float(float* data, int n, int stride,
			float sign, unsigned long* flops,
			unsigned long* memrefs)
{
    int i, mask;
    int nn;
    int fn;
    /* Bit reverse */
    int ir=0;
    int n2=n/2;
    int nd;
    double* dd=(double*)data;
    for(i=0;i<n;i++){
	if(i>0){
	    /* Increment ir */
	    mask=n2;
	    while(ir >= mask){
		ir -= mask;
		mask>>=1;
	    }
	    ir+=mask;
	}
	if(i<ir){
	    /* Swap... */
#if 0
	    float tmp1=data[i*2];
	    float tmp2=data[i*2+1];
	    data[i*2]=data[ir*2];
	    data[i*2+1]=data[ir*2+1];
	    data[ir*2]=tmp1;
	    data[ir*2+1]=tmp2;
#endif
	    /* Swap... Trick - move both floats as a single double */
	    double tmp=dd[i*stride];
	    dd[i*stride]=dd[ir*stride];
	    dd[ir*stride]=tmp;
	}
    }
    *memrefs+=8*n/2*sizeof(float);

    /* N=2 stage */
    nd=2*n;
    for(i=0;i<nd;i+=4){
	float r1=data[i*stride];
	float i1=data[i*stride+1];
	float r2=data[i*stride+stride*2];
	float i2=data[i*stride+stride*2+1];
	float or1=r1+r2;
	float oi1=i1+i2;
	float or2=r1-r2;
	float oi2=i1-i2;
	data[i*stride]=or1;
	data[i*stride+1]=oi1;
	data[i*stride+stride*2]=or2;
	data[i*stride+stride*2+1]=oi2;
    }
    /*pdata("after n=2", data, n, 1);*/
    *memrefs+=nd*8*sizeof(float)/4;
    *flops+=nd*4/4;
    /* Rest... */
    nn=n/4;
    for(fn=4;fn<=n;fn*=2){
	int fn2=fn/2;
	double theta=sign*(2*M_PI/fn);
	float wnr=cos(theta);
	float wni=sin(theta);
	float wr=1;
	float wi=0;
	int ii;
	for(ii=0;ii<fn2;ii++){
	    if(nn<4){
		float* d1=data+ii*stride*2;
		float* d2=d1+fn*stride;
		int in;
		for(in=0;in<nn;in++){
		    float r2=d2[0];
		    float i2=d2[1];
		    /* Compute W*(r2,i2) */
		    float rp=wr*r2-wi*i2;
		    float ip=wr*i2+wi*r2;

		    float r1=d1[0];
		    float i1=d1[1];
		    /* Compute s1+W*s2 */
		    d1[0]=r1+rp;
		    d1[1]=i1+ip;
		    /* Compute s1-W*s2 */
		    d2[0]=r1-rp;
		    d2[1]=i1-ip;
		    d1+=fn*stride*2;
		    d2+=fn*stride*2;
		}
		*flops+=nn*10;
		*memrefs+=nn*8*sizeof(float);
	    } else {
		float* d1_0=data+ii*stride*2;
		float* d2_0=d1_0+fn*stride;
		float* d1_1=d2_0+fn*stride;
		float* d2_1=d1_1+fn*stride;
		float* d1_2=d2_1+fn*stride;
		float* d2_2=d1_2+fn*stride;
		float* d1_3=d2_2+fn*stride;
		float* d2_3=d1_3+fn*stride;
		int in;
		for(in=0;in<nn;in+=4){
		    float r2_0=d2_0[0];
		    float i2_0=d2_0[1];
		    float r2_1=d2_1[0];
		    float i2_1=d2_1[1];
		    float r2_2=d2_2[0];
		    float i2_2=d2_2[1];
		    float r2_3=d2_3[0];
		    float i2_3=d2_3[1];
		    /* Compute W*(r2,i2) */
		    float rp_0=wr*r2_0-wi*i2_0;
		    float ip_0=wr*i2_0+wi*r2_0;
		    float rp_1=wr*r2_1-wi*i2_1;
		    float ip_1=wr*i2_1+wi*r2_1;
		    float rp_2=wr*r2_2-wi*i2_2;
		    float ip_2=wr*i2_2+wi*r2_2;
		    float rp_3=wr*r2_3-wi*i2_3;
		    float ip_3=wr*i2_3+wi*r2_3;

		    float r1_0=d1_0[0];
		    float i1_0=d1_0[1];
		    float r1_1=d1_1[0];
		    float i1_1=d1_1[1];
		    float r1_2=d1_2[0];
		    float i1_2=d1_2[1];
		    float r1_3=d1_3[0];
		    float i1_3=d1_3[1];
		    /* Compute s1+W*s2 */
		    d1_0[0]=r1_0+rp_0;
		    d1_0[1]=i1_0+ip_0;
		    d1_1[0]=r1_1+rp_1;
		    d1_1[1]=i1_1+ip_1;
		    d1_2[0]=r1_2+rp_2;
		    d1_2[1]=i1_2+ip_2;
		    d1_3[0]=r1_3+rp_3;
		    d1_3[1]=i1_3+ip_3;
		    /* Compute s1-W*s2 */
		    d2_0[0]=r1_0-rp_0;
		    d2_0[1]=i1_0-ip_0;
		    d2_1[0]=r1_1-rp_1;
		    d2_1[1]=i1_1-ip_1;
		    d2_2[0]=r1_2-rp_2;
		    d2_2[1]=i1_2-ip_2;
		    d2_3[0]=r1_3-rp_3;
		    d2_3[1]=i1_3-ip_3;
		    d1_0+=fn*8*stride;
		    d2_0+=fn*8*stride;
		    d1_1+=fn*8*stride;
		    d2_1+=fn*8*stride;
		    d1_2+=fn*8*stride;
		    d2_2+=fn*8*stride;
		    d1_3+=fn*8*stride;
		    d2_3+=fn*8*stride;
		}
		*flops+=nn*10;
		*memrefs+=nn*8*sizeof(float);
	    }
	    {
		float new_wr=wnr*wr-wni*wi;
		float new_wi=wni*wr+wnr*wi;
		wr=new_wr;
		wi=new_wi;
		*flops+=6;
	    }
	}
	nn/=2;
	/*pdata("after n=2", data, n, 1);*/
    }
}

void fft1d_float(float* data, int n, float sign, unsigned long* flops,
		 unsigned long* memrefs)
{
    int i, mask;
    int nn;
    int fn;
    /* Bit reverse */
    int ir=0;
    int n2=n/2;
    int nd;
    double* dd=(double*)data;
    for(i=0;i<n;i++){
	if(i>0){
	    /* Increment ir */
	    mask=n2;
	    while(ir >= mask){
		ir -= mask;
		mask>>=1;
	    }
	    ir+=mask;
	}
	if(i<ir){
	    /* Swap... */
#if 0
	    float tmp1=data[i*2];
	    float tmp2=data[i*2+1];
	    data[i*2]=data[ir*2];
	    data[i*2+1]=data[ir*2+1];
	    data[ir*2]=tmp1;
	    data[ir*2+1]=tmp2;
#endif
	    /* Swap... Trick - move both floats as a single double */
	    double tmp=dd[i];
	    dd[i]=dd[ir];
	    dd[ir]=tmp;
	}
    }
    *memrefs+=8*n/2*sizeof(float);
    /*pdata("after swap", data, n, 1);*/

    /* N=2 stage */
    nd=2*n;
    for(i=0;i<nd;i+=4){
	float r1=data[i];
	float i1=data[i+1];
	float r2=data[i+2];
	float i2=data[i+3];
	float or1=r1+r2;
	float oi1=i1+i2;
	float or2=r1-r2;
	float oi2=i1-i2;
	data[i]=or1;
	data[i+1]=oi1;
	data[i+2]=or2;
	data[i+3]=oi2;
    }
    /*pdata("after n=2", data, n, 1);*/
    *memrefs+=nd*8*sizeof(float)/4;
    *flops+=nd*4/4;
    /* Rest... */
    nn=n/4;
    for(fn=4;fn<=n;fn*=2){
	int fn2=fn/2;
	double theta=sign*(2*M_PI/fn);
	float wnr=cos(theta);
	float wni=sin(theta);
	float wr=1;
	float wi=0;
	int ii;
	for(ii=0;ii<fn2;ii++){
	    if(nn<4){
		float* d1=data+ii*2;
		float* d2=d1+fn;
		int in;
		for(in=0;in<nn;in++){
		    float r2=d2[0];
		    float i2=d2[1];
		    /* Compute W*(r2,i2) */
		    float rp=wr*r2-wi*i2;
		    float ip=wr*i2+wi*r2;

		    float r1=d1[0];
		    float i1=d1[1];
		    /*printf("w=%g %g\n", wr, wi);*/
		    /* Compute s1+W*s2 */
		    d1[0]=r1+rp;
		    d1[1]=i1+ip;
		    /* Compute s1-W*s2 */
		    d2[0]=r1-rp;
		    d2[1]=i1-ip;
		    d1+=fn*2;
		    d2+=fn*2;
		}
		*flops+=nn*10;
		*memrefs+=nn*8*sizeof(float);
	    } else {
		float* d1_0=data+ii*2;
		float* d2_0=d1_0+fn;
		float* d1_1=d2_0+fn;
		float* d2_1=d1_1+fn;
		float* d1_2=d2_1+fn;
		float* d2_2=d1_2+fn;
		float* d1_3=d2_2+fn;
		float* d2_3=d1_3+fn;
		int in;
		for(in=0;in<nn;in+=4){
		    float r2_0=d2_0[0];
		    float i2_0=d2_0[1];
		    float r2_1=d2_1[0];
		    float i2_1=d2_1[1];
		    float r2_2=d2_2[0];
		    float i2_2=d2_2[1];
		    float r2_3=d2_3[0];
		    float i2_3=d2_3[1];
		    /* Compute W*(r2,i2) */
		    float rp_0=wr*r2_0-wi*i2_0;
		    float ip_0=wr*i2_0+wi*r2_0;
		    float rp_1=wr*r2_1-wi*i2_1;
		    float ip_1=wr*i2_1+wi*r2_1;
		    float rp_2=wr*r2_2-wi*i2_2;
		    float ip_2=wr*i2_2+wi*r2_2;
		    float rp_3=wr*r2_3-wi*i2_3;
		    float ip_3=wr*i2_3+wi*r2_3;

		    float r1_0=d1_0[0];
		    float i1_0=d1_0[1];
		    float r1_1=d1_1[0];
		    float i1_1=d1_1[1];
		    float r1_2=d1_2[0];
		    float i1_2=d1_2[1];
		    float r1_3=d1_3[0];
		    float i1_3=d1_3[1];
		    /* Compute s1+W*s2 */
		    d1_0[0]=r1_0+rp_0;
		    d1_0[1]=i1_0+ip_0;
		    d1_1[0]=r1_1+rp_1;
		    d1_1[1]=i1_1+ip_1;
		    d1_2[0]=r1_2+rp_2;
		    d1_2[1]=i1_2+ip_2;
		    d1_3[0]=r1_3+rp_3;
		    d1_3[1]=i1_3+ip_3;
		    /* Compute s1-W*s2 */
		    d2_0[0]=r1_0-rp_0;
		    d2_0[1]=i1_0-ip_0;
		    d2_1[0]=r1_1-rp_1;
		    d2_1[1]=i1_1-ip_1;
		    d2_2[0]=r1_2-rp_2;
		    d2_2[1]=i1_2-ip_2;
		    d2_3[0]=r1_3-rp_3;
		    d2_3[1]=i1_3-ip_3;
		    d1_0+=fn*8;
		    d2_0+=fn*8;
		    d1_1+=fn*8;
		    d2_1+=fn*8;
		    d1_2+=fn*8;
		    d2_2+=fn*8;
		    d1_3+=fn*8;
		    d2_3+=fn*8;
		}
		*flops+=nn*10;
		*memrefs+=nn*8*sizeof(float);
	    }
	    {
		float new_wr=wnr*wr-wni*wi;
		float new_wi=wni*wr+wnr*wi;
		wr=new_wr;
		wi=new_wi;
		*flops+=6;
	    }
	}
	nn/=2;
	/*pdata("after n=?", data, n, 1);*/
    }
}

void fft2d_float(float* data, int n, float sign, unsigned long* flops,
		 unsigned long* memrefs)
{
    int x, y;
    float* p=data;
    for(y=0;y<n;y++){
	fft1d_float(p, n, sign, flops, memrefs);
	p+=n*2;
    }
    p=data;
    for(x=0;x<n;x++){
	fft1d_stride_float(p, n, n, sign, flops, memrefs);
	p+=2;
    }
    /*pdata2("final", data, n);*/
}
