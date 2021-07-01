#pragma once

#include "../gamedefine.h"

/**
    Types to hold data in memory
*/

/** Handle for a file in memory */
struct binary_file_handle_t
{
    uint64  size    = 0;        // size of file in memory
    void*   memory  = nullptr;  // pointer to file in memory
};

/** Handle for an UNSIGNED BYTE bitmap in memory */
struct bitmap_handle_t : binary_file_handle_t
{
    uint32  width = 0;      // image width
    uint32  height = 0;     // image height
    uint8   bit_depth = 0;  // bit depth of bitmap in bytes (e.g. bit depth = 3 means there are 3 bytes in the bitmap per pixel)
};
