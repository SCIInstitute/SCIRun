# FairMesh

This module smooths surface meshes without shrinking them.

**Detailed Description**

```eval_rst
Based on the surface smoothing algorithm published by Taubin in 1995 :cite:p:`Tau1995`.
```

User Interface:

  * Weighting methods: Equal and curvature normals (default: equal)
  * Iterations: Number of times the surface is put through the smoothing algorithm (default: 50)
  * Spatial cut off frequency: Similar to low pass filter setting (default: 0.1)
  * Relaxation Parameter: negative scale factor -- the shrinking term (default: 0.6307)
  * Note: an unshrinking term is produced by: Spatial cutoff = 1/Relaxation parameter + 1/Dilation parameter
