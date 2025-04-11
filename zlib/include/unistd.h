#ifndef _UNISTD_H
#define _UNISTD_H

#include <io.h>       // Windows equivalent of unistd.h I/O functions
#include <process.h>  // For process-related functions like getpid()

// Define common POSIX functions/macros if missing
#define access _access
// #define write _write 
#define unlink _unlink

// Other missing definitions as needed
#endif // _UNISTD_H
