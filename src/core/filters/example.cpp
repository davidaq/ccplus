#include "filter.hpp"

CCPLUS_FILTER(Example) {
    printf("PASS!\n");
    dest->getData();
}