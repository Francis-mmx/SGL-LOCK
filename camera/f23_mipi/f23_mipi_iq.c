

#include "asm/isp_alg.h"
#include "f23_mipi_iq.h"
static const u8 gamma_65_65_plus_poly_40_70_192_200[256] = {
    0,   5,   9,  13,  16,  18,  21,  23,  25,  28,  30,  32,  34,  35,  37,  39,
    41,  42,  44,  46,  47,  49,  51,  52,  54,  55,  57,  58,  59,  61,  62,  64,
    65,  66,  68,  69,  70,  72,  73,  74,  75,  77,  78,  79,  80,  82,  83,  84,
    85,  86,  87,  89,  90,  91,  92,  93,  94,  95,  96,  98,  99, 100, 101, 102,
    103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118,
    119, 120, 121, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 132,
    133, 134, 135, 136, 137, 138, 138, 139, 140, 141, 142, 143, 143, 144, 145, 146,
    146, 147, 148, 149, 149, 150, 151, 152, 152, 153, 154, 155, 155, 156, 157, 157,
    158, 159, 159, 160, 161, 162, 162, 163, 164, 164, 165, 166, 166, 167, 168, 168,
    169, 169, 170, 171, 171, 172, 173, 173, 174, 175, 175, 176, 177, 177, 178, 178,
    179, 180, 180, 181, 182, 182, 183, 184, 184, 185, 185, 186, 187, 187, 188, 189,
    189, 190, 191, 191, 192, 193, 193, 194, 195, 195, 196, 197, 197, 198, 199, 199,
    200, 201, 201, 202, 203, 203, 204, 205, 206, 206, 207, 208, 209, 209, 210, 211,
    212, 212, 213, 214, 215, 215, 216, 217, 218, 219, 219, 220, 221, 222, 223, 224,
    224, 225, 226, 227, 228, 229, 230, 231, 232, 232, 233, 234, 235, 236, 237, 238,
    239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 251, 252, 253, 254, 255
};

static isp_iq_params_t F23_mipi_iq_params = {
    .blc = {
        0, 0, 0, 0,
    },
    .lsc = {
        0, 0, 0, 0, 0, 0, 0, 0,
    },

    .adj = {
        0x100, 0x200, 0x200, 0, 0, 0,
    },

    .gamma = NULL,//&gamma_65_65_plus_poly_40_70_192_200,

    .ccm = {
        0x80, 0, 0, 0,
        0, 0x80, 0, 0,
        0, 0, 0x80, 0,
    },

    .dpc =   {(1 << 4), (1 << 4), (15 << 4)},
    .tnr =   {1, 32, 175, 25, 175, 25, 180, 0, {256, 256, 256, 256, 256, 256, 230, 230}},
    .nr =    {30, 8, 0},
    .shp =   {1, 15, 64, 64, 160, 160, 96, 96, 0, 8, 192, 8, 192},
    .cnr =   {1, 255, 30},

    .md_wms = {100, 130, 180, 250, 400},
    .md_level = 3,
};

void *F23_mipi_get_iq_params()
{
    return (void *)&F23_mipi_iq_params;
}
