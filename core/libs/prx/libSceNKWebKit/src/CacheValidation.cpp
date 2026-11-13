// Derived from Source/WebCore/platform/network/CacheValidation.cpp (PS5 WebKit 11.x 80f77e94c63e)
/*
 * Copyright (C) 2014-2017 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "prx/libSceNKWebKit/include/CacheValidationExports.hpp"
#include "prx/libSceNKWebKit/include/ResourceResponseExports.hpp"
#include "prx/libSceNKWebKit/include/WtfExports.hpp"

#include <algorithm>
#include <stdexcept>
#include <type_traits>

static_assert(sizeof(WTF::WallTime) == sizeof(double));
static_assert(sizeof(WTF::Seconds) == sizeof(double));
static_assert(std::is_trivially_copyable_v<WTF::WallTime>);
static_assert(std::is_trivially_copyable_v<WTF::Seconds>);

extern "C" {

WTF::Seconds APS5_VABI _ZN7WebCore17computeCurrentAgeERKNS_16ResourceResponseEN3WTF8WallTimeE(const NKWebKit::WebCoreAbi::ResourceResponseBase* response, WTF::WallTime responseTime) {
    if (!response) throw std::invalid_argument("null WebCore::ResourceResponse");
    const auto date = _ZNK7WebCore20ResourceResponseBase4dateEv(response);
    const double apparentAge = date.hasValue ? std::max(0.0, responseTime.m_value - date.seconds) : 0.0;
    const auto age = _ZNK7WebCore20ResourceResponseBase3ageEv(response);
    const double correctedInitialAge = std::max(apparentAge, age.hasValue ? age.seconds : 0.0);
    const double residentTime = _ZN3WTF8WallTime3nowEv().m_value - responseTime.m_value;
    return WTF::Seconds{correctedInitialAge + residentTime};
}

}
