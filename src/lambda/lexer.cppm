/**
 * @file lexer.cppm
 * @author cpp-love (15865418+cpp-love@user.noreply.gitee.com)
 * @brief 实现了 lexer。
 * @version 0.1.0-1
 * @date 2026-11-07
 * 
 * @copyright cpp-love
 * 
 */

export module lambda:lexer;

import std;
import :base;

export namespace lambda {

    /// @brief lexer 解析出来的 token。
    struct token {
        /// @brief 左小括号。
        struct left_parenthesis {};
        /// @brief 右小括号。
        struct right_parenthesis {};
        /// @brief lambda 符号（使用 `\` 代替）。
        struct lambda {};
        /// @brief `.` 符号。
        struct dot {};
        /// @brief `;` 符号。
        struct semicolon {};
        /// @brief 结束标志。
        struct end {};
        /// @brief 其他字符串。
        struct str {
            std::string_view str; ///< 字符串。
        };

        std::variant<left_parenthesis, right_parenthesis, lambda, dot, semicolon, end, str>
                   value; ///< token 的值。
        source_loc loc;   ///< token 对应的源码的位置。
    };

    using lex_result = std::expected<std::vector<token>, error>; ///< lexer 解析结果别名。

    /**
     * @brief 执行 lexer。
     * @param [in] source_str 源码字符串。
     * @return lex_result 如合法，返回解析出来的 token 序列；如不合法，返回错误原因。
     */
    [[nodiscard]] lex_result lex(std::string_view source_str) {
        std::vector<token>   tokens;            ///< token 列表。
        std::optional<token> current_token;     ///< 当前的 token。
        bool                 iscomment = false; ///< 是否是注释。
        for (const auto &[offset, character] : std::views::enumerate(source_str)) {
            if (iscomment) {
                if (character == '\n') {
                    iscomment = false;
                }
                continue;
            }
            switch (character) {
                case '\\':
                    if (current_token.has_value()) {
                        tokens.push_back(*std::exchange(current_token, {}));
                    }
                    tokens.push_back(
                        {.value = token::lambda{}, .loc = {.begin = offset, .end = offset + 1}});
                    break;
                case '.':
                    if (current_token.has_value()) {
                        tokens.push_back(*std::exchange(current_token, {}));
                    }
                    tokens.push_back(
                        {.value = token::dot{}, .loc = {.begin = offset, .end = offset + 1}});
                    break;
                case '(':
                    if (current_token.has_value()) {
                        tokens.push_back(*std::exchange(current_token, {}));
                    }
                    tokens.push_back({
                        .value = token::left_parenthesis{},
                        .loc = {.begin = offset, .end = offset + 1},
                    });
                    break;
                case ')':
                    if (current_token.has_value()) {
                        tokens.push_back(*std::exchange(current_token, {}));
                    }
                    tokens.push_back({
                        .value = token::right_parenthesis{},
                        .loc = {.begin = offset, .end = offset + 1},
                    });
                    break;
                case ';':
                    if (current_token.has_value()) {
                        tokens.push_back(*std::exchange(current_token, {}));
                    }
                    tokens.push_back({
                        .value = token::semicolon{},
                        .loc = {.begin = offset, .end = offset + 1},
                    });
                    break;
                case '#':
                    if (current_token.has_value()) {
                        tokens.push_back(*std::exchange(current_token, {}));
                    }
                    iscomment = true;
                    break;
                default:
                    if (std::isspace(static_cast<int>(character)) != 0) {
                        if (current_token.has_value()) {
                            tokens.push_back(*std::exchange(current_token, {}));
                        }
                        break;
                    }
                    if (std::isalpha(static_cast<int>(character)) == 0) {
                        return lex_result{
                            std::unexpect,
                            error{
                                .type = "lexer",
                                .id = 1,
                                .message = std::format("Unrecognizable character '{}'.", character),
                                .loc = source_loc{.begin = offset, .end = offset + 1},
                            }};
                    }
                    current_token =
                        std::move(current_token)
                            .transform([&](token current_token) {
                                current_token.loc.end = offset + 1;
                                std::get<token::str>(current_token.value).str =
                                    source_str.substr(current_token.loc.begin, current_token.loc.size());
                                return current_token;
                            })
                            .or_else([&] {
                                return std::optional<token>{{
                                    .value = token::str{source_str.substr(offset, 1)},
                                    .loc = {.begin = offset, .end = offset + 1},
                                }};
                            });
                    break;
            }
        }
        if (current_token.has_value()) {
            tokens.push_back(*current_token);
        }
        tokens.push_back({
            .value = token::end{},
            .loc = {.begin = std::ranges::ssize(source_str), .end = std::ranges::ssize(source_str)},
        });
        return tokens;
    }

} // namespace lambda