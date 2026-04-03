#include "../include/page.h"

// int Page::getKey() const { return this->key; }
// int Page::getValue() const { return this->value; }
void Page::set_value(int val) {this->value = val;}
void Page::set_key(int key) {this->key = key;}
char* Page::get_data() { return &this->data; }
