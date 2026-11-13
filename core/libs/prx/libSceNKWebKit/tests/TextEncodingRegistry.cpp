#include "prx/libSceNKWebKit/libSceNKWebKit.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <source_location>
#include <type_traits>

namespace {

using NKWebKit::WtfAbi::String;

void Require(bool condition, std::source_location location = std::source_location::current()) {
    if (!condition) {
        std::fprintf(stderr, "WebCore::defaultTextEncodingNameForSystemLanguage check failed at line %u\n", location.line());
        std::abort();
    }
}

using DefaultTextEncodingName = String (APS5_VABI *)();
static_assert(std::is_same_v<decltype(&_ZN7WebCore40defaultTextEncodingNameForSystemLanguageEv), DefaultTextEncodingName>);
static_assert(!std::is_trivially_copyable_v<String>);

constexpr unsigned char ExpectedName[] = "ISO-8859-1";
constexpr unsigned ExpectedLength = sizeof(ExpectedName) - 1;

void RequireName(const String& name) {
    Require(name.impl != nullptr);
    Require(name.impl->length == ExpectedLength);
    Require(name.impl->hashAndFlags == 4);
    Require(name.impl->data != nullptr);
    Require(std::memcmp(name.impl->data, ExpectedName, ExpectedLength) == 0);
}

void TestName() {
    const String name = _ZN7WebCore40defaultTextEncodingNameForSystemLanguageEv();
    RequireName(name);
    Require(name.impl->refCount == 2);
    Require(_ZN3WTF5equalEPKNS_10StringImplEPKh(name.impl, ExpectedName));
    std::printf("WebCore::defaultTextEncodingNameForSystemLanguage name: ISO-8859-1 as a singly referenced 8-bit string passed\n");
}

void TestEachCallCreatesAString() {
    String first = _ZN7WebCore40defaultTextEncodingNameForSystemLanguageEv();
    const String second = _ZN7WebCore40defaultTextEncodingNameForSystemLanguageEv();
    RequireName(first);
    RequireName(second);
    Require(first.impl != second.impl);
    Require(first.impl->refCount == 2);
    Require(second.impl->refCount == 2);
    {
        const String copy = first;
        Require(copy.impl == first.impl);
        Require(first.impl->refCount == 4);
        Require(second.impl->refCount == 2);
    }
    Require(first.impl->refCount == 2);
    first = String();
    Require(first.impl == nullptr);
    RequireName(second);
    Require(second.impl->refCount == 2);
    std::printf("WebCore::defaultTextEncodingNameForSystemLanguage references: separate string per call and independent counts passed\n");
}

void TestRepeatedCalls() {
    for (unsigned call = 0; call < 4096; ++call) {
        const String name = _ZN7WebCore40defaultTextEncodingNameForSystemLanguageEv();
        RequireName(name);
        Require(name.impl->refCount == 2);
    }
    std::printf("WebCore::defaultTextEncodingNameForSystemLanguage repeated calls: 4096 created and released strings passed\n");
}

}

int main() {
    TestName();
    TestEachCallCreatesAString();
    TestRepeatedCalls();
}
