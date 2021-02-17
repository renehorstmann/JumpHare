#ifndef JUMPHARE_IO_H
#define JUMPHARE_IO_H

#include <stdbool.h>
#include "image.h"


Image *io_load_image(const char *file);

bool io_save_image(const Image *image, const char *file);

#endif //JUMPHARE_IO_H
