#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "RakHook/signal.hpp"
#include "RakHook/version_manager.hpp"

TEST_CASE("signal calls callbacks in order", "[signal]") {
    std::vector<int> order;

    rakhook::signal<void(int)> sig;
    sig += [&order](int value) {
        order.push_back(value);
    };
    sig.push_back([&order](int value) {
        order.push_back(value * 2);
    });

    sig.call(3);

    REQUIRE(order == std::vector<int>{3, 6});
}

TEST_CASE("signal stops on false for bool return type", "[signal]") {
    int calls = 0;

    rakhook::signal<bool()> sig;
    sig += [&calls] {
        ++calls;
        return true;
    };
    sig += [&calls] {
        ++calls;
        return false;
    };
    sig += [&calls] {
        ++calls;
        return true;
    };

    REQUIRE_FALSE(sig.call());
    REQUIRE(calls == 2);
}

TEST_CASE("empty bool signal returns true", "[signal]") {
    rakhook::signal<bool()> sig;
    REQUIRE(sig.call());
}

TEST_CASE("version_manager is invalid without samp.dll", "[version_manager]") {
    rakhook::version_manager versions;
    REQUIRE_FALSE(versions.valid());
    REQUIRE(versions.base() == 0);
}
