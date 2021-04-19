#include <complex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

#define MAX_ITERATIONS 500
#define ROOTS          3

#define HEIGHT    500
#define WIDTH     500
#define CHANNELS  3
#define COMP_SIZE sizeof(double complex)

int main(void) {
    // Image buffer
    static unsigned char img[WIDTH * HEIGHT * CHANNELS] = {};

    // File for writing data to
    FILE *fp = fopen("data.txt", "w");
    if (fp == NULL) {
        printf("data.txt file could not be opened/created!\n");
        exit(1);
    }

    int num_roots;
    double complex *roots;

    int choice = 1; // Choice of function
    // Initialize roots array depending on choice of function
    switch (choice) {
    case 1:
        // f(z) = z^3 - 1.0
        num_roots = 3;
        roots = (double complex *) malloc(COMP_SIZE);
        roots[0] = 1.0;
        roots[COMP_SIZE] = -0.5 + (sqrt(3.0) / 2.0) * I;
        roots[2 * COMP_SIZE] = -0.5 - (sqrt(3.0) / 2.0) * I;
        break;
    default:
        printf("Invalid function choice!\n");
        exit(1);
        break;
    }

    // Color palettes
    uint32_t rgb[3] = { 0xff0000, 0x00ff00, 0x0000ff };
    uint32_t heat[3] = { 0x03071e, 0x9d0208, 0xffba08 };

    double scale_factor = 2.0;

    int max_iter_count = 0;
    printf("Rendering image...\n");
    for (int i = 0; i < WIDTH * HEIGHT * CHANNELS; i += 3) {
        double x = i % (WIDTH * CHANNELS) / CHANNELS;
        double y = i / WIDTH / CHANNELS;

        printf("Progress: %0.2f%%\r", y / HEIGHT * 100.0);
        fflush(stdout);

        // Remap to complex range
        // Normalize
        x = x / WIDTH;
        y = y / HEIGHT;
        // Scale by scale factor
        x *= scale_factor;
        y *= scale_factor;
        // Translate and flip imaginary axis
        x -= scale_factor / 2.0;
        y -= scale_factor / 2.0;
        y *= -1.0;

        double complex z = x + y * I;
        double complex prev_z;

        bool done = false; // Flag for determining when to exit loop
        int iterations = 0;
        while (iterations < MAX_ITERATIONS && !done) {
            // Keep track of max iteration
            if (iterations > max_iter_count) {
                max_iter_count = iterations;
            }

            // Keep track of z value from previous iteration
            prev_z = z;

            // Choose an iteration function
            switch (choice) {
            case 1:
                // f(z) = z^3 - 1
                // f'(z) = 3z^2
                // f_n+1 = f_n - f(n) / f'(n)
                z = z - (cpow(z, 3.0) - 1.0) / (3 * cpow(z, 2.0));
                break;
            default:
                printf("Invalid function choice!\n");
                exit(1);
                break;
            }

            // Go through each root and check if iteration is close enough
            double ε = 1E-6;
            for (int j = 0; j < num_roots; j++) {
                // Get distance from current iteration to one of roots
                double complex difference = z - roots[j * COMP_SIZE];
                // If the current iteration is close enough to a root, color the pixel
                if (cabs(difference) < ε) {
                    done = true; // We've converged to a root, so set flag to true
                    double d0 = cabs(prev_z - roots[j * COMP_SIZE]);
                    double d1 = cabs(difference);

                    // Fetch red, green, and blue channels from color value
                    uint32_t col = rgb[j];
                    uint8_t r = (col >> 16) & 0xff; // Red channel
                    uint8_t g = (col >> 8) & 0xff; // Blue channel
                    uint8_t b = col & 0xff; // Green channel

                    // Color image pixel with each channel of color
                    double fract = (log(ε) - log(d0)) / (log(d1) - log(d0));
                    //double brightness = 1.0 - 0.5 * log((double)iterations+fract);
                    //double brightness = 1.0 - (log((double)iterations+fract+1.0) / log(50.0));
                    double brightness = 40.0 / (pow(iterations + fract, 2) + 40.0);

                    brightness = clamp(brightness, 0.0, 1.0);
                    img[i] = (uint8_t)(r * brightness);
                    img[i + 1] = (uint8_t)(g * brightness);
                    img[i + 2] = (uint8_t)(b * brightness);
                }
            }
            iterations++;
        }

        // If we didn't converge on anything, color pixel black
        if (!done) {
            img[i] = 0;
            img[i + 1] = 0;
            img[i + 2] = 0;
        }
    }

    printf("Progress: 100.00%%");
    printf("\nDone!\n");

    if (stbi_write_png("out.png", WIDTH, HEIGHT, CHANNELS, img, WIDTH * CHANNELS) == 0) {
        fprintf(stderr, "Failed to write to image!\n");
        exit(1);
    }

    return 0;
}
