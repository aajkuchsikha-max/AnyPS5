#pragma once

#include "SceTypes.hpp"
#include "prx/libSceNKWebKit/src/ResourceResponseAbi.hpp"
#include "prx/libSceNKWebKit/src/WtfTimeTypes.hpp"

extern "C" {

WTF::Seconds APS5_VABI _ZN7WebCore17computeCurrentAgeERKNS_16ResourceResponseEN3WTF8WallTimeE(const NKWebKit::WebCoreAbi::ResourceResponseBase* response, WTF::WallTime responseTime);

}
