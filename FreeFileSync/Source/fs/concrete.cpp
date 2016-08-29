// *****************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under    *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0           *
// * Copyright (C) Zenju (zenju AT freefilesync DOT org) - All Rights Reserved *
// *****************************************************************************

#include "concrete.h"
#include "native.h"
#ifdef ZEN_WIN_VISTA_AND_LATER
    #include "mtp.h"
    #include "sftp.h"
#endif

using namespace zen;


AbstractPath zen::createAbstractPath(const Zstring& itemPathPhrase) //noexcept
{
    //greedy: try native evaluation first
    if (acceptsItemPathPhraseNative(itemPathPhrase)) //noexcept
        return createItemPathNative(itemPathPhrase); //noexcept

    //then the rest:
#ifdef ZEN_WIN_VISTA_AND_LATER
    if (acceptsItemPathPhraseMtp(itemPathPhrase)) //noexcept
        return createItemPathMtp(itemPathPhrase); //noexcept

    if (acceptsItemPathPhraseSftp(itemPathPhrase)) //noexcept
        return createItemPathSftp(itemPathPhrase); //noexcept
#endif

    //no idea? => native!
    return createItemPathNative(itemPathPhrase);
}
