#ifndef __MACROS_H_INCLUDED__
#define __MACROS_H_INCLUDED__

#define BITS_IN_BYTE 8

#define DEFAULT_MOVE_CTOR(class_name) class_name(class_name&& other) = default;
#define NO_DEFAULT_MOVE_CTOR(class_name) class_name(class_name&& other) = delete;

#define DEFAULT_DTOR(class_name) ~class_name() = default
#define DEFAULT_CTOR(class_name) class_name() = default

#define NO_DEFAULT_DTOR(class_name) ~class_name() = delete
#define NO_DEFAULT_CTOR(class_name) class_name() = delete

#define NO_COPY_CTOR(class_name) class_name(const class_name&) = delete

#define LOG_ERROR(msg) std::cerr << "ERROR/[" << __FUNCTION__ << "] " << msg << "\n";

#define LOG(msg) std::cerr << "LOG/[" << __FUNCTION__ << "] " << msg << "\n";

#define GETTER(func_name, field_name)                                                             \
    auto Get##func_name()                                                                         \
    {                                                                                             \
        return field_name;                                                                        \
    }                                                                                             \
    const auto Get##func_name() const                                                             \
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
