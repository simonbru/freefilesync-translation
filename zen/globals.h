// *****************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under    *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0           *
// * Copyright (C) Zenju (zenju AT freefilesync DOT org) - All Rights Reserved *
// *****************************************************************************

#ifndef GLOBALS_H_8013740213748021573485
#define GLOBALS_H_8013740213748021573485

#include <atomic>
#include <memory>
#include "scope_guard.h"

namespace zen
{
//solve static destruction order fiasco by providing scoped ownership and serialized access to global variables
template <class T>
class Global
{
public:
	Global() {}
	explicit Global(std::unique_ptr<T>&& newInst) { set(std::move(newInst)); }
	~Global() { set(nullptr); }
	
    std::shared_ptr<T> get() //=> return std::shared_ptr to let instance life time be handled by caller (MT usage!)
    {
        while (spinLock.exchange(true)) ;
        ZEN_ON_SCOPE_EXIT(spinLock = false);
        if (inst)
            return *inst;
        return nullptr;
    }

    void set(std::unique_ptr<T>&& newInst)
    {
        std::shared_ptr<T>* tmpInst = nullptr;
        if (newInst)
            tmpInst = new std::shared_ptr<T>(std::move(newInst));
        {
            while (spinLock.exchange(true)) ;
            ZEN_ON_SCOPE_EXIT(spinLock = false);
            std::swap(inst, tmpInst);
        }
        delete tmpInst;
    }

private:
    //avoid static destruction order fiasco: there may be accesses to "Global<T>::get()" during process shutdown
    //e.g. show message in debug_minidump.cpp or some detached thread assembling an error message!
    //=> use trivially-destructible POD only!!!
    std::shared_ptr<T>* inst = nullptr;
    //serialize access: can't use std::mutex because of non-trival destructor
    std::atomic<bool> spinLock { false };
};
}

#endif //GLOBALS_H_8013740213748021573485
