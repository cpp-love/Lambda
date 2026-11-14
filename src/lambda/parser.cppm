/**
 * @file parser.cppm
 * @author cpp-love (15865418+cpp-love@user.noreply.gitee.com)
 * @brief 实现了 parser。
 * @version 0.1.0-1
 * @date 2026-11-12
 * 
 * @copyright cpp-love
 * 
 */

export module lambda:parser;

import std;
import :lexer;
import :program_ast;

namespace lambda {

    export using parse_result =
        std::expected<program_ast::program_ast, error>; ///< parser 解析结果别名。

    /// @brief parser。
    class parser {
      private:
        std::span<token> m_tokens;    ///< lexer 解析出来的 token 列表。
        std::size_t      m_index = 0; ///< 下一个要被处理的索引。

      public:
        /**
         * @brief 构造 parser 对象。
         * @param [in] tokens lexer 解析出来的 token 列表。 
         */
        explicit parser(std::span<token> tokens) : m_tokens(tokens) {}

        /**
         * @brief 执行 parser。
         * @return parse_result 如合法，返回生成的 AST；如不合法，返回错误原因。
         */
        parse_result parse() {
            program_ast::program_ast ast;
            while (!match<token::end>()) {
                auto ret = parse_stmt();
                if (!ret.has_value()) {
                    return parse_result{std::unexpect, ret.error()};
                }
                ast.stmts.push_back(std::move(*ret));
            }
            return ast;
        }

      private:
        /**
         * @brief 查看后面的 token。
         * @param [in] offset 可选的偏移量（默认为 0，即下一个元素）。
         * @return const token& 后面的 token。
         */
        [[nodiscard]] const token &peek(std::ptrdiff_t offset = 0) const noexcept {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
            return m_tokens[m_index + offset];
        }

        /**
         * @brief 获取 token 以用于处理。
         * @return const token& 要被处理的 token。
         */
        const token &advance() noexcept {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
            return m_tokens[m_index++];
        }

        /**
         * @brief 后面的元素是否为指定类型的 token。
         * @tparam T 指定的 token 类型。
         * @param [in] offset 可选的偏移量（默认为 0，即下一个元素）。
         * @return true 是。
         * @return false 不是。
         */
        template <typename T>
        [[nodiscard]] bool match(std::ptrdiff_t offset = 0) noexcept {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
            return std::holds_alternative<T>(m_tokens[m_index + offset].value);
        }

        using parse_stmt_result =
            std::expected<program_ast::stmt_ptr, error>; ///< parser 解析语句的结果。
        using parse_expr_result =
            std::expected<program_ast::expr_ptr, error>; ///< parser 解析表达式的结果。

        /**
         * @brief 解析一句语句。
         * @return parse_stmt_result 如合法，返回生成的 AST；如不合法，返回错误原因。
         */
        parse_stmt_result parse_stmt() {
            if (!match<token::str>()) {
                return parse_stmt_result{
                    std::unexpect,
                    error{
                        .type = "parser",
                        .id = 1,
                        .message = "Expect a statement introducer at the beginning of a statement.",
                        .loc = {.begin = peek().loc.begin, .end = peek().loc.begin},
                    }};
            }
            const auto &token = advance();
            auto        str = std::get<token::str>(token.value).str;
            if (str != "print") {
                return parse_stmt_result{
                    std::unexpect,
                    error{
                        .type = "parser",
                        .id = 1,
                        .message = "Expect a statement introducer at the beginning of a statement.",
                        .loc = {.begin = token.loc.begin, .end = token.loc.begin},
                    }};
            }
            return parse_expr().and_then([this, token](program_ast::expr_ptr expr) -> parse_stmt_result {
                if (!match<token::semicolon>()) {
                    return parse_stmt_result{
                        std::unexpect, error{
                                           .type = "parser",
                                           .id = 2,
                                           .message = "Expect ';' at the end of a statement.",
                                           .loc = {.begin = peek().loc.begin, .end = peek().loc.begin},
                                       }};
                }
                advance();
                return std::make_unique<program_ast::stmt>(program_ast::print{
                    .func = std::move(expr),
                    .loc{.begin = token.loc.begin, .end = peek(-1).loc.end},
                });
            });
        }

        /**
         * @brief 解析一句表达式。
         * @return parse_expr_result 如合法，返回生成的 AST；如不合法，返回错误原因。
         */
        parse_expr_result parse_expr() {
            // 解析单个表达式（不包含 `function_call`）。
            auto parse_single_expr = [&] -> parse_expr_result {
                const auto &start_token = advance();
                return std::visit(
                    overloaded(
                        [&](const token::lambda & /*unused*/) -> parse_expr_result {
                            if (!match<token::str>()) {
                                return parse_expr_result{
                                    std::unexpect,
                                    error{
                                        .type = "parser",
                                        .id = 3,
                                        .message = "Expect a parameter after the '\\' lambda "
                                                   "abstraction introducer.",
                                        .loc = {.begin = peek().loc.begin, .end = peek().loc.begin},
                                    }};
                            }
                            const auto &var_token = advance();
                            auto        param = program_ast::var_def{
                                .var{std::get<token::str>(var_token.value).str},
                                .loc = var_token.loc,
                            };
                            if (!match<token::dot>()) {
                                return parse_expr_result{
                                    std::unexpect,
                                    error{
                                        .type = "parser",
                                        .id = 4,
                                        .message = "Expect '.' after the lambda abstraction parameter.",
                                        .loc = {.begin = peek().loc.begin, .end = peek().loc.begin},
                                    }};
                            }
                            advance();
                            return parse_expr().transform([&](program_ast::expr_ptr expr) {
                                return std::make_unique<program_ast::expr>(program_ast::abstraction{
                                    .param = param,
                                    .body = std::move(expr),
                                    .loc{.begin = start_token.loc.begin, .end = peek(-1).loc.end},
                                });
                            });
                        },
                        [&](const token::left_parenthesis & /*unused*/) -> parse_expr_result {
                            return parse_expr().and_then([&](program_ast::expr_ptr expr)
                                                             -> parse_expr_result {
                                if (!match<token::right_parenthesis>()) {
                                    return parse_expr_result{
                                        std::unexpect,
                                        error{
                                            .type = "parser",
                                            .id = 5, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
                                            .message = "Expect ')' to match '('.",
                                            .loc = {.begin = peek().loc.begin, .end = peek().loc.begin},
                                        }};
                                }
                                advance();
                                expr->get_loc() = {.begin = start_token.loc.begin,
                                                   .end = peek(-1).loc.end};
                                return expr;
                            });
                        },
                        [&](const token::str &str) -> parse_expr_result {
                            return std::make_unique<program_ast::expr>(
                                program_ast::var_ref{.var{str.str}, .loc = start_token.loc});
                        },
                        [&](const auto & /*unused*/) -> parse_expr_result {
                            return parse_expr_result{
                                std::unexpect,
                                error{
                                    .type = "parser",
                                    .id = 6, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
                                    .message = "Expect a expression at this place.",
                                    .loc = {.begin = peek().loc.begin, .end = peek().loc.begin},
                                }};
                        }),
                    start_token.value);
            };

            // 处理 `function_call`。
            auto single_expr = parse_single_expr();
            if (!single_expr.has_value()) {
                return single_expr;
            }
            program_ast::expr_ptr lhs = std::move(*single_expr);

            while (!(match<token::dot>() || match<token::right_parenthesis>()
                     || match<token::semicolon>())) {
                auto single_expr = parse_single_expr();
                if (!single_expr.has_value()) {
                    return single_expr;
                }
                source_loc loc{.begin = lhs->get_loc().begin, .end = (*single_expr)->get_loc().end};
                lhs = std::make_unique<program_ast::expr>(program_ast::application{
                    .func = std::move(lhs),
                    .arg = std::move(*single_expr),
                    .loc = loc,
                });
            }
            return lhs;
        }
    };

    /**
     * @brief 执行 parser。
     * @param [in] tokens lexer 解析出来的 token 列表。 
     * @return parse_result 如合法，返回生成的 AST；如不合法，返回错误原因。
     */
    export parse_result parse(std::span<token> tokens) { return parser{tokens}.parse(); }

} // namespace lambda