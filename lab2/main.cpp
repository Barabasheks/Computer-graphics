#include <iostream>
#include <cmath>
using namespace std;

double fpart(double x) {
    return x - (int)x;
}

struct Pnm{
    int version;
    unsigned int width;
    unsigned int height;
    unsigned int depth;
    int size;
    unsigned char* bytes;

    void plot(int x, int y, double brightness, double gamma){
        bytes[y * width + x] = pow(brightness / depth, gamma) * depth;
    }
};

const double STANDART_GAMMA = 2.2;

int main(int argc, char* argv[]) {
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

    //Начертание линии
    double brightness = picture.depth - atof(argv[3]);
    double thickness = atof(argv[4]);
    double x1 = atof(argv[5]);
    double y1 = atof(argv[6]);
    double x2 = atof(argv[7]);
    double y2 = atof(argv[8]);
    double gamma;
    if (argc < 10)
        gamma = STANDART_GAMMA;
    else
        gamma = atof(argv[9]);

    if (x2 < x1) {
        swap(x1, x2);
        swap(y1, y2);
    }
    double dx = x2 - x1;
    double dy = y2 - y1;

    if (dx > dy) {
        double gradient = dy / dx;
        int xpxl1 = round(x1 + (thickness * dy)/(2*sqrt(dx * dx + dy * dy)));
        double yend = y1 - (thickness * dx)/(2*sqrt(dx * dx + dy * dy));
        double intery = yend;
        int xpxl2 = round(x2 - (thickness * dy)/(2*sqrt(dx * dx + dy * dy)));

        for (int i = 0; i < thickness; ++i) {
            picture.plot(round(xpxl1 - i * gradient), (int) (intery) + i,
                    picture.depth - brightness + 1 * brightness, gamma);
        }

        intery = yend;
        for (int x = xpxl1 + 1; x < xpxl2; ++x) {
            picture.plot(x, (int)(intery), picture.depth - brightness + fpart(intery) * brightness, gamma);
            for (int i = 1; i < thickness - 1; ++i) {
                picture.plot(round(x - i * gradient), (int) (intery) + i, picture.depth - brightness, gamma);
                picture.plot(round(x - i * gradient), (int) (intery) + i + 1, picture.depth - brightness, gamma);
            }
            picture.plot(round(x - thickness * gradient), (int)(intery + thickness),
                    picture.depth - brightness + (1 - fpart(intery)) * brightness,
                    gamma);
            intery = intery + gradient;
        }

        double grad = gradient;
        for (int i = 0; i < thickness - 1; ++i) {
            picture.plot(round(xpxl2 - i * gradient), (int) (intery) + i + 1,
                    picture.depth - brightness + grad * brightness, gamma);
            if (grad + gradient > 1)
                grad = gradient;
            else
                grad += gradient;
        }
//    } else{
//        double gradient = dx / dy;
//        int ypxl1 = round(y1);
//        double xend = x1 + gradient * (ypxl1 - y1);
//        double intery = xend + gradient;
//        int ypxl2 = round(y2);
//        for (int y = ypxl1; y < ypxl2; ++y) {
//            picture.plot((int)(intery), y, picture.depth - brightness + fpart(intery) * brightness, gamma);
//            for (int i = 1; i < thickness; ++i) {
//                picture.plot((int)(intery) + i, y, picture.depth - brightness, gamma);
//            }
//            picture.plot((int)(intery) + thickness, y, picture.depth - brightness + (1 - fpart(intery)) * brightness,
//                         gamma);
//            intery = intery + gradient;
//        }
    }


    FILE* new_file = fopen(argv[2], "wb");
    if(new_file == NULL){
        cerr << "File open error";
        return 1;
    }
    rez = fprintf(new_file, "P%d\n%d %d\n%d\n", picture.version, picture.width, picture.height, picture.depth);
    if (rez != 17){
        cerr << "Write file error";
        return 1;
    }
    frez = fwrite(picture.bytes, 1, picture.size, new_file);
    if (frez != picture.size){
        cerr << "Write file error";
        return 1;
    }
    fclose(new_file);
    return 0;
}
