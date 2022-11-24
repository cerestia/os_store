#ifndef ONIX_STDARG_H
#define ONIX_STDARG_H

typedef char* va_list;
//ap 为指向参数的指针
//v表示传入的参数个数，必须是函数的第一个参数
//下一个地址为第一个可变参数地址
#define va_start(ap,v) (ap=(va_list)&v + sizeof(char*))
//t表示传入参数的类型
//
#define va_arg(ap,t) (*(t*)((ap+=sizeof(char*))-sizeof(char*)))

#define va_end(ap) (ap=(va_list)0)

#endif