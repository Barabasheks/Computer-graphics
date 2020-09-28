#include <stdio.h>
#include <stdlib.h>

enum vP{
    P5 = 5,
    P6 = 6
};

enum Func{
    finversion,
    fhorizontal_mirror_ref,
    fvertical_mirror_ref,
    fturn_clockwise,
    fturn_counterclockwise
};

struct Pnm{
    enum vP version;
    unsigned int width;
    unsigned int height;
    unsigned int depth;
    int size;
    unsigned char* bytes;
    void (*inversion)(struct Pnm*);
    void (*horizontal_mirror_ref)(struct Pnm*);
    void (*vertical_mirror_ref)(struct Pnm*);
    int (*turn_clockwise)(struct Pnm*);
    int (*turn_counterclockwise)(struct Pnm*);
};

void inversion(struct Pnm* picture){
    for (int i = 0; i < picture->size; ++i) {
        picture->bytes[i] = picture->depth - picture->bytes[i];
    }
}

void horizontal_mirror_ref(struct Pnm* picture){
    if (picture->version == P5){
        for (int i = 0; i < picture->height / 2; ++i) {
            for (int j = 0; j < picture->width; ++j) {
                int pixel = picture->bytes[i * picture->width + j];
                picture->bytes[i * picture->width + j] = picture->bytes[(picture->height - i - 1) * picture->width + j];
                picture->bytes[(picture->height - i - 1) * picture->width + j] = pixel;
            }
        }
    } else{
        for (int i = 0; i < picture->height / 2; ++i) {
            for (int j = 0; j < picture->width * 3; ++j) {
                int pixel = picture->bytes[i * picture->width * 3 + j];
                picture->bytes[i * picture->width * 3 + j] = picture->bytes[
                        (picture->height - i - 1) * picture->width * 3 + j];
                picture->bytes[(picture->height - i - 1) * picture->width * 3 + j] = pixel;
            }
        }
    }
}

void vertical_mirror_ref(struct Pnm* picture){
    if (picture->version == P5){
        for (int i = 0; i < picture->height; ++i) {
            for (int j = 0; j < picture->width / 2; ++j) {
                int pixel = picture->bytes[i * picture->width + j];
                picture->bytes[i * picture->width + j] = picture->bytes[i * picture->width + (picture->width - j - 1)];
                picture->bytes[i * picture->width + (picture->width - j - 1)] = pixel;
            }
        }
    } else{
        for (int i = 0; i < picture->height; ++i) {
            for (int j = 0; j < (picture->width / 2) * 3; j += 3) {
                int pixel[3];
                for (int k = 0; k < 3; ++k) {
                    pixel[k] = picture->bytes[i * picture->width * 3 + j + k];
                    picture->bytes[i * picture->width * 3 + j + k] = picture->bytes[i * picture->width * 3 +
                                                                                (picture->width * 3 - j - (3 - k))];
                    picture->bytes[i * picture->width * 3 + (picture->width * 3 - j - (3 - k))] = pixel[k];
                }
            }
        }
    }
}

int turn_clockwise(struct Pnm* picture){
    unsigned char* new_bytes =(unsigned char*) malloc(picture->size);
    if(new_bytes == NULL){
        printf("Memory allocation error");
        return 1;
    }
    int num = picture->height;
    picture->height = picture->width;
    picture->width = num;
    int l = 0;
    if (picture->version == P5){
        for (int i = picture->width - 1; i >= 0; --i) {
            for (int j = 0; j < picture->height; ++j) {
                new_bytes[i + picture->width * j] = picture->bytes[l];
                l++;
            }
        }
    } else{
        for (int i = picture->width * 3 - 1; i >= 0; i -= 3) {
            for (int j = 0; j < picture->height; ++j) {
                for (int k = 0; k < 3; ++k) {
                    new_bytes[i + picture->width * 3 * j - 2 + k] = picture->bytes[l];
                    l++;
                }
            }
        }
    }
    picture->bytes = new_bytes;
    return 0;
}

int turn_counterclockwise(struct Pnm* picture){
    unsigned char* new_bytes =(unsigned char*) malloc(picture->size);
    if(new_bytes == NULL){
        printf("Memory allocation error");
        return 1;
    }
    int num = picture->height;
    picture->height = picture->width;
    picture->width = num;
    int l = 0;
    if (picture->version == P5){
        for (int i = 0; i < picture->width; ++i) {
            for (int j = picture->height - 1; j >= 0; --j) {
                new_bytes[i + picture->width * j] = picture->bytes[l];
                l++;
            }
        }
    } else{
        for (int i = 0; i < picture->width * 3 ; i += 3) {
            for (int j = picture->height - 1; j >= 0; --j) {
                for (int k = 0; k < 3; ++k) {
                    new_bytes[i + picture->width * 3 * j + k] = picture->bytes[l];
                    l++;
                }
            }
        }
    }
    picture->bytes = new_bytes;
    return 0;
}

int main(int argc, char* argv[]) {
    FILE* file = fopen(argv[1], "rb");
    if(file == NULL){
        fprintf(stderr, "File open error");
        return 1;
    }
    struct Pnm picture;
    picture.inversion = &inversion;
    picture.vertical_mirror_ref = &vertical_mirror_ref;
    picture.horizontal_mirror_ref = &horizontal_mirror_ref;
    picture.turn_clockwise = &turn_clockwise;
    picture.turn_counterclockwise = &turn_counterclockwise;
    int rez = fscanf(file, "P%u\n%d %d\n%d\n", &picture.version, &picture.width, &picture.height, &picture.depth);
    if (rez != 4){
        fprintf(stderr, "Wrong format");
        return 1;
    }
    if (picture.version == P6)
        picture.size = picture.height * picture.width * 3;
    else if (picture.version == P5)
        picture.size = picture.height * picture.width;
    else{
        fprintf(stderr, "Wrong format");
        return 1;
    }
    picture.bytes = (unsigned char*)malloc(picture.size);
    if(picture.bytes == NULL){
        fprintf(stderr, "Memory allocation error");
        return 1;
    }
    size_t frez = fread(picture.bytes, 1, picture.size, file);
    if(frez != picture.size){
        fprintf(stderr, "read file error");
        return 1;
    }
    fclose(file);

    switch (atoi(argv[3])){
        case finversion:
            picture.inversion(&picture);
            break;
        case fhorizontal_mirror_ref:
            picture.horizontal_mirror_ref(&picture);
            break;
        case fvertical_mirror_ref:
            picture.vertical_mirror_ref(&picture);
            break;
        case fturn_clockwise:
            if (picture.turn_clockwise(&picture) == 1)
                return 1;
            break;
        case fturn_counterclockwise:
            if(picture.turn_counterclockwise(&picture) == 1)
                return 1;
            break;
        default:
            printf(stderr, "Wrong last arg");
            return 1;
    }

    FILE* new_file = fopen(argv[2], "wb");
    if(new_file == NULL){
        fprintf(stderr, "File open error");
        return 1;
    }
    rez = fprintf(new_file, "P%d\n%d %d\n%d\n", picture.version, picture.width, picture.height, picture.depth);
    if (rez != 15){
        fprintf(stderr, "Write file error");
        return 1;
    }
    frez = fwrite(picture.bytes, 1, picture.size, new_file);
    if (frez != picture.size){
        fprintf(stderr, "Write file error");
        return 1;
    }
    fclose(new_file);
    free(picture.bytes);
    return 0;
}
