#ifndef ICONS_H
#define ICONS_H

// Vscode is whining about.. something - idk.
const unsigned char kClockIcon[] PROGMEM = {
    0xe0, 0x01, 0x18, 0x06, 0x04, 0x08, 0x42, 0x10, 0x42, 0x10,
    0x41, 0x20, 0x41, 0x20, 0xc1, 0x23, 0x01, 0x20, 0x02, 0x10,
    0x02, 0x10, 0x04, 0x08, 0x18, 0x06, 0xe0, 0x01};

const unsigned char kPlaceholderIcon[] PROGMEM = {
    0xfe, 0x1f, 0x03, 0x30, 0x09, 0x24, 0x01, 0x20, 0x01, 0x20,
    0xe1, 0x21, 0x11, 0x22, 0x11, 0x22, 0xe1, 0x21, 0x01, 0x20,
    0x01, 0x20, 0x01, 0x20, 0x03, 0x30, 0xfe, 0x1f};

const unsigned char kSliderIcon[] PROGMEM = {
   0x00, 0x00, 0x88, 0x00, 0xd8, 0x00, 0xf8, 0x00, 0x70, 0x00, 0x21, 0x20,
   0x03, 0x30, 0xff, 0x3f, 0x03, 0x30, 0x57, 0x3a, 0x12, 0x2a, 0x56, 0x3a,
   0x54, 0x1b, 0x56, 0x2b };

// Square Icon Bitmap (16x16)
const unsigned char kSquareIcon[] PROGMEM = {
    0x00, 0x00, 0x3f, 0xfc, 0x5f, 0xfa, 0x6f, 0xf6, 0x77, 0xee, 0x7b,
    0xde, 0x7c, 0x3e, 0x7c, 0x3e, 0x7c, 0x3e, 0x7c, 0x3e, 0x7b, 0xde,
    0x77, 0xee, 0x6f, 0xf6, 0x5f, 0xfa, 0x3f, 0xfc, 0x00, 0x00};

#endif // ICONS_H