/*
 * Use with http://lcamtuf.coredump.cx/afl/
 */
#include <stdio.h>
#include <stdlib.h>
#include "../lib/libunshield.h"

#ifdef HAVE_CONFIG_H

#include "lib/unshield_config.h"

#endif

/*
#ifndef __AFL_LOOP
static int _count = 1;
#define __AFL_LOOP(x) (_count--)
#endif
*/
static bool extract_file(Unshield *unshield, const char *prefix, int index) {
    return unshield_file_save(unshield, index, NULL);
}

static int extract_helper(Unshield *unshield, const char *prefix, int first, int last)/*{{{*/
{
    int i;
    int count = 0;

    for (i = first; i <= last; i++) {
        if (unshield_file_is_valid(unshield, i)
            && extract_file(unshield, prefix, i))
            count++;
    }

    return count;
}/*}}}*/


int main(int argc, char *const argv[]) {
    Unshield *unshield = NULL;

    while (__AFL_LOOP(1000)) {
        unshield = unshield_open_force_version(argv[1], -1);
        if (!unshield) {
            exit(1);
        }

        int i;

        for (i = 0; i < unshield_file_group_count(unshield); i++) {
            UnshieldFileGroup *file_group = unshield_file_group_get(unshield, i);
            if (file_group)
                extract_helper(unshield, file_group->name, file_group->first_file, file_group->last_file);
        }
        unshield_close(unshield);
    }

    return 0;
}