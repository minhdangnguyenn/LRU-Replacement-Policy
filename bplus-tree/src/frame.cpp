#include "../include/frame.h"

bool Frame::get_dirty() { return this->is_dirty; }
int Frame::get_pin_count() { return this->pin_count; }
Page* Frame::get_page() { return this->page; }
