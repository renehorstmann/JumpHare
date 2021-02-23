#ifndef JUMPHARE_IO_H
#define JUMPHARE_IO_H

#include <stdbool.h>
#include "image.h"


Image *io_load_image(const char *file, int layers);

bool io_save_image(const char *file, const Image *image);

#endif //JUMPHARE_IO_H
