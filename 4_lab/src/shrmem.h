//
// Created by Temi4 on 17.10.2021.
//

#ifndef SRC__SHRMEM_H_
#define SRC__SHRMEM_H_

#include <fcntl.h>

const char *BackingFile = "os_lab4.back";
const char *SemaphoreName = "os_lab4.semaphore";
unsigned AccessPerms = S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH;

#endif//SRC__SHRMEM_H_
