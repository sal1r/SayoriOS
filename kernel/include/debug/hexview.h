#pragma once

void hexview_advanced(char* buffer, size_t length, size_t width, bool relative, void (*printer_func)(const char *, ...));