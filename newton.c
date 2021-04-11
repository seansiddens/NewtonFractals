#include <complex.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

#define MAX_ITERATIONS 255
#define ROOTS          3

#define HEIGHT   1000
#define WIDTH    1000
#define CHANNELS 3

double clamp(double x, double min, double max) {
    const double t = x < min ? min : x;
    return t > max ? max : t;
}

typedef struct {
    uint8_t r, g, b;
} Color;

double colormap_red(double x) {
    if (x < 0.7) {
        return 4.0 * x - 1.5;
    } else {
        return -4.0 * x + 4.5;
    }
}

double colormap_green(double x) {
    if (x < 0.5) {
        return 4.0 * x - 0.5;
    } else {
        return -4.0 * x + 3.5;
    }
}

double colormap_blue(double x) {
    if (x < 0.3) {
       return 4.0 * x + 0.5;
    } else {
       return -4.0 * x + 2.5;
    }
}

Color colormap_jet(double x) {
    const double r = clamp(colormap_red(x), 0.0, 1.0);
    const double g = clamp(colormap_blue(x), 0.0, 1.0);
    const double b = clamp(colormap_green(x), 0.0, 1.0);

    Color col;
    col.r = (uint8_t)(r * 255.0); 
    col.g = (uint8_t)(g * 255.0); 
    col.b = (uint8_t)(b * 255.0); 
    return col;
}

double complex function(double complex z, int choice) {
    double complex y;

    switch (choice) {
        case 1:
            // f(z) = z^3 - 1
            y = cpow(z, 3.0) - 1.0;
            break;
        case 2:
            // f(z) = cosh(z) - 1

        default:
            // f(z) = z^3 - 1
            y = cpow(z, 3.0) - 1.0;
            break;
    }


    return y;
}

double complex derivative(double complex z, int choice) {
    double complex y;

    // f'(z) = 3z^2
    y = 3.0 * z * z;

    return y;
}

int main(void) {
    static unsigned char img[WIDTH * HEIGHT * CHANNELS] = {};

    double complex roots[ROOTS] = { 1.0, 
                                    -0.5 + (sqrt(3.0) / 2.0) * I, 
                                    -0.5 - (sqrt(3.0) / 2.0) * I};
    // Colors
    Color dark_blue     = {0x03, 0x07, 0x1e};
    Color reddish       = {0x9d, 0x02, 0x08};
    Color bright_yellow = {0xff, 0xba, 0x08};
    Color red           = {0xff, 0x00, 0x00};
    Color green         = {0x00, 0xff, 0x00};
    Color blue          = {0x00, 0x00, 0xff};
    Color white         = {0xff, 0xff, 0xff};
    Color black         = {0x00, 0x00, 0x00};
    Color magenta       = {0xff, 0x00, 0x6e};
    Color baby_blue     = {0x3a, 0x86, 0xff};

    // Color palettes
    Color heat[3] = {dark_blue, reddish, bright_yellow};
    Color card[3] = {red, white, black};
    Color cmyk[3] = {baby_blue, magenta, bright_yellow};
    Color rgb[3]  = {red, green, blue};

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
            double ε = 1E-6;
            z -= function(z) / derivative(z);

            for (int j = 0; j < ROOTS; j++) {
                double complex difference = z - roots[j];
                // If the current iteration is close enough to a root, color the pixel
                if (cabs(difference) < ε) {
                    done = true;
                    Color col = rgb[j];

                    double brightness = ((double)iterations / (double)MAX_ITERATIONS);
                    brightness = brightness < 0.5 ? 0.5 : brightness;
                    brightness = 1.0;


                }
            }
            iterations++;
        }

        if (!done) {
            img[i] = 0;
            img[i+1] = 0;
            img[i+2] = 0;
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
