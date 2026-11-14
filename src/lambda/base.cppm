/**
 * @file base.cppm
 * @author cpp-love (15865418+cpp-love@user.noreply.gitee.com)
 * @brief 定义了 Lambda 的 AST 部分的基本组件。
 * @version 0.1.0-1
 * @date 2026-11-07
 * 
 * @copyright cpp-love
 * 
 */

export module lambda:base;

import std;

export namespace lambda {

    /// @brief 源码的位置。
    /// @details 表示范围为 [.begin, .end)。
    struct source_loc {
        std::ptrdiff_t               begin{}; ///< 起始位置（以字节偏移量计）。
        std::ptrdiff_t               end{};   ///< 结束位置的后一位（以字节偏移量计）。

        /**
         * @brief 获取长度。
         * @return std::ptrdiff_t 位置的长度。
         */
        [[nodiscard]] std::ptrdiff_t size() const noexcept { return end - begin; }

        /**
         * @brief 判断是否为点。
         * @return true 是点。
         * @return false 不是点。
         */
        [[nodiscard]] bool           is_point() const noexcept { return end == begin; }
    };

    /**
     * @brief 源码位置类的等于运算符重载。
     * @param [in] lhs 左操作数。
     * @param [in] rhs 右操作数。
     * @return true 源码位置相等。
     * @return false 源码位置不相等。
     */
    [[nodiscard]] bool operator==(source_loc lhs, source_loc rhs) noexcept {
        return lhs.begin == rhs.begin && lhs.end == rhs.end;
    }

    /// @brief 错误类型。
    struct error {
        std::string               type;    ///< 错误类型。
        std::uint32_t             id{};    ///< 错误 id。
        std::string               message; ///< 错误信息。
        source_loc                loc;     ///< 源码位置。

        /**
         * @brief 生成错误信息。
         * @return std::string 错误信息。
         */
        [[nodiscard]] std::string what() const { return std::format("{} ({} #{})", message, type, id); }
    };

    /**
     * @brief 提供重载可调用对象的调用运算符的类。
     * @tparam Ts 要重载的可调用对象。
     */
    template <typename... Ts>
    struct overloaded : Ts... {
        using Ts::operator()...;
    };

} // namespace lambda