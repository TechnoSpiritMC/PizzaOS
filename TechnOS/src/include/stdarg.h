#pragma once

typedef char* va_list;

#define va_start(ap, last) \
(ap = (char*)&(last) + sizeof(last))

#define va_arg(ap, type) \
(*(type*)((ap += sizeof(type)) - sizeof(type)))

#define va_end(ap) (ap = (va_list)0)
