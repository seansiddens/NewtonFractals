#include <complex.h>
#include <stdio.h>
#include <stdint.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

#define MAX_ITERATIONS 200
#define ROOTS          3

#define HEIGHT   256
#define WIDTH    256
#define CHANNELS 3

typedef struct {
    uint8_t r, g, b;
} color;

double complex function(double complex z) {
    double complex y;

    // f(z) = z^3 - 1
    y = cpow(z, 3.0) - 1.0;

    return y;
}

double complex derivative(double complex z) {
    double complex y;

    // f'(z) = 3z^2
    y = 3.0 * z * z;

    return y;
}

int main(void) {
    unsigned char img[WIDTH * HEIGHT * CHANNELS] = {};

    double complex roots[ROOTS] = { 1.0, -0.5 + (sqrt(3.0) / 2.0) * I, -0.5 - (sqrt(3.0) / 2.0) };

    color red = {255, 0, 0};
    color green = {0, 255, 0};
    color blue = {0, 0, 255};
    color colors[ROOTS] = {red, green, blue};

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

        for (int iteration = 0; iteration < MAX_ITERATIONS; iteration++) {
            double ε = 1E-6;
            z -= function(z) / derivative(z);

            for (int j = 0; j < ROOTS; j++) {
                double complex difference = z - roots[j];

                // If the current iteration is close enough to a root, color the pixel
                if (cabs(difference) < ε) {
                    img[i + j] = (int)(255 * (iteration / (float)MAX_ITERATIONS));
                }
            }
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
