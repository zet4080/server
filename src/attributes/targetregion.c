/*
Copyright (c) 1998-2015, Enno Rehling Rehling <enno@eressea.de>
Katja Zedel <katze@felidae.kn-bremen.de
Christian Schlittchen <corwin@amber.kn-bremen.de>

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
**/

#include <platform.h>
#include "targetregion.h"

#include <kernel/config.h>
#include <kernel/region.h>
#include <kernel/save.h>
#include <kernel/version.h>

#include <util/attrib.h>
#include <util/resolve.h>

#include <storage.h>

static void
write_targetregion(const attrib * a, const void *owner, struct storage *store)
{
    write_region_reference((region *)a->data.v, store);
}

static int read_targetregion(attrib * a, void *owner, struct storage *store)
{
    int result =
        read_reference(&a->data.v, store, read_region_reference,
        RESOLVE_REGION(global.data_version));
    if (result == 0 && !a->data.v)
        return AT_READ_FAIL;
    return AT_READ_OK;
}

attrib_type at_targetregion = {
    "targetregion",
    NULL,
    NULL,
    NULL,
    write_targetregion,
    read_targetregion,
    ATF_UNIQUE
};

attrib *make_targetregion(struct region * r)
{
    attrib *a = a_new(&at_targetregion);
    a->data.v = r;
    return a;
}
