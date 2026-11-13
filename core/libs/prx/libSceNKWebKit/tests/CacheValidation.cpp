#include "prx/libc/include/general/VabiMacros.hpp"
#include "prx/libSceNKWebKit/libSceNKWebKit.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <type_traits>

namespace {

using NKWebKit::HttpAbi::HTTPHeaderName;
using NKWebKit::WebCoreAbi::ResourceResponseBase;
using NKWebKit::WtfAbi::String;

using ComputeCurrentAge = WTF::Seconds (APS5_VABI *)(const ResourceResponseBase*, WTF::WallTime);
static_assert(std::is_same_v<decltype(&_ZN7WebCore17computeCurrentAgeERKNS_16ResourceResponseEN3WTF8WallTimeE), ComputeCurrentAge>);

constexpr std::string_view DateText = "Sun, 06 Nov 1994 08:49:37 GMT";
constexpr double DateSeconds = 784111777.0;
constexpr double Infinity = std::numeric_limits<double>::infinity();
constexpr double NotANumber = std::numeric_limits<double>::quiet_NaN();
constexpr int ClockReadingAttempts = 3;

void Check(bool value, int line) {
    if (!value) {
        std::fprintf(stderr, "CacheValidation check failed at line %d\n", line);
        std::abort();
    }
}
#define Require(value) Check((value), __LINE__)

String Text(std::string_view value) {
    return _ZN3WTF10StringImpl6createEPKhj(reinterpret_cast<const unsigned char*>(value.data()), static_cast<unsigned>(value.size()));
}

String Text(std::u16string_view value) {
    return _ZN3WTF10StringImpl6createEPKDsj(value.data(), static_cast<unsigned>(value.size()));
}

template<class TEntry>
void ReleaseHeaders(NKWebKit::HttpAbi::HeaderVector<TEntry>& entries) {
    if (entries.buffer) {
        std::destroy_n(entries.buffer, entries.size);
        _ZN3WTF8fastFreeEPv(entries.buffer);
    }
}

struct ResponseFixture {
    ResourceResponseBase response{};

    ~ResponseFixture() {
        ReleaseHeaders(response.httpHeaderFields.common);
        ReleaseHeaders(response.httpHeaderFields.uncommon);
    }

    void Set(HTTPHeaderName header, const String& value) {
        _ZN7WebCore20ResourceResponseBase18setHTTPHeaderFieldENS_14HTTPHeaderNameERKN3WTF6StringE(&response, header, value);
    }
};

double Now() {
    return _ZN3WTF8WallTime3nowEv().m_value;
}

double CurrentAge(const ResponseFixture& fixture, double responseTime) {
    return _ZN7WebCore17computeCurrentAgeERKNS_16ResourceResponseEN3WTF8WallTimeE(&fixture.response, WTF::WallTime{responseTime}).m_value;
}

void ExpectInitialAge(const ResponseFixture& fixture, double responseTime, double correctedInitialAge, int line) {
    for (int attempt = 0; attempt < ClockReadingAttempts; ++attempt) {
        const double before = Now();
        const double age = CurrentAge(fixture, responseTime);
        const double after = Now();
        if (age >= correctedInitialAge + (before - responseTime) && age <= correctedInitialAge + (after - responseTime)) return;
        std::fprintf(stderr, "CacheValidation line %d attempt %d: age %.9f outside [%.9f, %.9f]\n", line, attempt,
                     age, correctedInitialAge + (before - responseTime), correctedInitialAge + (after - responseTime));
    }
    std::abort();
}
#define RequireInitialAge(fixture, responseTime, correctedInitialAge) ExpectInitialAge((fixture), (responseTime), (correctedInitialAge), __LINE__)

void TestWithoutHeaders() {
    ResponseFixture fixture;
    const double now = Now();
    RequireInitialAge(fixture, now - 30, 0);
    RequireInitialAge(fixture, now, 0);
    RequireInitialAge(fixture, now + 1000, 0);
    RequireInitialAge(fixture, 0, 0);
    std::puts("computeCurrentAge without Date and Age: ok");
}

void TestAgeOnly() {
    ResponseFixture fixture;
    const double now = Now();
    fixture.Set(HTTPHeaderName::Age, Text("120"));
    RequireInitialAge(fixture, now - 30, 120);
    fixture.Set(HTTPHeaderName::Age, Text(u"1.5"));
    RequireInitialAge(fixture, now - 30, 1.5);
    fixture.Set(HTTPHeaderName::Age, Text("-2.5"));
    RequireInitialAge(fixture, now - 30, 0);
    fixture.Set(HTTPHeaderName::Age, Text("10x"));
    RequireInitialAge(fixture, now - 30, 0);
    std::puts("computeCurrentAge with Age only: ok");
}

void TestApparentAge() {
    ResponseFixture fixture;
    fixture.Set(HTTPHeaderName::Date, Text(DateText));
    const auto date = _ZNK7WebCore20ResourceResponseBase4dateEv(&fixture.response);
    Require(date.hasValue && date.seconds == DateSeconds);
    RequireInitialAge(fixture, DateSeconds + 50, 50);
    RequireInitialAge(fixture, DateSeconds, 0);
    RequireInitialAge(fixture, DateSeconds + 0.25, 0.25);
    fixture.Set(HTTPHeaderName::Age, Text("20"));
    RequireInitialAge(fixture, DateSeconds + 50, 50);
    fixture.Set(HTTPHeaderName::Age, Text("50"));
    RequireInitialAge(fixture, DateSeconds + 50, 50);
    fixture.Set(HTTPHeaderName::Age, Text(u"80"));
    RequireInitialAge(fixture, DateSeconds + 50, 80);
    fixture.Set(HTTPHeaderName::Age, Text("-20"));
    RequireInitialAge(fixture, DateSeconds + 50, 50);
    std::puts("computeCurrentAge with Age below, equal to and above the apparent age: ok");
}

void TestDateAfterResponseTime() {
    ResponseFixture fixture;
    fixture.Set(HTTPHeaderName::Date, Text(DateText));
    RequireInitialAge(fixture, DateSeconds - 50, 0);
    fixture.Set(HTTPHeaderName::Age, Text("7"));
    RequireInitialAge(fixture, DateSeconds - 50, 7);
    fixture.Set(HTTPHeaderName::Age, Text("-20"));
    RequireInitialAge(fixture, DateSeconds - 50, 0);
    std::puts("computeCurrentAge with Date after the response time: ok");
}

void TestInvalidDate() {
    ResponseFixture fixture;
    fixture.Set(HTTPHeaderName::Date, Text("invalid"));
    Require(!_ZNK7WebCore20ResourceResponseBase4dateEv(&fixture.response).hasValue);
    RequireInitialAge(fixture, DateSeconds + 50, 0);
    fixture.Set(HTTPHeaderName::Age, Text("9"));
    RequireInitialAge(fixture, DateSeconds + 50, 9);
    std::puts("computeCurrentAge with an unparsable Date: ok");
}

void TestNonFiniteResponseTime() {
    ResponseFixture fixture;
    Require(std::isnan(CurrentAge(fixture, NotANumber)));
    Require(CurrentAge(fixture, Infinity) == -Infinity);
    Require(CurrentAge(fixture, -Infinity) == Infinity);
    fixture.Set(HTTPHeaderName::Date, Text(DateText));
    fixture.Set(HTTPHeaderName::Age, Text("80"));
    Require(std::isnan(CurrentAge(fixture, NotANumber)));
    Require(std::isnan(CurrentAge(fixture, Infinity)));
    Require(CurrentAge(fixture, -Infinity) == Infinity);
    std::puts("computeCurrentAge with a non-finite response time: ok");
}

void TestNullResponse() {
    bool thrown = false;
    try {
        _ZN7WebCore17computeCurrentAgeERKNS_16ResourceResponseEN3WTF8WallTimeE(nullptr, WTF::WallTime{DateSeconds});
    } catch (const std::invalid_argument&) {
        thrown = true;
    }
    Require(thrown);
    std::puts("computeCurrentAge with a null response: ok");
}

}

int main() {
    TestWithoutHeaders();
    TestAgeOnly();
    TestApparentAge();
    TestDateAfterResponseTime();
    TestInvalidDate();
    TestNonFiniteResponseTime();
    TestNullResponse();
    return 0;
}
