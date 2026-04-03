#ifndef FRAME_H
#define FRAME_H

#include "page.h"
class Frame {
public:
    Frame() = default;
    ~Frame() = default;
    bool get_dirty();
    int get_pin_count();
    Page* get_page();
private:
    Page* page;
    bool is_dirty;
    int pin_count;
};

#endif //FRAME_H
