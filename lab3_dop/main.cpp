#include <iostream>
#include <string>
#include <cmath>
#include <random>
#include <vector>
using namespace std;

struct Pnm {
    int version;
    unsigned int width;
    unsigned int height;
    unsigned int depth;
    unsigned int size;
    unsigned char *bytes;

    unsigned char color(int x, int y) {
        return bytes[y * width + x];
    }

    void plot(int x, int y, double value, double gamma){
        if (gamma == 0) {
            value = value / depth;
            value = (value <= 0.0031308 ? value * 12.92 : pow(1.055 * value, 0.416) - 0.055);
            bytes[y * width + x] = value * depth;
        } else{
            bytes[y * width + x] = pow((value / depth),1 / gamma) * depth;
        }
    }

    void gamma_correction(){
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                double value = double(bytes[y * width + x]) / depth;
                value = (value < 0.04045 ? value / 12.92 : pow((value + 0.055) / 1.055, 2.4));
                bytes[y * width + x] = value * depth;
            }
        }
    }

    void error_dispersion_gradient(const vector<vector<double>>& mistake_map, int bits, double gamma){
        vector<vector<double>> new_bytes (height, vector<double>(width, 0));
        double color = 0;
        double step_color = depth / (pow(2, bits) - 1);
        double grad = (double)depth / height;
        double mistake = 0;
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                int new_value = round((color + new_bytes[i][j]) / step_color) * step_color;
                mistake = (color + new_bytes[i][j]) - new_value;
                new_bytes[i][j] = new_value;
                for (int mist_i = i; mist_i < i + mistake_map.size(); ++mist_i) {
                    for (int mist_j = j - (mistake_map.size() - 1); mist_j < (j - (mistake_map.size() - 1)) + mistake_map[0].size(); ++mist_j) {
                        if (mist_i >= 0 && mist_i < height && mist_j >= 0 && mist_j < width)
                            new_bytes[mist_i][mist_j] += mistake * mistake_map[mist_i - i][mist_j - (j - (mistake_map.size() - 1))];
                    }
                }
            }
            color += grad;
        }
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                unsigned char value_color = (new_bytes[i][j] >= 0 ? new_bytes[i][j]: 0);
                value_color = (value_color <= depth ? value_color: depth);
                plot(j, i, value_color, gamma);
            }
        }
    }

    void error_dispersion_picture(const vector<vector<double>>& mistake_map, int bits, double gamma){
        vector<vector<double>> new_bytes (height, vector<double>(width, 0));
        double step_color = depth / (pow(2, bits) - 1);
        double mistake = 0;
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                int new_value = round((color(j, i) + new_bytes[i][j]) / step_color) * step_color;
                mistake = (color(j, i) + new_bytes[i][j]) - new_value;
                new_bytes[i][j] = new_value;
                for (int mist_i = i; mist_i < i + mistake_map.size(); ++mist_i) {
                    for (int mist_j = j - (mistake_map.size() - 1); mist_j < (j - (mistake_map.size() - 1)) + mistake_map[0].size(); ++mist_j) {
                        if (mist_i >= 0 && mist_i < height && mist_j >= 0 && mist_j < width)
                            new_bytes[mist_i][mist_j] += mistake * mistake_map[mist_i - i][mist_j - (j - (mistake_map.size() - 1))];
                    }
                }
            }
        }
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                unsigned char value_color = (new_bytes[i][j] >= 0 ? new_bytes[i][j]: 0);
                value_color = (value_color <= depth ? value_color: depth);
                plot(j, i, value_color, gamma);
            }
        }
    }
};

enum Gradient_or_Picture{
    picture,
    gradient
};

enum Dithering{
    no_dithering,
    ordered,
    random,
    floyd_steinberg,
    jarvis_judice_ninke,
    sierra,
    atkinson,
    halftone
};

struct Maps{
    const vector<vector<double>> ordered_map = {{0, 48./64, 12./64, 60./64, 3./64, 51./64, 15./64, 63./64},
                                  {32./64, 16./64, 44./64, 28./64, 35./64, 19./64, 47./64, 31./64},
                                  {8./64, 56./64, 4./64, 52./64, 11./64, 59./64, 7./64, 55./64},
                                  {40./64, 24./64, 36./64, 20./64, 43./64, 27./64, 39./64, 23./64},
                                  {2./64, 50./64, 14./64, 62./64, 1./64, 49./64, 13./64, 61./64},
                                  {34./64, 18./64, 46./64, 30./64, 33./64, 17./64, 45./64, 29./64},
                                  {10./64, 58./64, 6./64, 54./64, 9./64, 57./64, 5./64, 53./64},
                                  {42./64, 26./64, 38./64, 22./64, 41./64, 25./64, 37./64, 21./64}};

    const vector<vector<double>> floyd_steinberg_map = {{0, 0, 7./16}, {3./16, 5./16, 1./16}};

    const vector<vector<double>> jarvis_judice_ninke_map = {{0, 0, 0, 7./48, 5./48},
                                          {3./48, 5./48, 7./48, 5./48, 3./48},
                                          {1./48, 3./48, 5./48, 3./48, 1./48}};

    const vector<vector<double>> sierra_map = {{0, 0, 0, 5./32, 3./32},
                                          {2./32, 4./32, 5./32, 4./32, 2./32},
                                          {0, 2./32, 3./32, 2./32, 0}};

    const vector<vector<double>> atkinson_map = {{0, 0, 0, 1./8, 1./8},
                                          {0, 1./8, 1./8, 1./8, 0},
                                          {0, 0, 1./8, 0, 0}};

    const vector<vector<double>> halftone_map = {{13.0 / 16.0, 11.0 / 16.0, 4.0 / 16.0, 8.0 / 16.0},
                                                 {6.0 / 16.0, 0, 3.0 / 16.0, 15.0 / 16.0},
                                                 {14.0 / 16.0, 1.0 / 16.0, 2.0 / 16.0, 7.0 / 16.0},
                                                 {9.0 / 16.0, 5.0 / 16.0, 10.0 / 16.0, 12.0 / 16.0}};
};

pair<unsigned int, unsigned int> parse_size(char* str_size){
    int i = 0;
    string width, height;
    while(str_size[i] != 'x'){
        width += str_size[i];
        i++;
    }
    i++;
    while(str_size[i] != '\0'){
        height += str_size[i];
        i++;
    }
    pair<unsigned int, unsigned int> size (stoi(width), stoi(height));
    return size;
}

int main(int argc, char* argv[]) {
    Maps maps;
    Gradient_or_Picture gr_or_pic = static_cast<Gradient_or_Picture>(atoi(argv[3]));
    Dithering dithering = static_cast<Dithering>(atoi(argv[4]));
    int bits = atoi(argv[5]);
    double gamma = atof(argv[6]);
    Pnm pic;

    if (gr_or_pic == picture) {
        //read file
        FILE *file = fopen(argv[1], "rb");
        if (file == NULL) {
            cerr << "File open error";
            return 1;
        }
        int rez = fscanf(file, "P%u\n%d %d\n%d\n", &pic.version, &pic.width, &pic.height, &pic.depth);
        if (rez != 4) {
            cerr << "Read error";
            return 1;
        }
        pic.size = pic.width * pic.height;
        pic.bytes = new unsigned char[pic.size];
        if (pic.bytes == NULL) {
            cerr << "Memory allocation error";
            return 1;
        }
        size_t frez = fread(pic.bytes, 1, pic.size, file);
        if (frez != pic.size) {
            cerr << "read file error";
            return 1;
        }
        fclose(file);
        pic.gamma_correction();

        switch (dithering) {
            case no_dithering: {
                double step_color = pic.depth / (pow(2, bits) - 1);
                for (int i = 0; i < pic.height; ++i) {
                    for (int j = 0; j < pic.width; ++j) {
                        int value_color = round(pic.color(j, i) / step_color) * step_color;
                        value_color = (value_color > 255 ? 255: value_color);
                        if (value_color == 255)
                            cout << "";
                        pic.plot(j, i, value_color, gamma);
                    }
                }
                break;
            }
            case ordered:{
                vector<vector<double>> map = maps.ordered_map;
                double step_color = pow(2, bits) - 1;
                for (int i = 0; i < pic.height; ++i) {
                    for (int j = 0; j < pic.width; ++j) {
                        double change_color = (double)pic.color(j, i) / pic.depth + (map[i % map.size()][j % map.size()] - 0.5) / bits;
                        int value_color = round(change_color * step_color) /step_color * pic.depth;
                        value_color = (value_color > 255 ? 255: value_color);
                        value_color = (value_color < 0 ? 0: value_color);
                        pic.plot(j, i, value_color, gamma);
                    }
                }
                break;
            }
            case random:{
                default_random_engine generator;
                uniform_real_distribution<double> distribution(0, 1);

                double step_color = pic.depth / (pow(2, bits) - 1);
                for (int i = 0; i < pic.height; ++i) {
                    for (int j = 0; j < pic.width; ++j) {
                        int value_color = (int)(pic.color(j, i) / step_color + distribution(generator)) * step_color;
                        value_color = (value_color > 255 ? 255: value_color);
                        pic.plot(j, i, value_color, gamma);
                    }
                }
                break;
            }
            case floyd_steinberg:{
                vector<vector<double>> mistake_map = maps.floyd_steinberg_map;
                pic.error_dispersion_picture(mistake_map, bits, gamma);
                break;
            }
            case jarvis_judice_ninke:{
                vector<vector<double>> mistake_map = maps.jarvis_judice_ninke_map;
                pic.error_dispersion_picture(mistake_map, bits, gamma);
                break;
            }
            case sierra:{
                vector<vector<double>> mistake_map = maps.sierra_map;
                pic.error_dispersion_picture(mistake_map, bits, gamma);
                break;
            }
            case atkinson:{
                vector<vector<double>> mistake_map = maps.atkinson_map;
                pic.error_dispersion_picture(mistake_map, bits, gamma);
                break;
            }
            case halftone:{
                vector<vector<double>> map = maps.halftone_map;
                double step_color = pow(2, bits) - 1;
                for (int i = 0; i < pic.height; ++i) {
                    for (int j = 0; j < pic.width; ++j) {
                        double change_color = (double)pic.color(j, i) / pic.depth + (map[i % map.size()][j % map.size()] - 0.5) / bits;
                        int value_color = round(change_color * step_color) /step_color * pic.depth;
                        value_color = (value_color > 255 ? 255: value_color);
                        value_color = (value_color < 0 ? 0: value_color);
                        pic.plot(j, i, value_color, gamma);
                    }
                }
                break;
            }
        }
    } else {
        pair<unsigned int, unsigned int> size = parse_size(argv[1]);
        pic = {5, size.first, size.second, 255, size.first * size.second};
        pic.bytes = new unsigned char[pic.size];

        switch (dithering) {
            case no_dithering: {
                double color = 0;
                double step_color = pic.depth / (pow(2, bits) - 1);
                double grad = (double)pic.depth / pic.height;
                for (int i = 0; i < pic.height; ++i) {
                    for (int j = 0; j < pic.width; ++j) {
                        int value_color = round(color / step_color) * step_color;
                        value_color = (value_color > 255 ? 255: value_color);
                        pic.plot(j, i, value_color, gamma);
                    }
                    color += grad;
                }
                break;
            }
            case ordered:{
                vector<vector<double>> map = maps.ordered_map;
                double color = 0;
                double step_color = pow(2, bits) - 1;
                double grad = (double)pic.depth / pic.height;
                for (int i = 0; i < pic.height; ++i) {
                    for (int j = 0; j < pic.width; ++j) {
                        double change_color = color / pic.depth + (map[i % map.size()][j % map.size()] - 0.5) / bits;
                        int value_color = round(change_color * step_color) /step_color * pic.depth;
                        value_color = (value_color > 255 ? 255: value_color);
                        value_color = (value_color < 0 ? 0: value_color);
                        pic.plot(j, i, value_color, gamma);
                    }
                    color += grad;
                }
                break;
            }
            case random:{
                default_random_engine generator;
                uniform_real_distribution<double> distribution(0, 1);

                double color = 0;
                double step_color = pic.depth / (pow(2, bits) - 1);
                double grad = (double)pic.depth / pic.height;
                for (int i = 0; i < pic.height; ++i) {
                    for (int j = 0; j < pic.width; ++j) {
                        int value_color = (int)(color / step_color + distribution(generator)) * step_color;
                        value_color = (value_color > 255 ? 255: value_color);
                        pic.plot(j, i, value_color, gamma);
                    }
                    color += grad;
                }
                break;
            }
            case floyd_steinberg:{
                vector<vector<double>> mistake_map = maps.floyd_steinberg_map;
                pic.error_dispersion_gradient(mistake_map, bits, gamma);
                break;
            }
            case jarvis_judice_ninke:{
                vector<vector<double>> mistake_map = maps.jarvis_judice_ninke_map;
                pic.error_dispersion_gradient(mistake_map, bits, gamma);
                break;
            }
            case sierra:{
                vector<vector<double>> mistake_map = maps.sierra_map;
                pic.error_dispersion_gradient(mistake_map, bits, gamma);
                break;
            }
            case atkinson:{
                vector<vector<double>> mistake_map = maps.atkinson_map;
                pic.error_dispersion_gradient(mistake_map, bits, gamma);
                break;
            }
            case halftone:{
                vector<vector<double>> map = maps.halftone_map;
                double color = 0;
                double step_color = pow(2, bits) - 1;
                double grad = (double)pic.depth / pic.height;
                for (int i = 0; i < pic.height; ++i) {
                    for (int j = 0; j < pic.width; ++j) {
                        double change_color = color / pic.depth + (map[i % map.size()][j % map.size()] - 0.5) / bits;
                        int value_color = round(change_color * step_color) /step_color * pic.depth;
                        value_color = (value_color > 255 ? 255: value_color);
                        value_color = (value_color < 0 ? 0: value_color);
                        pic.plot(j, i, value_color, gamma);
                    }
                    color += grad;
                }
                break;
            }
        }
    }

    //write file
    FILE *new_file = fopen(argv[2], "wb");
    if (new_file == NULL) {
        cerr << "File open error";
        return 1;
    }
    int rez = fprintf(new_file, "P%d\n%d %d\n%d\n", pic.version, pic.width, pic.height, pic.depth);
    if (rez != 9 + to_string(pic.width).size() + to_string(pic.height).size()) {
        cerr << "Write file error at header";
        return 1;
    }
    size_t frez = fwrite(pic.bytes, 1, pic.size, new_file);
    if (frez != pic.size) {
        cerr << "Write file error at bytes";
        return 1;
    }
    fclose(new_file);
    return 0;
}
