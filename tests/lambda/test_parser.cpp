/**
 * @file test_parser.cpp
 * @author cpp-love (15865418+cpp-love@user.noreply.gitee.com)
 * @brief parser 的测试。
 * @version 0.1.0-1
 * @date 2026-11-14
 * 
 * @copyright cpp-love
 * 
 */

import std;
import lambda;

std::optional<std::string> is_equal(const lambda::program_ast::expr_ptr &lhs,
                                    const lambda::program_ast::expr_ptr &rhs) {
    using namespace lambda::program_ast;

    return std::visit(
        lambda::overloaded{
            [](const abstraction &lhs, const abstraction &rhs) -> std::optional<std::string> {
                if (lhs.loc != rhs.loc) {
                    return std::format(
                        "abstraction location differs (actual: [{}, {}), expected: [{}, {}))",
                        lhs.loc.begin, lhs.loc.end, rhs.loc.begin, rhs.loc.end);
                }
                if (lhs.param.loc != rhs.param.loc) {
                    return std::format(
                        "parameter location differs (actual: [{}, {}), expected: [{}, {}))",
                        lhs.param.loc.begin, lhs.param.loc.end, rhs.param.loc.begin, rhs.param.loc.end);
                }
                if (lhs.param.var != rhs.param.var) {
                    return std::format("parameter name differs (actual: '{}', expected: '{}')",
                                       lhs.param.var, rhs.param.var);
                }
                if (auto mismatch = is_equal(lhs.body, rhs.body); mismatch) {
                    return std::format("abstraction body: {}", *mismatch);
                }
                return std::nullopt;
            },
            [](const var_ref &lhs, const var_ref &rhs) -> std::optional<std::string> {
                if (lhs.loc != rhs.loc) {
                    return std::format(
                        "variable reference location differs (actual: [{}, {}), expected: [{}, {}))",
                        lhs.loc.begin, lhs.loc.end, rhs.loc.begin, rhs.loc.end);
                }
                if (lhs.var != rhs.var) {
                    return std::format("variable name differs (actual: '{}', expected: '{}')", lhs.var,
                                       rhs.var);
                }
                return std::nullopt;
            },
            [](const application &lhs, const application &rhs) -> std::optional<std::string> {
                if (lhs.loc != rhs.loc) {
                    return std::format(
                        "application location differs (actual: [{}, {}), expected: [{}, {}))",
                        lhs.loc.begin, lhs.loc.end, rhs.loc.begin, rhs.loc.end);
                }
                if (auto mismatch = is_equal(lhs.func, rhs.func); mismatch) {
                    return std::format("application function: {}", *mismatch);
                }
                if (auto mismatch = is_equal(lhs.arg, rhs.arg); mismatch) {
                    return std::format("application argument: {}", *mismatch);
                }
                return std::nullopt;
            },
            [&](const auto & /*lhs*/, const auto & /*rhs*/) -> std::optional<std::string> {
                return std::format("expression type differs (actual: {}, expected: {})",
                                   lhs->value.index(), rhs->value.index());
            },
        },
        lhs->value, rhs->value);
}

std::optional<std::string> is_equal(const lambda::program_ast::stmt_ptr &lhs,
                                    const lambda::program_ast::stmt_ptr &rhs) {
    using namespace lambda::program_ast;

    return std::visit(
        lambda::overloaded{
            [](const print &lhs, const print &rhs) -> std::optional<std::string> {
                if (lhs.loc != rhs.loc) {
                    return std::format(
                        "print statement location differs (actual: [{}, {}), expected: [{}, {}))",
                        lhs.loc.begin, lhs.loc.end, rhs.loc.begin, rhs.loc.end);
                }
                if (auto mismatch = is_equal(lhs.func, rhs.func); mismatch) {
                    return std::format("print expression: {}", *mismatch);
                }
                return std::nullopt;
            },
            [&](const auto & /*lhs*/, const auto & /*rhs*/) -> std::optional<std::string> {
                return std::format("statement type differs (actual: {}, expected: {})",
                                   lhs->value.index(), rhs->value.index());
            },
        },
        lhs->value, rhs->value);
}

int main() {

    try {
        auto ast_expected = [] {
            using namespace lambda::program_ast;
            std::vector<stmt_ptr> stmts;
            // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
            stmts.push_back(std::make_unique<stmt>(print{
                .func = std::make_unique<expr>(abstraction{
                    .param = var_def{.var = "a", .loc = {.begin = 9, .end = 10}},
                    .body = std::make_unique<expr>(application{
                        .func = std::make_unique<expr>(abstraction{
                            .param = var_def{.var = "a", .loc = {.begin = 13, .end = 14}},
                            .body = std::make_unique<expr>(abstraction{
                                .param = var_def{.var = "b", .loc = {.begin = 16, .end = 17}},
                                .body = std::make_unique<expr>(application{
                                    .func = std::make_unique<expr>(abstraction{
                                        .param = var_def{.var = "b", .loc = {.begin = 20, .end = 21}},
                                        .body = std::make_unique<expr>(
                                            var_ref{.var = "b", .loc = {.begin = 22, .end = 23}}),
                                        .loc = {.begin = 18, .end = 24},
                                    }),
                                    .arg = std::make_unique<expr>(
                                        var_ref{.var = "a", .loc = {.begin = 24, .end = 25}}),
                                    .loc = {.begin = 18, .end = 25},
                                }),
                                .loc = {.begin = 15, .end = 25},
                            }),
                            .loc = {.begin = 11, .end = 26},
                        }),
                        .arg =
                            std::make_unique<expr>(var_ref{.var = "a", .loc = {.begin = 26, .end = 27}}),
                        .loc = {.begin = 11, .end = 27},
                    }),
                    .loc = {.begin = 7, .end = 28},
                }),
                .loc = {.begin = 1, .end = 29},
            }));
            // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
            return program_ast{.stmts = std::move(stmts)};
        }();

        auto tokens = lambda::lex(R"guard(
print (\a.(\a.\b.(\b.b)a)a);
# This is just a comment.
)guard");

        if (!tokens.has_value()) {
            const auto &err = tokens.error();
            std::println("test_parser: error: {} (begin: {}, end: {}).", err.what(), err.loc.begin,
                         err.loc.end);
            return 1;
        }

        auto ast = lambda::parse(*tokens);

        if (!ast.has_value()) {
            const auto &err = ast.error();
            std::println("test_parser: error: {} (location: [{}, {})).", err.what(), err.loc.begin,
                         err.loc.end);
            return 1;
        }

        if (ast->stmts.size() != ast_expected.stmts.size()) {
            std::println(
                "test_parser: error: The size of the parsed AST's statements is incorrect. (expected to "
                "be {}, but it is actually {})",
                ast_expected.stmts.size(), ast->stmts.size());
            return 1;
        }

        for (const auto &[stmt, stmt_expected] : std::views::zip(ast->stmts, ast_expected.stmts)) {
            if (auto mismatch = is_equal(stmt, stmt_expected); mismatch) {
                std::println("test_parser: error: A statement is incorrect: {}", *mismatch);
                return 1;
            }
        }

        std::println("test_parser: ok.");
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