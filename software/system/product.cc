#include <string.h>

#include "flash.h"
#include "versions.h"
#include "product.h"
#include "small_printf.h"
#include "u64.h"

// Longest supported product_name string is 16. Update below functions and callers if this increases!

static char *product_name[] = {
    "Ultimate",  // Default if unknown product
    "Ultimate II",
    "Ultimate II+",
    "Ultimate II+L",
    "Ultimate 64",
    "Ultimate 64E",
    "Ultimate 64E2",
};

#if U64
const char *getBoardRevision(void)
{
    uint8_t rev = (U2PIO_BOARDREV >> 3);

    switch (rev) {
    case 0x10:
        return "U64 Prototype";
    case 0x11:
        return "U64 V1.1 (Null Series)";
    case 0x12:
        return "U64 V1.2 (Mass Prod)";
    case 0x13:
        return "U64 V1.3 (Elite)";
    case 0x14:
        return "U64 V1.4 (Std/Elite)";
    case 0x15:
        return "U64E V2.0 (Early Proto)";
    case 0x16:
        return "U64E V2.1 (Null Series)";
    case 0x17:
        return "U64E V2.2 (Mass Prod)";
    }
    return "Unknown";
}

bool isEliteBoard(void)
{
    uint8_t rev = (U2PIO_BOARDREV >> 3);
    if (rev == 0x13) {
        return true;
    }
    if (rev == 0x14) { // may be either!
        uint8_t joyswap = C64_PLD_JOYCTRL;
        if (joyswap & 0x80) {
            return true;
        }
        return false;
    }
    return false;
}
#endif

uint8_t getProductId() {
    return
#if U64 == 1
    isEliteBoard() ? PRODUCT_ULTIMATE_64_ELITE : PRODUCT_ULTIMATE_64
#elif U64 == 2
    PRODUCT_ULTIMATE_64_ELITE_II
#elif U2P == 1
    PRODUCT_ULTIMATE_II_PLUS
#elif U2P == 2
    PRODUCT_ULTIMATE_II_PLUS_L
#else
    PRODUCT_ULTIMATE_II
#endif
    ;
}

const char *getProductString() {
    uint8_t product = getProductId();
    if (product >= sizeof(product_name) / sizeof(char *))
        product = 0;
    return  product_name[product];
}

char *getProductVersionString(char *buf, int sz) {
    uint8_t fpga_version;
    const char *format;

    // Required size is max of all fields:
    //
    //   7 fixed non-%-format-chars, 16 product, 2 fpga_version, 5 APPL_VERSION, 1 NULL-byte
    if (sz < 7 + 16 + 2 + 5 + 1) {
        return NULL;
    }

    // FIXME: Should order of APPL vs fpga version be unified across products?
    switch(getProductId()) {
        case PRODUCT_ULTIMATE_II:
        case PRODUCT_ULTIMATE_II_PLUS:
        case PRODUCT_ULTIMATE_II_PLUS_L:
            format = "%s %s (1%b)";
            fpga_version = getFpgaVersion();
            sprintf(buf, format, getProductString(), APPL_VERSION, fpga_version);
            break;
        case PRODUCT_ULTIMATE_64:
        case PRODUCT_ULTIMATE_64_ELITE:
        case PRODUCT_ULTIMATE_64_ELITE_II:
            format = "%s (V1.%b) %s";
            fpga_version = C64_CORE_VERSION;
            sprintf(buf, format, getProductString(), fpga_version, APPL_VERSION);
            break;
        default:
            strcpy(buf, "Unknown product");
    }
    return buf;
}

char *getProductTitleString(char *buf, int sz)
{
    char product_version[41];

    if (sz < 17)
        return NULL;
    if (sz < sizeof(product_version) || !getProductVersionString(product_version, sizeof(product_version))) {
       strcpy(buf, "*** Ultimate ***");
       return buf;
    }

    // Add as many stars around the title as space will allow
    int space = 40 - strlen(product_version);
    char *format = "%s";
    if (space >= 8) format = "*** %s ***";
    else if (space >= 6) format = "** %s **";
    else if (space >= 4) format = "* %s *";
    sprintf(buf, format, product_version);
    return buf;
}

char *getProductUpdateFileExtension() {
    return
#if U64 == 1
    "U64"
#elif U64 == 2
    "UE2"
#elif U2P == 1
    "U2P"
#elif U2P == 2
    "U2L"
#else

#ifdef RISCV
    "U2R"
#else
    "U2U"
#endif

#endif
    ;
}
