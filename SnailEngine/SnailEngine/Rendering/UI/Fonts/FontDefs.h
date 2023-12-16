#pragma once

struct Character {
    int codePoint, x, y, width, height, originX, originY;
};

struct FontData {
    const char* name;
    int size, bold, italic, width, height, characterCount;
    Character* characters;
};