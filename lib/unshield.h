/* $Id$ */
#ifndef __unshield_h__
#define __unshield_h__

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
 
typedef struct _Unshield Unshield;

/*
   Open/close functions
 */

Unshield* unshield_open(const char* filename);
void unshield_close(Unshield* unshield);

/*
   Component functions
 */

int         unshield_component_count    (Unshield* unshield);
const char* unshield_component_name     (Unshield* unshield, int index);

/*
   File group functions
 */

int         unshield_file_group_count   (Unshield* unshield);
const char* unshield_file_group_name    (Unshield* unshield, int index);

/*
   Directory functions
 */

int         unshield_directory_count    (Unshield* unshield);
const char* unshield_directory_name     (Unshield* unshield, int index);

/*
   File functions
 */

int         unshield_file_count         (Unshield* unshield);
const char* unshield_file_name          (Unshield* unshield, int index);
bool        unshield_file_is_valid      (Unshield* unshield, int index);
bool        unshield_file_save          (Unshield* unshield, int index, const char* filename);
int         unshield_file_directory     (Unshield* unshield, int index);

#ifdef __cplusplus
}
#endif

  
#endif

