#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <cuda_runtime.h>

#define W 12800
#define H 12800
#define IMG_FILE "mandelbrot.ppm"
#define BLOCK_SIZE 16

int pic[W][H];

void write_ppm(FILE *fp, int step);

__device__ double mandelbrot(double x, double y) {
    int n = 0;
    double a = 0, b = 0, c, d;
    while ((c = a * a) + (d = b * b) < 4 && n++ < 880) {
        b = 2 * a * b + y * 1024 / H * 8e-9 - 0.645411;
        a = c - d + x * 1024 / W * 8e-9 + 0.356888;
    }
    return n;
}

__global__ void mandelbrot_kernel(int *pic_d) {
    // Calculate pixel coordinates from thread and block indices
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    
    // Check if within image bounds
    if (x < W && y < H) {
        // Compute Mandelbrot value for this pixel
        int n = mandelbrot(x, y);
        // Store result in global memory
        pic_d[y * W + x] = n;
    }
}

int main(int argc, char *argv[]) {
    // Allocate device memory for the result
    int *pic_d;
    cudaMalloc(&pic_d, W * H * sizeof(int));
    
    // Define grid and block dimensions
    dim3 blockDim(BLOCK_SIZE, BLOCK_SIZE);
    dim3 gridDim((W + BLOCK_SIZE - 1) / BLOCK_SIZE, (H + BLOCK_SIZE - 1) / BLOCK_SIZE);
    
    // Launch kernel
    printf("Rendering Mandelbrot set with CUDA (%d x %d pixels)...\n", W, H);
    float elapsed = 0;
    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    
    cudaEventRecord(start);
    mandelbrot_kernel<<<gridDim, blockDim>>>(pic_d);
    cudaEventRecord(stop);
    
    // Wait for kernel to finish
    cudaDeviceSynchronize();
    
    // Check for errors
    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) {
        fprintf(stderr, "CUDA Error: %s\n", cudaGetErrorString(err));
        return 1;
    }
    
    // Copy result back to host
    cudaMemcpy(pic, pic_d, W * H * sizeof(int), cudaMemcpyDeviceToHost);
    
    // Calculate elapsed time
    cudaEventElapsedTime(&elapsed, start, stop);
    printf("Render time: %.1f ms\n", elapsed);
    
    // High-resolution final image
    FILE *fp = fopen(IMG_FILE, "w");
    assert(fp);
    write_ppm(fp, 2);
    fclose(fp);
    
    // Clean up
    cudaFree(pic_d);
    cudaEventDestroy(start);
    cudaEventDestroy(stop);
    
    return 0;
}

void write_ppm(FILE *fp, int step) { 
    // Portable Pixel Map (PPM)

    int w = W / step, h = H / step;

    fprintf(fp, "P6\n%d %d 255\n", w, h);
    for (int j = 0; j < H; j += step) {
        for (int i = 0; i < W; i += step) {
            int n = pic[i][j];
            int r = 255 * pow((n - 80) / 800.0, 3);
            int g = 255 * pow((n - 80) / 800.0, 0.7);
            int b = 255 * pow((n - 80) / 800.0, 0.5);
            fprintf(fp, "%c%c%c", r, g, b);
        }
    }
}