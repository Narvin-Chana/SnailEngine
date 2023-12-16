#ifndef __GAUSSIAN_KERNEL__
#define __GAUSSIAN_KERNEL__

float Luminance(float3 rgb)
{
    return dot(rgb, float3(0.2125, 0.7154, 0.0721));
}

// Precomputed gaussian kernels
// sigma 0.25, MSIZE 3
static const float GaussianKernel3[3] =
{
    0.0478, 0.9044, 0.0478
};

// sigma 0.65, MSIZE 5
static const float GaussianKernel5[5] =
{
    0.0062, 0.1961, 0.5954, 0.1961, 0.0062
};

// sigma 1, MSIZE 9
static const float GaussianKernel9[9] =
{
    0.0002, 0.0060, 0.0606, 0.2417, 0.3829, 0.2417, 0.0606, 0.0060, 0.0002  
};

// sigma 10.0, MSIZE 15
static const float GaussianKernel15[15] = { 0.031225216, 0.033322271, 0.035206333, 0.036826804, 0.038138565, 0.039104044, 0.039695028, 0.039894000, 0.039695028, 0.039104044, 0.038138565, 0.036826804, 0.035206333, 0.033322271, 0.031225216 };

// sigma 10, MSIZE 21
static const float GaussianKernel21[21] = { 0.0005, 0.0015, 0.0039, 0.0089, 0.0183, 0.0334, 0.0549, 0.0807, 0.1063, 0.1253, 0.1324, 0.1253, 0.1063, 0.0807, 0.0549, 0.0334, 0.0183, 0.0089, 0.0039, 0.0015, 0.0005 };
#endif