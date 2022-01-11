#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define SVDEF static inline
#define SV_IMPLEMENTATION
#include <sv.h>

#define BUFFERDEF static inline
#define BUFFER_IMPLEMENTATION
#include <buffer.h>

#include "util.h"
#include "../lopsinvm/lopsinvm.h"
