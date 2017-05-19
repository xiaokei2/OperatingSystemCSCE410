/*
     File        : debug.H

     Author      : Dilma Da Silva
     Modified    : 09/24/15

     Description : setting debug output
*/

#ifndef _debug_h_
#define _debug_h_

#include "utils.H"

#define DEBUG

#ifdef DEBUG

#define DEBUG_OUT( m) Console::puts( m )
#define DEBUG_E9( m ) debug_out_E9( m )

#else

#define DEBUG_OUT( m ) 
#define DEBUG_E9 ( m )

#endif // #ifdef DEBUG

#endif // #ifndef _debug_h_
