#ifndef __MACROS_H_INCLUDED__
#define __MACROS_H_INCLUDED__

#define NOP void(0)

#define GUARD(expr)                                                                               \
    do {                                                                                          \
        expr;                                                                                     \
    } while (false)

#ifndef NDEBUG
#include <cassert>
#include <cstdlib>
#include <iostream>

#define STATIC_ASSERT(...) static_assert(__VA_ARGS__)
#define ASSERT(...) assert(__VA_ARGS__)
#define UNREACHABLE(msg) assert(msg && false)

#define LOG_ERROR(msg) GUARD(std::cerr << "ERROR/[" << __FUNCTION__ << "] " << msg << "\n");
#define LOG(msg) GUARD(std::cerr << "LOG/[" << __FUNCTION__ << "] " << msg << "\n");
#else
#define STATIC_ASSERT(...) NOP;
#define ASSERT(...) NOP;
#define UNREACHABLE(msg) std::exit(EXIT_FAILURE);

#define LOG_ERROR(msg) NOP;
#define LOG(msg) NOP;
#endif

#define COMMA() ,
#define EMPTY()
#define DEFER(id) id EMPTY()
#define REPEAT(...) __VA_ARGS__

// concatenation
#define CAT(x, y) CAT_(x, y)
#define CAT_(x, y) x##y

#define CHAIN_COMMA(LIST) CAT(CHAIN_COMMA_0 LIST, _END)
#define CHAIN_COMMA_0(...) DEFER(REPEAT)(__VA_ARGS__) CHAIN_COMMA_1
#define CHAIN_COMMA_1(...) DEFER(COMMA)() DEFER(REPEAT)(__VA_ARGS__) CHAIN_COMMA_2
#define CHAIN_COMMA_2(...) DEFER(COMMA)() DEFER(REPEAT)(__VA_ARGS__) CHAIN_COMMA_1
#define CHAIN_COMMA_0_END
#define CHAIN_COMMA_1_END
#define CHAIN_COMMA_2_END

// relies on definition of macro function SEPARATOR()
#define CHAIN_SEPARATOR(LIST) CAT(CHAIN_SEPARATOR_0 LIST, _END)
#define CHAIN_SEPARATOR_0(...) DEFER(REPEAT)(__VA_ARGS__) CHAIN_SEPARATOR_1
#define CHAIN_SEPARATOR_1(...) DEFER(SEPARATOR)() DEFER(REPEAT)(__VA_ARGS__) CHAIN_SEPARATOR_2
#define CHAIN_SEPARATOR_2(...) DEFER(SEPARATOR)() DEFER(REPEAT)(__VA_ARGS__) CHAIN_SEPARATOR_1
#define CHAIN_SEPARATOR_0_END
#define CHAIN_SEPARATOR_1_END
#define CHAIN_SEPARATOR_2_END

// relies on definition of macro functions PREFIX(ELEM, ...) AND SUFFIX(ELEM, ...)
#define CHAIN_PREFIX_SUFFIX(LIST) CAT(CHAIN_PREFIX_SUFFIX_1 LIST, _END)
#define CHAIN_PREFIX_SUFFIX_1(...) DEFER(SUFFIX)(DEFER(PREFIX)(__VA_ARGS__)) CHAIN_PREFIX_SUFFIX_2
#define CHAIN_PREFIX_SUFFIX_2(...) DEFER(SUFFIX)(DEFER(PREFIX)(__VA_ARGS__)) CHAIN_PREFIX_SUFFIX_1
#define CHAIN_PREFIX_SUFFIX_1_END
#define CHAIN_PREFIX_SUFFIX_2_END

// relies on definition of macro function OPERATOR(ELEM, ...)
#define CHAIN_OPERATOR(LIST) CAT(CHAIN_OPERATOR_1 LIST, _END)
#define CHAIN_OPERATOR_1(...) DEFER(OPERATOR)(__VA_ARGS__) CHAIN_OPERATOR_2
#define CHAIN_OPERATOR_2(...) DEFER(OPERATOR)(__VA_ARGS__) CHAIN_OPERATOR_1
#define CHAIN_OPERATOR_1_END
#define CHAIN_OPERATOR_2_END

#define DEFAULT_DTOR(class_name) ~class_name() = default
#define DEFAULT_CTOR(class_name) class_name() = default

#define NO_DEFAULT_DTOR(class_name) ~class_name() = delete
#define NO_DEFAULT_CTOR(class_name) class_name() = delete

#define NO_COPY_CTOR(class_name) class_name(const class_name&) = delete
#define NO_COPY_OPERATOR(class_name) void operator=(const class_name&) = delete

#define NO_COPY_SEMANTIC(class_name)                                                              \
    NO_COPY_CTOR(class_name);                                                                     \
    NO_COPY_OPERATOR(class_name)

#define NO_MOVE_CTOR(class_name) class_name(class_name&&) = delete
#define NO_MOVE_OPERATOR(class_name) class_name& operator=(class_name&&) = delete

#define NO_MOVE_SEMANTIC(class_name)                                                              \
    NO_MOVE_CTOR(class_name);                                                                     \
    NO_MOVE_OPERATOR(class_name)

#define DEFAULT_MOVE_CTOR(class_name) class_name(class_name&&) = default

#define DEFAULT_MOVE_OPERATOR(class_name) class_name& operator=(class_name&&) = default

#define DEFAULT_MOVE_SEMANTIC(class_name)                                                         \
    DEFAULT_MOVE_CTOR(class_name);                                                                \
    DEFAULT_MOVE_OPERATOR(class_name)

#define DEFAULT_COPY_CTOR(class_name) class_name(const class_name&) = default

#define DEFAULT_COPY_OPERATOR(class_name) class_name& operator=(const class_name&) = default

#define DEFAULT_COPY_SEMANTIC(class_name)                                                         \
    DEFAULT_COPY_CTOR(class_name);                                                                \
    DEFAULT_COPY_OPERATOR(class_name)

#define GETTER(func_name, field_name)                                                             \
    auto Get##func_name() const                                                                   \
    {                                                                                             \
        return field_name;                                                                        \
    }

#define SETTER(func_name, field_type, field_name)                                                 \
    void Set##func_name(field_type f_##field_name)                                                \
    {                                                                                             \
        field_name = f_##field_name;                                                              \
    }

#define GETTER_SETTER(func_name, field_type, field_name)                                          \
    GETTER(func_name, field_name)                                                                 \
    SETTER(func_name, field_type, field_name)

#endif
