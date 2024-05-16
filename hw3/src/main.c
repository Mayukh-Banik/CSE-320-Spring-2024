#include <stdio.h>
#include "sfmm.h"
#include "debug.h"

int main(int argc, char const *argv[]) {
    // sf_set_magic(0);
    // sf_malloc(32704);


    // sf_show_heap();
    // sf_malloc(1);
    // debug("%f", sf_utilization());
    sf_malloc(32704);
    sf_show_heap();
    return EXIT_SUCCESS;
}
