/**
 * @file test_lexer.cpp
 * @author cpp-love (15865418+cpp-love@user.noreply.gitee.com)
 * @brief lexer 的测试。
 * @version 0.1.0-1
 * @date 2026-11-12
 * 
 * @copyright cpp-love
 * 
 */

import std;
import lambda;

int main() {

    try {
        // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
        auto tokens_expected = std::vector{
            lambda::token{.value = lambda::token::left_parenthesis{}, .loc = {.begin = 0, .end = 1}},
            lambda::token{.value = lambda::token::lambda{}, .loc = {.begin = 1, .end = 2}},
            lambda::token{.value = lambda::token::str{.str = "a"}, .loc = {.begin = 2, .end = 3}},
            lambda::token{.value = lambda::token::dot{}, .loc = {.begin = 3, .end = 4}},
            lambda::token{.value = lambda::token::left_parenthesis{}, .loc = {.begin = 4, .end = 5}},
            lambda::token{.value = lambda::token::lambda{}, .loc = {.begin = 5, .end = 6}},
            lambda::token{.value = lambda::token::str{.str = "a"}, .loc = {.begin = 6, .end = 7}},
            lambda::token{.value = lambda::token::dot{}, .loc = {.begin = 7, .end = 8}},
            lambda::token{.value = lambda::token::lambda{}, .loc = {.begin = 8, .end = 9}},
            lambda::token{.value = lambda::token::str{.str = "b"}, .loc = {.begin = 9, .end = 10}},
            lambda::token{.value = lambda::token::dot{}, .loc = {.begin = 10, .end = 11}},
            lambda::token{.value = lambda::token::left_parenthesis{}, .loc = {.begin = 11, .end = 12}},
            lambda::token{.value = lambda::token::lambda{}, .loc = {.begin = 12, .end = 13}},
            lambda::token{.value = lambda::token::str{.str = "b"}, .loc = {.begin = 13, .end = 14}},
            lambda::token{.value = lambda::token::dot{}, .loc = {.begin = 14, .end = 15}},
            lambda::token{.value = lambda::token::str{.str = "b"}, .loc = {.begin = 15, .end = 16}},
            lambda::token{.value = lambda::token::right_parenthesis{}, .loc = {.begin = 16, .end = 17}},
            lambda::token{.value = lambda::token::str{.str = "a"}, .loc = {.begin = 17, .end = 18}},
            lambda::token{.value = lambda::token::right_parenthesis{}, .loc = {.begin = 18, .end = 19}},
            lambda::token{.value = lambda::token::str{.str = "a"}, .loc = {.begin = 19, .end = 20}},
            lambda::token{.value = lambda::token::right_parenthesis{}, .loc = {.begin = 20, .end = 21}},
            lambda::token{.value = lambda::token::semicolon{}, .loc = {.begin = 21, .end = 22}},
            lambda::token{.value = lambda::token::end{}, .loc = {.begin = 48, .end = 48}},
        };
        // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

        auto tokens = lambda::lex(R"guard((\a.(\a.\b.(\b.b)a)a);
# This is just a comment.)guard");

        if (!tokens.has_value()) {
            const auto &err = tokens.error();
            std::println("test_lexer: error: {} (location: [{}, {})).", err.what(), err.loc.begin,
                         err.loc.end);
            return 1;
        }

        if (tokens->size() != tokens_expected.size()) {
            std::println("test_lexer: error: The size of the parsed tokens is incorrect. (expected to "
                         "be {}, but it is actually {})",
                         tokens_expected.size(), tokens->size());
            return 1;
        }

        for (const auto &[token, token_expected] : std::views::zip(*tokens, tokens_expected)) {
            if (token.loc != token_expected.loc) {
                std::println(
                    "test_lexer: error: The location of a parsed token is incorrect. "
                    "(expected to be (location: [{}, {})), but it is actually (location: [{}, {})))",
                    token_expected.loc.begin, token_expected.loc.end, token.loc.begin, token.loc.end);
                return 1;
            }
            if (token.value.index() != token_expected.value.index()) {
                std::println("test_lexer: error: The type id of a parsed token is incorrect. (expected "
                             "to be {}, but it is actually {})",
                             token_expected.value.index(), token.value.index());
                return 1;
            }
            if (std::holds_alternative<lambda::token::str>(token.value)) {
                if (auto str = std::get<lambda::token::str>(token.value).str,
                    str_expected = std::get<lambda::token::str>(token_expected.value).str;
                    str != str_expected) {
                    std::println("test_lexer: error: The value of one of the parsed string tokens is "
                                 "incorrect. ({}, but expected to be {})",
                                 str, str_expected);
                    return 1;
                }
            }
        }

        std::println("test_lexer: ok.");
        return 0;
    } catch (std::exception &e) {
        try {
            std::println("An Exception throwed.\nwhat(): {}", e.what());
        } catch (std::exception &e) {
            return 3;
        }
        return 1;
    }
}
