/**
 * @file program_ast.cppm
 * @author cpp-love (15865418+cpp-love@user.noreply.gitee.com)
 * @brief 定义了 AST 的各种节点。
 * @version 0.1.0-1
 * @date 2026-11-07
 * 
 * @copyright cpp-love
 * 
 */

export module lambda:program_ast;

import std;
import :base;

export namespace lambda::program_ast {

    /// @brief 表达式节点。
    struct expr;
    using expr_ptr = std::unique_ptr<expr>; ///< 表达式节点指针别名。

    /// @brief 定义变量的节点。
    struct var_def {
        std::string var; ///< 定义的变量。
        source_loc  loc; ///< 对应的源码位置。
    };

    /// @brief 引用变量的节点。
    struct var_ref {
        std::string var; ///< 使用的变量。
        source_loc  loc; ///< 对应的源码位置。
    };

    /// @brief 定义 lambda 抽象的节点。
    struct abstraction {
        var_def    param; ///< 定义的形参。
        expr_ptr   body;  ///< 返回的表达式。
        source_loc loc;   ///< 对应的源码位置。
    };

    /// @brief 应用 lambda 抽象的节点。
    struct application {
        expr_ptr   func; ///< 要被应用的 lambda 抽象。
        expr_ptr   arg;  ///< 传入的实参。
        source_loc loc;  ///< 对应的源码位置。
    };

    struct expr {
        using variant = std::variant<var_ref, abstraction, application>; ///< variant 别名。
        variant                   value;                                 ///< 存储的值。

        /**
         * @brief 获取对应的源码位置。
         * @return source_loc& 对应的源码位置。
         */
        [[nodiscard]] source_loc &get_loc() & {
            return std::visit([](auto &value) -> source_loc & { return value.loc; }, value);
        }
        /**
         * @brief 获取对应的源码位置。
         * @return const source_loc& 对应的源码位置。
         */
        [[nodiscard]] const source_loc &get_loc() const & {
            return std::visit([](const auto &value) -> const source_loc & { return value.loc; }, value);
        }
        /**
         * @brief 获取对应的源码位置。
         * @return source_loc 对应的源码位置。
         */
        [[nodiscard]] source_loc get_loc() && {
            return std::visit([](auto &&value) -> source_loc { return value.loc; }, std::move(value));
        }
    };

    /// @brief 语句节点。
    struct stmt;
    using stmt_ptr = std::unique_ptr<stmt>; ///< 语句节点指针别名。

    /// @brief 输出 lambda 抽象的节点。
    struct print {
        expr_ptr   func; ///< 要打印的 lambda 抽象。
        source_loc loc;  ///< 对应的源码位置。
    };

    struct stmt {
        using variant = std::variant<print>; ///< variant 别名。
        variant                   value;     ///< 存储的值。

        /**
         * @brief 获取对应的源码位置。
         * @return source_loc& 对应的源码位置。
         */
        [[nodiscard]] source_loc &get_loc() & {
            return std::visit([](auto &value) -> source_loc & { return value.loc; }, value);
        }
        /**
         * @brief 获取对应的源码位置。
         * @return const source_loc& 对应的源码位置。
         */
        [[nodiscard]] const source_loc &get_loc() const & {
            return std::visit([](const auto &value) -> const source_loc & { return value.loc; }, value);
        }
        /**
         * @brief 获取对应的源码位置。
         * @return source_loc 对应的源码位置。
         */
        [[nodiscard]] source_loc get_loc() && {
            return std::visit([](auto &&value) -> source_loc { return value.loc; }, std::move(value));
        }
    };

    /// @brief 整个程序的 AST。
    struct program_ast {
        std::vector<stmt_ptr> stmts; ///< 语句列表。
    };

} // namespace lambda::program_ast
