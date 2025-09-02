#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <thread>

const int WIDTH = 8192;
const int HEIGHT = 8192;
const double XMIN = -0.7445;
const double XMAX = -0.7425;
const double YMIN = 0.1304;
const double YMAX = 0.1334;
const int MAX_ITERATION = 20;
const int NUM_THREADS = std::thread::hardware_concurrency();

class Complex {
public:
    Complex(double _real = 0, double _imag = 0) noexcept : real(_real), imag(_imag) {}
    Complex(const Complex& com) noexcept : real(com.real), imag(com.imag) {}
    Complex(const Complex&& com) noexcept : real(com.real), imag(com.imag) {}
    double& operator[](int i) { return i == 0 ? real : imag; }
    double getNormSquare() { return pow(real, 2) + pow(imag, 2); }
    Complex& operator= (const Complex&& com) { real = com.real; imag = com.imag; return *this; }
    Complex operator+ (const Complex &com) const { return Complex(real + com.real, imag + com.imag); }
    Complex operator* (const Complex &com) const { return Complex(real*com.real - imag*com.imag, real*com.imag + imag*com.real); }

private:
    double real, imag;
};


class Color {
public:
    Color(int _r = 255, int _g = 255, int _b = 255) noexcept : r(_r), g(_g), b(_b) {}
    int r, g, b;
};


int mandelbrot(Complex z, Complex c) {
    for (int iter = 0; iter < MAX_ITERATION; iter++) {
        if (z.getNormSquare() > 4.0f)
            return iter;
        z = z * z + c;
    }
    return MAX_ITERATION;
}

void generateMandelbrotSet(int *output) {
    std::vector<std::thread> threads;
    auto threadFunction = [&](int startRow, int endRow) {
        for (int y = startRow; y < endRow; y++) {
            for (int x = 0; x < WIDTH; x++)  {
                Complex c = Complex((double)(1.0f * x * (XMAX - XMIN) / WIDTH + 1.0f * XMIN), (double)(1.0f * y  * (YMAX - YMIN) / HEIGHT + 1.0f * YMIN));
                Complex z = Complex();
                int value = mandelbrot(z, c);
                output[y * WIDTH + x] = value;
            }
        }
        std::cout << ".";
    };

    int rowsPerThread = HEIGHT / NUM_THREADS;
    for (int i = 0; i < NUM_THREADS; i++) {
        int startRow = i * rowsPerThread;
        int endRow = (i == NUM_THREADS - 1) ? HEIGHT : (i + 1) * rowsPerThread;

        threads.emplace_back(threadFunction, startRow, endRow);
    }

    for (auto& thread : threads)
        thread.join();
}


int main() {
    int *output = new int[WIDTH * HEIGHT];
    std::vector<Color> colorMap;
    
    for (int i = 0; i < 50; i++)
        colorMap.push_back(Color(255, 255, 255));
    for (int i = 0; i < 45; i++)
        colorMap.push_back(Color(int(255 - 255 * i / 44), int(255 - 255 * i / 44), int(255 - 255 * i / 44)));
    for (int i = 0; i < 5; i++)
        colorMap.push_back(Color(255, 255, 255));

    generateMandelbrotSet(output);
    std::ofstream image("mandelbrot_set_cpp.ppm");
    image << "P3\n" << WIDTH << " " << HEIGHT << "\n255\n";
    
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        double value = 1.0f * (output[i] % 256) / 255;
        Color cl = colorMap[int(value * (colorMap.size()-1))];
        //std::cout << int(value * colorMap.size());
        //std::cout << value << std::endl;
        image << cl.r << " " << cl.g << " " << cl.b << "\n";
    }

    delete[] output;
    return 0;
}