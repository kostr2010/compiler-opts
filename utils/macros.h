#ifndef __MACROS_H_INCLUDED__
#define __MACROS_H_INCLUDED__

#define DEFAULT_DTOR(class_name) ~class_name() = default;
#define DEFAULT_CTOR(class_name) class_name() = default;

#define NO_DEFAULT_DTOR(class_name) ~class_name() = delete;
#define NO_DEFAULT_CTOR(class_name) class_name() = delete;

#define NO_COPY_CTOR(class_name) class_name(const class_name&) = delete;

#define SINGLETON_GET_INSTANCE GetInstance

#define SINGLETON_GET_INSTANCE_IMPLEMENT(class_name)                                              \
    static class_name* SINGLETON_GET_INSTANCE(Graph* graph = nullptr)                             \
    {                                                                                             \
        static class_name inst{ graph };                                                          \
        return &inst;                                                                             \
    }

#define SINGLETON_CTOR(class_name)                                                                \
    class_name(Graph* graph) : Pass(graph)                                                        \
    {                                                                                             \
        std::cout << "ctor\n";                                                                    \
    }

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