/** Handle for a file in memory */
struct BinaryFileHandle 
{
    uint64  size    = 0;    // size of file in memory
    void*   memory  = NULL; // pointer to file in memory
};

/** Handle for an UNSIGNED BYTE bitmap in memory */
struct BitmapHandle
{
    uint32          width       = 0;    // image width
    uint32          height      = 0;    // image height
    uint64          size        = 0;    // size of the bitmap in memory
    uint8           bit_depth   = 0;    // bit depth of bitmap in bytes (e.g. bit depth = 3 means there are 3 bytes in the bitmap per pixel)
    unsigned char*  memory      = NULL; // pointer to UNSIGNED BYTE bitmap in memory
};