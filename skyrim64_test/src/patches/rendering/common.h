#pragma once

#include "../../zydis/include/Zydis/Zydis.h"
#include "../../xbyak/xbyak.h"
#include "../../common.h"
#include "codegen.h"
#include "d3d11_tls.h"

extern thread_local char BSGraphics_TLSGlob[0x4000];