/*
 * Speed Dreams, a free and open source motorsport simulator.
 * Copyright (C) 2024 Xavier Del Campo Romero
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#define WIN32_LEAN_AND_MEAN
#include "portability.h"

#include <windows.h>
#include <winerror.h>
#include <shellapi.h>
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <cstdio>
#include <cstring>

static char *getdir(const char *path)
{
    /* SHFileOperation requires a double null-terminated string. */
    static const char end[] = {'\0', '\0'};
    size_t sz = strlen(path) + sizeof end;
    char *ret = static_cast<char *>(malloc(sz));

    if (!ret)
    {
        fprintf(stderr, "%s: malloc(3): %s\n", __func__, strerror(errno));
        goto failure;
    }

    strcpy(ret, path);
    ret[sz - 1] = '\0';
    return ret;

failure:
    free(ret);
    return nullptr;
}

int portability::rmdir_r(const char *path)
{
    int ret = -1, res;
    char *dir = getdir(path);
    SHFILEOPSTRUCT op = {0};

    if (!dir)
    {
        fprintf(stderr, "%s: getdir failed\n", __func__);
        goto end;
    }

    op.wFunc = FO_DELETE;
    op.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
    op.lpszProgressTitle = "";
    op.pFrom = dir;

    if ((res = SHFileOperation(&op)) && res != ERROR_FILE_NOT_FOUND)
    {
        fprintf(stderr, "%s: SHFileOperation failed with %#x\n", __func__, res);
        goto end;
    }
    else if (op.fAnyOperationsAborted)
    {
        fprintf(stderr, "%s: operation aborted\n", __func__);
        goto end;
    }

    ret = 0;

end:
    free(dir);
    return ret;
}
