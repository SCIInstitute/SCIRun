/*
  Teem: Tools to process and visualize scientific data and images              
  Copyright (C) 2008, 2007, 2006, 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License
  (LGPL) as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  The terms of redistributing and/or modifying this software also
  include exceptions to the LGPL that facilitate static linking.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "ten.h"
#include "privateTen.h"


#define MAX_KMEANS_ITERATIONS 50




/* Calculate the Q-ball profile from DWIs */
void
_tenQball(const double b, const int gradcount, const double svals[],
          const double grads[], double qvals[] ) {
  /* Not an optimal Q-ball implementation! (Taken from submission to
     MICCAI 2006) Should be solved analytically in the future,
     implemented from recent papers. */
  int i,j;
  double d, dist, weight, min, max;
  
  AIR_UNUSED(b);
  min = max = svals[1] / svals[0];
  for( i = 0; i < gradcount; i++ ) {
    d = svals[i+1] / svals[0];
    if( d > max )
      max = d;
    else if( d < min )
      min = d;
  }
  
  for( i = 0; i < gradcount; i++ ) {
    qvals[i] = 0;
    for( j = 0; j < gradcount; j++ ) {
      d = AIR_AFFINE( min, svals[j+1] / svals[0], max, 0,1 );
      dist = ELL_3V_DOT(grads + 3*i, grads + 3*j);
      dist = AIR_ABS(dist);
      weight = cos( 0.5 * AIR_PI * dist );
      qvals[i] += d * weight*weight*weight*weight;
    }
  }
  return;
}

/* Segments DWIs into 2 segments based on Q-ball profiles */
void
_tenSegsamp2(const int gradcount, const double qvals[],
             const double grads[], const double qpoints[],
             unsigned int seg[], double dists[]) {
  const int segcount = 2;
  int i, changed=AIR_TRUE;
  double centroids[ 3*2 ]; /* 3*segcount */

  AIR_UNUSED(grads);
  _tenInitcent2(gradcount, qvals, qpoints, centroids);
  
  for( i = 0; i < MAX_KMEANS_ITERATIONS && changed; i++ ) {
    _tenCalcdists(segcount, centroids, gradcount, qpoints, dists);
    changed = _tenCalccent2(gradcount, qpoints, dists, centroids, seg);
    
    /*
      printf( "Seg[%d]\t= { ",i );
      for( j = 0; j < gradcount; j++ )
      printf( "%d ", seg[j] );
      printf( changed ? "}\n" : "} Convergence!\n" );
    */
    
  }
}

/* Gives an inital choice of 2 centroids */
void
_tenInitcent2(const int gradcount, const double qvals[],
              const double qpoints[], double centroids[6]) {
  int i, maxidx;
  double max, dist;
  
  /* Find largest peak in Q-ball */
  maxidx = 0;
  for( i = 0; i < gradcount; i++ )
    if( qvals[maxidx] < qvals[i] )
      maxidx = i;
  
  ELL_3V_COPY( centroids, qpoints +3*maxidx );
  /*
    printf("init: max=%d cent0=[%f %f %f]\n", maxidx,
    centroids[0], centroids[1], centroids[2]);
  */
  
  /* Find peak/axis from Q-ball furthest away from first peak */
  max = 0;
  for( i = 0; i < gradcount; i++ ) {
    dist = _tenPldist(qpoints +3*i, centroids);
    if (dist > max) {
      maxidx = i;
      max = dist;
    }
  }
  
  ELL_3V_COPY( centroids+3, qpoints +3*maxidx );
  /*
    printf( "\ninit: max=%d cent1=[%f %f %f]\n", maxidx, 
    centroids[3], centroids[4], centroids[5]);
  */
}

/* Calculates 2 new centroids (and a new segmentation) from distances
   between Q-balls and centroids, returns true if segmentation changed
*/
int
_tenCalccent2(const int gradcount, const double qpoints[],
              const double dists[], double centroid[6], unsigned int seg[]) {
#if 0
  /* HEY: Attempt to implement better line-adding by adding
     outerproducts of points and estimating major eigenvector
     afterwards */
  int i,changed=AIR_FALSE;
  double sum0[9],sum1[9],mat[9], eval[3],evec[9];
  
  ELL_3M_ZERO_SET( sum0 );
  ELL_3M_ZERO_SET( sum1 );
  for( i = 0; i < gradcount; i++ ) {
    if( dists[i] < dists[gradcount+i] ) {
      ELL_3MV_OUTER( mat, qpoints +3*i, qpoints +3*i );
      ELL_3M_ADD2( sum0, sum0, mat );
      changed = changed || (seg[i] != 0);
      seg[i] = 0;
    } else {
      ELL_3MV_OUTER( mat, qpoints +3*i +gradcount, qpoints +3*i +gradcount );
      ELL_3M_ADD2( sum1, sum1, mat );
      changed = changed || (seg[i] != 1);
      seg[i] = 1;
    }
  }
  
  ell_3m_eigensolve_d( eval, evec, sum0, 0 );
  ELL_3V_COPY( centroid, evec + 3*ELL_MAX3_IDX( eval[0], eval[1], eval[2] ) );
  /* ELL_3V_SCALE( centroid, ELL_3V_LEN( centroid ), centroid ); */
  
  
  ell_3m_eigensolve_d( eval, evec, sum1, 0 );
  ELL_3V_COPY( centroid +3, evec + 3*ELL_MAX3_IDX( eval[0], eval[1], eval[2] ) );
  /* ELL_3V_SCALE( centroid +3, ELL_3V_LEN( centroid ), centroid +3); Normalize */
  
  return changed;
#endif
  
  int i, sign, seg0count=0, seg1count=0, changed=AIR_FALSE;
  double oldcentroid[6], diff[3], sum[3];
  
  memcpy( oldcentroid, centroid, 6 * sizeof( double ));
  
  for( i = 0; i < gradcount; i++ ) {
    if( dists[ 0*gradcount +i] < dists[1*gradcount +i] ) {
      /* Try to resolve sign so that centroid do not end up as all 0 */
      /* Choose signs so that the point lies "on the same side" as */
      /* the previous centroid. */
      diff[0] = oldcentroid[0] - qpoints[3*i +0];
      diff[1] = oldcentroid[1] - qpoints[3*i +1];
      diff[2] = oldcentroid[2] - qpoints[3*i +2];
      sum[0] = oldcentroid[0] + qpoints[3*i +0];
      sum[1] = oldcentroid[1] + qpoints[3*i +1];
      sum[2] = oldcentroid[2] + qpoints[3*i +2];
      sign = (diff[0]*diff[0] + diff[1]*diff[1] + diff[2]*diff[2]) <
        (sum[0]*sum[0] + sum[1]*sum[1] + sum[2]*sum[2]) ? -1 : +1;
      
      changed = changed || (seg[i] != 0);
      
      seg[i] = 0;
      centroid[0] += sign * qpoints[3*i +0];
      centroid[1] += sign * qpoints[3*i +1];
      centroid[2] += sign * qpoints[3*i +2];
      seg0count++;
    } else {
      diff[0] = oldcentroid[3+0] - qpoints[3*i +0];
      diff[1] = oldcentroid[3+1] - qpoints[3*i +1];
      diff[2] = oldcentroid[3+2] - qpoints[3*i +2];
      sum[0] = oldcentroid[3+0] + qpoints[3*i +0];
      sum[1] = oldcentroid[3+1] + qpoints[3*i +1];
      sum[2] = oldcentroid[3+2] + qpoints[3*i +2];
      sign = (diff[0]*diff[0] + diff[1]*diff[1] + diff[2]*diff[2]) <
        (sum[0]*sum[0] + sum[1]*sum[1] + sum[2]*sum[2]) ? -1 : +1;
      
      changed = changed || (seg[i] != 1);
      
      seg[i] = 1;
      centroid[3+0] += sign * qpoints[3*i +0];
      centroid[3+1] += sign * qpoints[3*i +1];
      centroid[3+2] += sign * qpoints[3*i +2];
      seg1count++;
    }
  }
  centroid[0] /= seg0count;
  centroid[1] /= seg0count;
  centroid[2] /= seg0count;
  centroid[3+0] /= seg1count;
  centroid[3+1] /= seg1count;
  centroid[3+2] /= seg1count;
  
  /* printf("cent = %f %f %f %f %f %f\n", centroid[0],centroid[1],centroid[2],centroid[3],centroid[4],centroid[5] ); */
  
  /*
    Should give error if any segment contains less than 6 elements,
    i.e. if( seg0count < 6 || seg1count < 6 ), since that would
    imply that a tensor cannot be computed for that segment.
  */
  
  return changed;
}

/* Converts Q-values and gradients to points on the Q-ball surface */
void
_tenQvals2points(const int gradcount, const double qvals[],
                 const double grads[], double qpoints[] ) {
  int i;
  memcpy( qpoints, grads, 3 * gradcount * sizeof( double ) );
  for( i = 0; i < gradcount; i++ ) {
    qpoints[3*i +0] *= qvals[i];
    qpoints[3*i +1] *= qvals[i];
    qpoints[3*i +2] *= qvals[i];
  }
}

/* Calculates the shortest distances from each centroid/axis to each
   Q-ball point */
void
_tenCalcdists(const int centcount, const double centroid[],
              const int gradcount, const double qpoints[], double dists[] ) {
  int i,j;
  for( j = 0; j < centcount; j++ )
    for( i = 0; i < gradcount; i++ )
      dists[j*gradcount +i] = _tenPldist(&qpoints[3*i], &centroid[3*j]);
  
  /*
    printf("dists = ");
    for( i = 0; i < 2*gradcount; i++ )
    printf( "%f ", dists[i] );
    printf("\n");
  */
}

/* Estimates the shortest distance from a point to a line going
   through the origin */
double
_tenPldist( const double point[], const double line[] ) {
  
  double cross[3];
  double negpoint[3];
  
  negpoint[0] = -point[0];
  negpoint[1] = -point[1];
  negpoint[2] = -point[2];
  
  ELL_3V_CROSS( cross, line, negpoint );
  
  return ELL_3V_LEN( cross ) / ELL_3V_LEN( line );
}

/* Converts a segmentation into a set of 0-1 weights */
void
_tenSeg2weights(const int gradcount, const int seg[],
                const int segcount, double weights[] ) {
  int i,j;
  for( j = 0; j < segcount; j++ ) {
    for( i = 0; i < gradcount; i++ ) {
      weights[j*gradcount +i] = (seg[i] == j) ? 1 : 0;
    }
  }
  return;
}

