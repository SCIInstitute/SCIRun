# SmoothVecFieldMedian

This function smooths vectors assigned to the elements of a mesh using a median filter. The filter creates a neighborhood by asking for elements that share faces two levels deep. The current vector is replaced by the vector with the median vector angle.

**Detailed Description**

Input requires a mesh with vectors assigned to the elements. The output produces the same mesh with a smooth vector field based on the median filter of the vector angle.  
