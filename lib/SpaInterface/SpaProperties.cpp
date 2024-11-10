#include "SpaProperties.h"

template<typename T>
std::function<bool(const char*, T)> Property<T>::_sendCallback = nullptr;
template<>
std::function<bool(const char*, String)> Property<String>::_sendCallback = nullptr;
template<>
std::function<bool(const char*, long)> Property<long>::_sendCallback = nullptr;

std::function<bool(const char*, int)> Property<int>::_sendCallback = nullptr;
std::function<bool(const char*, bool)> Property<bool>::_sendCallback = nullptr;
