#include <complex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

#define MAX_ITERATIONS 255
#define ROOTS          3

#define HEIGHT   1000
#define WIDTH    1000
#define CHANNELS 3
#define COMP_SIZE sizeof(double complex)

double clamp(double x, double min, double max) {
    const double t = x < min ? min : x;
    return t > max ? max : t;
}

typedef struct {
    uint8_t r, g, b;
} Color;

int main(void) {
    static unsigned char img[WIDTH * HEIGHT * CHANNELS] = {};

    int choice, num_roots;
    double complex *roots;

    choice = 1;
    switch (choice) {
    case 1:
        num_roots = 3;
        roots = (double complex *) malloc(COMP_SIZE);
        roots[0] = 1.0;
        roots[COMP_SIZE]   = -0.5 + (sqrt(3.0)/2.0)*I;
        roots[2*COMP_SIZE] = -0.5 - (sqrt(3.0)/2.0)*I;
        break;
    default:
        printf("Invalid function choice!\n");
        exit(1);
        break;
    }

    // Colors
    Color dark_blue = { 0x03, 0x07, 0x1e };
    Color reddish = { 0x9d, 0x02, 0x08 };
    Color bright_yellow = { 0xff, 0xba, 0x08 };
    Color red = { 0xff, 0x00, 0x00 };
    Color green = { 0x00, 0xff, 0x00 };
    Color blue = { 0x00, 0x00, 0xff };
    Color white = { 0xff, 0xff, 0xff };
    Color black = { 0x00, 0x00, 0x00 };
    Color magenta = { 0xff, 0x00, 0x6e };
    Color baby_blue = { 0x3a, 0x86, 0xff };

    // Color palettes
    Color heat[3] = { dark_blue, reddish, bright_yellow };
    Color card[3] = { red, white, black };
    Color cmyk[3] = { baby_blue, magenta, bright_yellow };
    uint32_t rgb[3] = { 0xff0000, 0x00ff00, 0x0000ff };

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
        // Scale
        x *= 2.0;
        y *= 2.0;
        // Translate and flip imaginary axis
        x -= 1.0;
        y -= 1.0;
        y *= -1.0;

        double complex z = x + y * I;

        bool done = false; // Flag for determining when to exit loop
        int iterations = 0;
        while (iterations < MAX_ITERATIONS && !done) {
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
                double complex difference = z - roots[j*COMP_SIZE];
                // If the current iteration is close enough to a root, color the pixel
                if (cabs(difference) < ε) {
                    done = true; // We've converged to a root, so set flag to true
                    uint32_t col = rgb[j];

                    // Color image pixel with each channel of color
                    img[i] = (col >> 16) & 0xff; // Red channel
                    img[i + 1] = (col >> 8) & 0xff; // Blue channel
                    img[i + 2] = col & 0xff; // Green channel
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
