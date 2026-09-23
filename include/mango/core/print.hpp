/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2025 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#pragma once

#include <cstdio>
#include <string>
#include <mango/core/configure.hpp>
#include <mango/core/string.hpp>

namespace mango
{

    enum class Print
    {
        Error,
        Warning,
        Info,
        Debug,
        Verbose
    };

    // Opt-in redirect; null = default printf; still gated by isEnable.
    // Handler is invoked synchronously and must copy before returning; must NOT store
    // pointer (it points into a call-site temporary); logical message only, no trailing newline, indent already expanded.
    // Storage lives in system.cpp: single instance even across DLL boundary.
    using PrintHandler = void (*)(Print target, const char* text);

    void setPrintHandler(PrintHandler handler);
    void resetPrintHandler();
    PrintHandler getPrintHandler();

    void printEnable(Print target, bool enable);
    bool isEnable(Print target);

    // ----------------------------------------------------------------------------------
    // print()
    // ----------------------------------------------------------------------------------

    inline
    void print(Print target, const std::string& text)
    {
        if (isEnable(target))
        {
            if (PrintHandler handler = getPrintHandler())
                handler(target, text.c_str());
            else
                std::printf("%s", text.c_str());
        }
    }

    inline
    void print(Print target, int indent, const std::string& text)
    {
        if (isEnable(target))
        {
            if (PrintHandler handler = getPrintHandler())
            {
                std::string tmp = std::string(indent, ' ') + text;
                handler(target, tmp.c_str());
            }
            else
                std::printf("%*s%s", indent, "",  text.c_str());
        }
    }

    template <typename... T>
    inline
    void print(Print target, fmt::format_string<T...> fmt, T&&... args)
    {
        if (isEnable(target))
        {
            std::string s = fmt::format(fmt, std::forward<T>(args)...);
            if (PrintHandler handler = getPrintHandler())
                handler(target, s.c_str());
            else
                std::printf("%s", s.c_str());
        }
    }

    template <typename... T>
    inline
    void print(Print target, int indent, fmt::format_string<T...> fmt, T&&... args)
    {
        if (isEnable(target))
        {
            if (PrintHandler handler = getPrintHandler())
            {
                std::string s = std::string(indent, ' ') + fmt::format(fmt, std::forward<T>(args)...);
                handler(target, s.c_str());
            }
            else
            {
                std::string s = fmt::format(fmt, std::forward<T>(args)...);
                std::printf("%*s%s", indent, "", s.c_str());
            }
        }
    }

    inline
    void print(const std::string& text)
    {
        print(Print::Verbose, text);
    }

    inline
    void print(int indent, const std::string& text)
    {
        print(Print::Verbose, indent, text);
    }

    template <typename... T>
    inline
    void print(fmt::format_string<T...> fmt, T&&... args)
    {
        print(Print::Verbose, fmt, std::forward<T>(args)...);
    }

    template <typename... T>
    inline
    void print(int indent, fmt::format_string<T...> fmt, T&&... args)
    {
        print(Print::Verbose, indent, fmt, std::forward<T>(args)...);
    }

    // ----------------------------------------------------------------------------------
    // printLine()
    // ----------------------------------------------------------------------------------

    inline
    void printLine(Print target, const std::string& text)
    {
        if (isEnable(target))
        {
            if (PrintHandler handler = getPrintHandler())
                handler(target, text.c_str());
            else
                std::printf("%s\n", text.c_str());
        }
    }

    inline
    void printLine(Print target, int indent, const std::string& text)
    {
        if (isEnable(target))
        {
            if (PrintHandler handler = getPrintHandler())
            {
                std::string tmp = std::string(indent, ' ') + text;
                handler(target, tmp.c_str());
            }
            else
                std::printf("%*s%s\n", indent, "", text.c_str());
        }
    }

    template <typename... T>
    inline
    void printLine(Print target, fmt::format_string<T...> fmt, T&&... args)
    {
        if (isEnable(target))
        {
            std::string s = fmt::format(fmt, std::forward<T>(args)...);
            if (PrintHandler handler = getPrintHandler())
                handler(target, s.c_str());
            else
                std::printf("%s\n", s.c_str());
        }
    }

    template <typename... T>
    inline
    void printLine(Print target, int indent, fmt::format_string<T...> fmt, T&&... args)
    {
        if (isEnable(target))
        {
            if (PrintHandler handler = getPrintHandler())
            {
                std::string s = std::string(indent, ' ') + fmt::format(fmt, std::forward<T>(args)...);
                handler(target, s.c_str());
            }
            else
            {
                std::string s = fmt::format(fmt, std::forward<T>(args)...);
                std::printf("%*s%s\n", indent, "", s.c_str());
            }
        }
    }

    inline
    void printLine(const std::string& text)
    {
        printLine(Print::Verbose, text);
    }

    inline
    void printLine(int indent, const std::string& text)
    {
        printLine(Print::Verbose, indent, text);
    }

    template <typename... T>
    inline
    void printLine(fmt::format_string<T...> fmt, T&&... args)
    {
        printLine(Print::Verbose, fmt, std::forward<T>(args)...);
    }

    template <typename... T>
    inline
    void printLine(int indent, fmt::format_string<T...> fmt, T&&... args)
    {
        printLine(Print::Verbose, indent, fmt, std::forward<T>(args)...);
    }

} // namespace mango
