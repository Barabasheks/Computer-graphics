#include <iostream>
#include <cmath>
using namespace std;

struct Pnm{
    int version;
    unsigned int width;
    unsigned int height;
    unsigned int depth;
    int size;
    unsigned char* bytes;

    void plot(int x, int y, double brightness, bool steep, double color, bool gamma, double gamma_value) {
        if (steep) {
            swap(x, y);
        }
        if(brightness < 0)
            return;
        if (x < 0 || x >= width || y < 0 || y >= height) {
            cerr << "Write file error";
            exit(1);
        }
        if (!gamma) {
            double oldValue = double((unsigned char) bytes[y * width + x]) / depth;
            oldValue = (oldValue < 0.04045 ? oldValue / 12.92 : pow((oldValue + 0.055) / 1.055, 2.4));
            oldValue *= (1.0 - brightness);
            oldValue += brightness * color / 255.0;
            oldValue = (oldValue <= 0.0031308 ? oldValue * 12.92 : pow(1.055 * oldValue, 0.416) - 0.055);
            bytes[y * width + x] = oldValue * depth;
        } else{
            bytes[y * width + x] = 255 * pow(((brightness / depth) * bytes[y * width + x] + brightness * (1 - (color / depth))) / 255.0,
                                            (1.0 / gamma - 1.0) * (1.0 - (brightness / depth)) + 1.0);
        }
    }

};

double sqrt(int x, int y, int x1, int y1) {
    return sqrt((x - x1) * (x - x1) + (y - y1) * (y - y1));
}

double findY(double x, double x1, double y1, double gradient) {
    return y1 + gradient * (x - x1);
}

int main(int argc, char* argv[]) {
    //read file
    FILE* file = fopen(argv[1], "rb");
    if(file == NULL){
        cerr << "File open error";
        return 1;
    }
    Pnm picture;
    int rez = fscanf(file, "P%u\n%d %d\n%d\n", &picture.version, &picture.width, &picture.height, &picture.depth);
    if (rez != 4){
        cerr << "Read error";
        return 1;
    }
    picture.size = picture.width * picture.height;
    picture.bytes = new unsigned char[picture.size];
    if(picture.bytes == NULL){
        cerr << "Memory allocation error";
        return 1;
    }
    size_t frez = fread(picture.bytes, 1, picture.size, file);
    if(frez != picture.size){
        cerr << "read file error";
        return 1;
    }
    fclose(file);

    double color = atof(argv[3]);
    double thickness = atof(argv[4]);
    double x1 = atof(argv[5]);
    double y1 = atof(argv[6]);
    double x2 = atof(argv[7]);
    double y2 = atof(argv[8]);
    bool gamma = false;
    double gamma_value;
    if(argc == 10){
        gamma = true;
        gamma_value = atof(argv[9]);
    }
    //write line
    bool steep = abs(x1 - x2) < abs(y1 - y2);
    if (steep) {
        swap(x1, y1);
        swap(x2, y2);
    }
    if (x1 > x2) {
        swap(x1, x2);
        swap(y1, y2);
    }

    double gradient = (y2 - y1) / (x2 - x1);
    double y;
    for (int plotX = round(x1) - thickness / 2; plotX < round(x1); plotX++) {
        y = findY(plotX, x1, y1, gradient);
        double pointx = round(x1);
        double pointy = findY(round(x1), x1, y1, gradient);
        for (int plotY = int(y - (thickness - 1) / 2.0); plotY <= int(y - (thickness - 1) / 2.0 + thickness); plotY++) {
            picture.plot(plotX, plotY, min(1.0, (thickness + 1.0) / 2.0 - sqrt(plotX, plotY,
                    pointx, pointy)), steep, color, gamma, gamma_value);
        }
    }

    for (int plotX = round(x1); plotX <= round(x2); plotX++) {
        y = findY(plotX, x1, y1, gradient);
        for (int plotY = int(y - (thickness - 1) / 2.0); plotY <= int(y - (thickness - 1) / 2.0 + thickness); plotY++) {
            picture.plot(plotX, plotY, min(1.0, (thickness + 1.0) / 2.0 - fabs(y - plotY)), steep, color, gamma, gamma_value);
        }
    }

    double end_x = round(x2);
    double end_y = round(y2);
    for (int plotX = round(x2) + 1; plotX <= round(x2) + thickness / 2; plotX++) {
        y = y1 + gradient * (plotX - x1);
        for (int plotY = int(y - (thickness - 1) / 2.0); plotY <= int(y - (thickness - 1) / 2.0 + thickness); plotY++) {
            picture.plot(plotX, plotY, min(1.0, (thickness + 0.5) / 2.0 - sqrt(plotX, plotY, end_x, end_y)), steep, color, gamma, gamma_value);
        }
    }

    //write file
    FILE* new_file = fopen(argv[2], "wb");
    if(new_file == NULL){
        cerr << "File open error";
        return 1;
    }
    rez = fprintf(new_file, "P%d\n%d %d\n%d\n", picture.version, picture.width, picture.height, picture.depth);
    if (rez != 15){
        cerr << "Write file error at header";
        return 1;
    }
    frez = fwrite(picture.bytes, 1, picture.size, new_file);
    if (frez != picture.size){
        cerr << "Write file error at bytes";
        return 1;
    }
    fclose(new_file);
    return 0;
}
