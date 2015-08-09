#ifndef HALIDE_INTROSPECTION_H
#define HALIDE_INTROSPECTION_H

#include <string>
#include <iostream>
#include <stdint.h>

// Always use assert, even if llvm-config defines NDEBUG
#ifdef NDEBUG
#undef NDEBUG
#include <assert.h>
#define NDEBUG
#else
#include <assert.h>
#endif

#ifndef HALIDE_UTIL_H
#define HALIDE_UTIL_H

/** \file
 * Various utility functions used internally Halide. */

#include <vector>
#include <string>
#include <cstring>

// by default, the symbol EXPORT does nothing. In windows dll builds we can define it to __declspec(dllexport)
#if defined(_WIN32) && defined(Halide_SHARED)
#ifdef Halide_EXPORTS
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif
#else
#define EXPORT
#endif

// If we're in user code, we don't want certain functions to be inlined.
#if defined(COMPILING_HALIDE) || defined(BUILDING_PYTHON)
#define NO_INLINE
#else
#ifdef _WIN32
#define NO_INLINE __declspec(noinline)
#else
#define NO_INLINE __attribute__((noinline))
#endif
#endif

namespace Halide {
namespace Internal {

/** Build small vectors of up to 10 elements. If we used C++11 and
 * had vector initializers, this would not be necessary, but we
 * don't want to rely on C++11 support. */
//@{
template<typename T>
std::vector<T> vec(T a) {
    std::vector<T> v(1);
    v[0] = a;
    return v;
}

template<typename T>
std::vector<T> vec(T a, T b) {
    std::vector<T> v(2);
    v[0] = a;
    v[1] = b;
    return v;
}

template<typename T>
std::vector<T> vec(T a, T b, T c) {
    std::vector<T> v(3);
    v[0] = a;
    v[1] = b;
    v[2] = c;
    return v;
}

template<typename T>
std::vector<T> vec(T a, T b, T c, T d) {
    std::vector<T> v(4);
    v[0] = a;
    v[1] = b;
    v[2] = c;
    v[3] = d;
    return v;
}

template<typename T>
std::vector<T> vec(T a, T b, T c, T d, T e) {
    std::vector<T> v(5);
    v[0] = a;
    v[1] = b;
    v[2] = c;
    v[3] = d;
    v[4] = e;
    return v;
}

template<typename T>
std::vector<T> vec(T a, T b, T c, T d, T e, T f) {
    std::vector<T> v(6);
    v[0] = a;
    v[1] = b;
    v[2] = c;
    v[3] = d;
    v[4] = e;
    v[5] = f;
    return v;
}

template<typename T>
std::vector<T> vec(T a, T b, T c, T d, T e, T f, T g) {
    std::vector<T> v(7);
    v[0] = a;
    v[1] = b;
    v[2] = c;
    v[3] = d;
    v[4] = e;
    v[5] = f;
    v[6] = g;
    return v;
}

template<typename T>
std::vector<T> vec(T a, T b, T c, T d, T e, T f, T g, T h) {
    std::vector<T> v(8);
    v[0] = a;
    v[1] = b;
    v[2] = c;
    v[3] = d;
    v[4] = e;
    v[5] = f;
    v[6] = g;
    v[7] = h;
    return v;
}

template<typename T>
std::vector<T> vec(T a, T b, T c, T d, T e, T f, T g, T h, T i) {
    std::vector<T> v(9);
    v[0] = a;
    v[1] = b;
    v[2] = c;
    v[3] = d;
    v[4] = e;
    v[5] = f;
    v[6] = g;
    v[7] = h;
    v[8] = i;
    return v;
}

template<typename T>
std::vector<T> vec(T a, T b, T c, T d, T e, T f, T g, T h, T i, T j) {
    std::vector<T> v(10);
    v[0] = a;
    v[1] = b;
    v[2] = c;
    v[3] = d;
    v[4] = e;
    v[5] = f;
    v[6] = g;
    v[7] = h;
    v[8] = i;
    v[9] = j;
    return v;
}
// @}

/** Convert an integer to a string. */
EXPORT std::string int_to_string(int x);

/** An aggressive form of reinterpret cast used for correct type-punning. */
template<typename DstType, typename SrcType>
DstType reinterpret_bits(const SrcType &src) {
    assert(sizeof(SrcType) == sizeof(DstType));
    DstType dst;
    memcpy(&dst, &src, sizeof(SrcType));
    return dst;
}

/** Make a unique name for an object based on the name of the stack
 * variable passed in. If introspection isn't working or there are no
 * debug symbols, just uses unique_name with the given prefix. */
EXPORT std::string make_entity_name(void *stack_ptr, const std::string &type, char prefix);

/** Generate a unique name starting with the given character. It's
 * unique relative to all other calls to unique_name done by this
 * process. Not thread-safe. */
EXPORT std::string unique_name(char prefix);

/** Generate a unique name starting with the given string.  Not
 * thread-safe. */
EXPORT std::string unique_name(const std::string &name, bool user = true);

/** Test if the first string starts with the second string */
EXPORT bool starts_with(const std::string &str, const std::string &prefix);

/** Test if the first string ends with the second string */
EXPORT bool ends_with(const std::string &str, const std::string &suffix);

/** Return the final token of the name string using the given delimiter. */
EXPORT std::string base_name(const std::string &name, char delim = '.');

}
}

#endif

/** \file
 *
 * Defines methods for introspecting in C++. Relies on DWARF debugging
 * metadata, so the compilation unit that uses this must be compiled
 * with -g.
 */

namespace Halide {
namespace Internal {

/** Get the name of a stack variable from its address. The stack
 * variable must be in a compilation unit compiled with -g to
 * work. The expected type helps distinguish between variables at the
 * same address, e.g a class instance vs its first member. */
EXPORT std::string get_variable_name(const void *, const std::string &expected_type);

/** Get the source location in the call stack, skipping over calls in
 * the Halide namespace. */
EXPORT std::string get_source_location();

// This gets called automatically by anyone who includes Halide.h by
// the code below. It tests if this functionality works for the given
// compilation unit, and disables it if not.
EXPORT void test_compilation_unit(bool (*test)(), void (*calib)());

}
}


// This code verifies that introspection is working before relying on
// it. The definitions must appear in Halide.h, but they should not
// appear in libHalide itself. They're defined as weak so that clients
// can include Halide.h multiple times without link errors.
#ifndef COMPILING_HALIDE

namespace Halide {
namespace Internal {
static bool check_introspection(const void *var, const std::string &type,
                                const std::string &correct_name,
                                const std::string &correct_file, int line) {
    std::string correct_loc = correct_file + ":" + int_to_string(line);
    std::string loc = get_source_location();
    std::string name = get_variable_name(var, type);
    return name == correct_name && loc == correct_loc;
}
}
}

namespace HalideIntrospectionCanary {

// A function that acts as a signpost. By taking it's address and
// comparing it to the program counter listed in the debugging info,
// we can calibrate for any offset between the debugging info and the
// actual memory layout where the code was loaded.
static void offset_marker() {
    std::cerr << "You should not have called this function\n";
}

struct A {
    int an_int;

    class B {
        int private_member;
    public:
        float a_float;
        A *parent;
        B() : private_member(17) {
            a_float = private_member * 2.0f;
        }
    };

    B a_b;

    A() {
        a_b.parent = this;
    }

    bool test(const std::string &my_name);
};

static bool test_a(const A &a, const std::string &my_name) {
    bool success = true;
    success &= Halide::Internal::check_introspection(&a.an_int, "int", my_name + ".an_int", __FILE__ , __LINE__);
    success &= Halide::Internal::check_introspection(&a.a_b, "HalideIntrospectionCanary::A::B", my_name + ".a_b", __FILE__ , __LINE__);
    success &= Halide::Internal::check_introspection(&a.a_b.parent, "HalideIntrospectionCanary::A *", my_name + ".a_b.parent", __FILE__ , __LINE__);
    success &= Halide::Internal::check_introspection(&a.a_b.a_float, "float", my_name + ".a_b.a_float", __FILE__ , __LINE__);
    success &= Halide::Internal::check_introspection(a.a_b.parent, "HalideIntrospectionCanary::A", my_name, __FILE__ , __LINE__);
    return success;
}

static bool test() {
    A a1, a2;

    return test_a(a1, "a1") && test_a(a2, "a2");
}

// Run the tests, and calibrate for the PC offset at static initialization time.
namespace {
struct TestCompilationUnit {
    TestCompilationUnit() {
        Halide::Internal::test_compilation_unit(&test, &offset_marker);
    }
};
}

static TestCompilationUnit test_object;

}

#endif

#endif
#ifndef HALIDE_TYPE_H
#define HALIDE_TYPE_H

#include <stdint.h>

/** \file
 * Defines halide types
 */

namespace Halide {

struct Expr;

/** Types in the halide type system. They can be ints, unsigned ints,
 * or floats of various bit-widths (the 'bits' field). They can also
 * be vectors of the same (by setting the 'width' field to something
 * larger than one). Front-end code shouldn't use vector
 * types. Instead vectorize a function. */
struct Type {
    /** The basic type code: signed integer, unsigned integer, or floating point */
    enum TypeCode {
        Int,  //!< signed integers
        UInt, //!< unsigned integers
        Float, //!< floating point numbers
        Handle //!< opaque pointer type (void *)
    } code;

    /** The number of bits of precision of a single scalar value of this type. */
    int bits;

    /** The number of bytes required to store a single scalar value of this type. Ignores vector width. */
    int bytes() const {return (bits + 7) / 8;}

    /** How many elements (if a vector type). Should be 1 for scalar types. */
    int width;

    /** Is this type boolean (represented as UInt(1))? */
    bool is_bool() const {return code == UInt && bits == 1;}

    /** Is this type a vector type? (width > 1) */
    bool is_vector() const {return width > 1;}

    /** Is this type a scalar type? (width == 1) */
    bool is_scalar() const {return width == 1;}

    /** Is this type a floating point type (float or double). */
    bool is_float() const {return code == Float;}

    /** Is this type a signed integer type? */
    bool is_int() const {return code == Int;}

    /** Is this type an unsigned integer type? */
    bool is_uint() const {return code == UInt;}

    /** Is this type an opaque handle type (void *) */
    bool is_handle() const {return code == Handle;}

    /** Compare two types for equality */
    bool operator==(const Type &other) const {
        return code == other.code && bits == other.bits && width == other.width;
    }

    /** Compare two types for inequality */
    bool operator!=(const Type &other) const {
        return code != other.code || bits != other.bits || width != other.width;
    }

    /** Produce a vector of this type, with 'width' elements */
    Type vector_of(int w) const {
        Type type = {code, bits, w};
        return type;
    }

    /** Produce the type of a single element of this vector type */
    Type element_of() const {
        Type type = {code, bits, 1};
        return type;
    }

    /** Can this type represent all values of another type? */
    EXPORT bool can_represent(Type other) const;

    /** Return an integer which is the maximum value of this type. */
    EXPORT int imax() const;

    /** Return an expression which is the maximum value of this type */
    EXPORT Expr max() const;

    /** Return an integer which is the minimum value of this type */
    EXPORT int imin() const;

    /** Return an expression which is the minimum value of this type */
    EXPORT Expr min() const;
};

/** Constructing a signed integer type */
inline Type Int(int bits, int width = 1) {
    Type t;
    t.code = Type::Int;
    t.bits = bits;
    t.width = width;
    return t;
}

/** Constructing an unsigned integer type */
inline Type UInt(int bits, int width = 1) {
    Type t;
    t.code = Type::UInt;
    t.bits = bits;
    t.width = width;
    return t;
}

/** Construct a floating-point type */
inline Type Float(int bits, int width = 1) {
    Type t;
    t.code = Type::Float;
    t.bits = bits;
    t.width = width;
    return t;
}

/** Construct a boolean type */
inline Type Bool(int width = 1) {
    return UInt(1, width);
}

/** Construct a handle type */
inline Type Handle(int width = 1) {
    Type t;
    t.code = Type::Handle;
    t.bits = 64; // All handles are 64-bit for now
    t.width = width;
    return t;
}

namespace {
template<typename T>
struct type_of_helper;

template<typename T>
struct type_of_helper<T *> {
    operator Type() {
        return Handle();
    }
};

template<>
struct type_of_helper<float> {
    operator Type() {return Float(32);}
};

template<>
struct type_of_helper<double> {
    operator Type() {return Float(64);}
};

template<>
struct type_of_helper<uint8_t> {
    operator Type() {return UInt(8);}
};

template<>
struct type_of_helper<uint16_t> {
    operator Type() {return UInt(16);}
};

template<>
struct type_of_helper<uint32_t> {
    operator Type() {return UInt(32);}
};

template<>
struct type_of_helper<uint64_t> {
    operator Type() {return UInt(64);}
};

template<>
struct type_of_helper<int8_t> {
    operator Type() {return Int(8);}
};

template<>
struct type_of_helper<int16_t> {
    operator Type() {return Int(16);}
};

template<>
struct type_of_helper<int32_t> {
    operator Type() {return Int(32);}
};

template<>
struct type_of_helper<int64_t> {
    operator Type() {return Int(64);}
};

template<>
struct type_of_helper<bool> {
    operator Type() {return Bool();}
};
}

/** Construct the halide equivalent of a C type */
template<typename T> Type type_of() {
    return Type(type_of_helper<T>());
}

}

#endif
#ifndef HALIDE_ARGUMENT_H
#define HALIDE_ARGUMENT_H

#include <string>

/** \file
 * Defines a type used for expressing the type signature of a
 * generated halide pipeline
 */

namespace Halide {

/**
 * A struct representing an argument to a halide-generated
 * function. Used for specifying the function signature of
 * generated code.
 */
struct Argument {
    /** The name of the argument */
    std::string name;

    /** An argument is either a primitive type (for parameters), or a
     * buffer pointer. If 'is_buffer' is true, then 'type' should be
     * ignored.
     */
    bool is_buffer;

    /** For buffers, these two variables can be used to specify whether the
     * buffer is read or written. By default, we assume that the argument
     * buffer is read-write and set both flags. */
    bool read;
    bool write;

    /** If this is a scalar parameter, then this is its type */
    Type type;

    Argument() : is_buffer(false) {}
    Argument(const std::string &_name, bool _is_buffer, Type _type) :
        name(_name), is_buffer(_is_buffer), type(_type) {
        read = write = is_buffer;
    }
};
}

#endif
#ifndef HALIDE_BOUNDS_H
#define HALIDE_BOUNDS_H

#ifndef HALIDE_IR_H
#define HALIDE_IR_H

/** \file
 * Halide expressions (\ref Halide::Expr) and statements (\ref Halide::Internal::Stmt)
 */

#include <string>
#include <vector>

#ifndef HALIDE_DEBUG_H
#define HALIDE_DEBUG_H

/** \file
 * Defines functions for debug logging during code generation.
 */

#include <iostream>
#include <string>
#include <stdlib.h>


namespace Halide {

struct Expr;
struct Type;
// Forward declare some things from IRPrinter, which we can't include yet.
EXPORT std::ostream &operator<<(std::ostream &stream, const Expr &);
EXPORT std::ostream &operator<<(std::ostream &stream, const Type &);

namespace Internal {

struct Stmt;
std::ostream &operator<<(std::ostream &stream, const Stmt &);

/** For optional debugging during codegen, use the debug class as
 * follows:
 *
 \code
 debug(verbosity) << "The expression is " << expr << std::endl;
 \endcode
 *
 * verbosity of 0 always prints, 1 should print after every major
 * stage, 2 should be used for more detail, and 3 should be used for
 * tracing everything that occurs. The verbosity with which to print
 * is determined by the value of the environment variable
 * HL_DEBUG_CODEGEN
 */

struct debug {
    EXPORT static int debug_level;
    EXPORT static bool initialized;
    int verbosity;

    debug(int v) : verbosity(v) {
        if (!initialized) {
            // Read the debug level from the environment
            #ifdef _WIN32
            char lvl[32];
            size_t read = 0;
            getenv_s(&read, lvl, "HL_DEBUG_CODEGEN");
            if (read) {
            #else
            if (char *lvl = getenv("HL_DEBUG_CODEGEN")) {
            #endif
                debug_level = atoi(lvl);
            } else {
                debug_level = 0;
            }

            initialized = true;
        }
    }

    template<typename T>
    debug &operator<<(T x) {
        if (verbosity > debug_level) return *this;
        std::cerr << x;
        return *this;
    }
};

}
}

#endif
#ifndef HALIDE_ERROR_H
#define HALIDE_ERROR_H

#include <string>
#include <sstream>
#include <stdexcept>


namespace Halide {

/** Query whether Halide was compiled with exceptions. */
EXPORT bool exceptions_enabled();

/** A base class for Halide errors. */
struct Error : public std::runtime_error {
    // Give each class a non-inlined constructor so that the type
    // doesn't get separately instantiated in each compilation unit.
    EXPORT Error(const std::string &msg);
};

/** An error that occurs while running a JIT-compiled Halide pipeline. */
struct RuntimeError : public Error {
    EXPORT RuntimeError(const std::string &msg);
};

/** An error that occurs while compiling a Halide pipeline that Halide
 * attributes to a user error. */
struct CompileError : public Error {
    EXPORT CompileError(const std::string &msg);
};

/** An error that occurs while compiling a Halide pipeline that Halide
 * attributes to an internal compiler bug, or to an invalid use of
 * Halide's internals. */
struct InternalError : public Error {
    EXPORT InternalError(const std::string &msg);
};


namespace Internal {


struct ErrorReport {
    std::ostringstream *msg;
    const char *file;
    const char *condition_string;
    int line;
    bool condition;
    bool user;
    bool warning;
    bool runtime;

    ErrorReport(const char *f, int l, const char *cs, bool c, bool u, bool w, bool r) :
        msg(NULL), file(f), condition_string(cs), line(l), condition(c), user(u), warning(w), runtime(r) {
        if (condition) return;
        msg = new std::ostringstream;
        const std::string &source_loc = get_source_location();

        if (user) {
            // Only mention where inside of libHalide the error tripped if we have debug level > 0
            debug(1) << "User error triggered at " << f << ":" << l << "\n";
            if (condition_string) {
                debug(1) << "Condition failed: " << condition_string << "\n";
            }
            if (warning) {
                (*msg) << "Warning";
            } else {
                (*msg) << "Error";
            }
            if (source_loc.empty()) {
                (*msg) << ":\n";
            } else {
                (*msg) << " at " << source_loc << ":\n";
            }

        } else {
            (*msg) << "Internal ";
            if (warning) {
                (*msg) << "warning";
            } else {
                (*msg) << "error";
            }
            (*msg) << " at " << f << ":" << l;
            if (!source_loc.empty()) {
                (*msg) << " triggered by user code at " << source_loc << ":\n";
            } else {
                (*msg) << "\n";
            }
            if (condition_string) {
                (*msg) << "Condition failed: " << condition_string << "\n";
            }
        }
    }

    template<typename T>
    ErrorReport &operator<<(T x) {
        if (condition) return *this;
        (*msg) << x;
        return *this;
    }

    /** When you're done using << on the object, and let it fall out of
     * scope, this errors out, or throws an exception if they are
     * enabled. This is a little dangerous because the destructor will
     * also be called if there's an exception in flight due to an
     * error in one of the arguments passed to operator<<. We handle
     * this by only actually throwing if there isn't an exception in
     * flight already.
     */
#if __cplusplus >= 201100
    ~ErrorReport() noexcept(false) {
#else
    ~ErrorReport() {
#endif

        if (condition) return;
        explode();
    }

    EXPORT void explode();
};

#define internal_error            Halide::Internal::ErrorReport(__FILE__, __LINE__, NULL, false, false, false, false)
#define internal_assert(c)        Halide::Internal::ErrorReport(__FILE__, __LINE__, #c,   c,     false, false, false)
#define user_error                Halide::Internal::ErrorReport(__FILE__, __LINE__, NULL, false, true, false, false)
#define user_assert(c)            Halide::Internal::ErrorReport(__FILE__, __LINE__, #c,   c,     true, false, false)
#define user_warning              Halide::Internal::ErrorReport(__FILE__, __LINE__, NULL, false, true, true, false)
#define halide_runtime_error      Halide::Internal::ErrorReport(__FILE__, __LINE__, NULL, false, true, false, true)

// The nicely named versions get cleaned up at the end of Halide.h,
// but user code might want to do halide-style user_asserts (e.g. the
// Extern macros introduce calls to user_assert), so for that purpose
// we define an equivalent macro that can be used outside of Halide.h
#define _halide_user_assert(c)     Halide::Internal::ErrorReport(__FILE__, __LINE__, #c, c, true, false, false)

// N.B. Any function that might throw a user_assert or user_error may
// not be inlined into the user's code, or the line number will be
// misattributed to Halide.h. Either make such functions internal to
// libHalide, or mark them as NO_INLINE.

}

}

#endif
#ifndef HALIDE_IR_VISITOR_H
#define HALIDE_IR_VISITOR_H

#include <set>
#include <map>
#include <string>

/** \file
 * Defines the base class for things that recursively walk over the IR
 */

namespace Halide {

struct Expr;

namespace Internal {

struct IRNode;
struct Stmt;
struct IntImm;
struct FloatImm;
struct StringImm;
struct Cast;
struct Variable;
struct Add;
struct Sub;
struct Mul;
struct Div;
struct Mod;
struct Min;
struct Max;
struct EQ;
struct NE;
struct LT;
struct LE;
struct GT;
struct GE;
struct And;
struct Or;
struct Not;
struct Select;
struct Load;
struct Ramp;
struct Broadcast;
struct Call;
struct Let;
struct LetStmt;
struct AssertStmt;
struct Pipeline;
struct For;
struct Store;
struct Provide;
struct Allocate;
struct Free;
struct Realize;
struct Block;
struct IfThenElse;
struct Evaluate;

class Function;

/** A base class for algorithms that need to recursively walk over the
 * IR. The default implementations just recursively walk over the
 * children. Override the ones you care about.
 */
class IRVisitor {
public:
    virtual ~IRVisitor();
    virtual void visit(const IntImm *);
    virtual void visit(const FloatImm *);
    virtual void visit(const StringImm *);
    virtual void visit(const Cast *);
    virtual void visit(const Variable *);
    virtual void visit(const Add *);
    virtual void visit(const Sub *);
    virtual void visit(const Mul *);
    virtual void visit(const Div *);
    virtual void visit(const Mod *);
    virtual void visit(const Min *);
    virtual void visit(const Max *);
    virtual void visit(const EQ *);
    virtual void visit(const NE *);
    virtual void visit(const LT *);
    virtual void visit(const LE *);
    virtual void visit(const GT *);
    virtual void visit(const GE *);
    virtual void visit(const And *);
    virtual void visit(const Or *);
    virtual void visit(const Not *);
    virtual void visit(const Select *);
    virtual void visit(const Load *);
    virtual void visit(const Ramp *);
    virtual void visit(const Broadcast *);
    virtual void visit(const Call *);
    virtual void visit(const Let *);
    virtual void visit(const LetStmt *);
    virtual void visit(const AssertStmt *);
    virtual void visit(const Pipeline *);
    virtual void visit(const For *);
    virtual void visit(const Store *);
    virtual void visit(const Provide *);
    virtual void visit(const Allocate *);
    virtual void visit(const Free *);
    virtual void visit(const Realize *);
    virtual void visit(const Block *);
    virtual void visit(const IfThenElse *);
    virtual void visit(const Evaluate *);
};

/** A base class for algorithms that walk recursively over the IR
 * without visiting the same node twice. This is for passes that are
 * capable of interpreting the IR as a DAG instead of a tree. */
class IRGraphVisitor : public IRVisitor {
protected:
    /** By default these methods add the node to the visited set, and
     * return whether or not it was already there. If it wasn't there,
     * it delegates to the appropriate visit method. You can override
     * them if you like. */
    // @{
    virtual void include(const Expr &);
    virtual void include(const Stmt &);
    // @}

    /** The nodes visited so far */
    std::set<const IRNode *> visited;

public:

    /** These methods should call 'include' on the children to only
     * visit them if they haven't been visited already. */
    // @{
    virtual void visit(const IntImm *);
    virtual void visit(const FloatImm *);
    virtual void visit(const StringImm *);
    virtual void visit(const Cast *);
    virtual void visit(const Variable *);
    virtual void visit(const Add *);
    virtual void visit(const Sub *);
    virtual void visit(const Mul *);
    virtual void visit(const Div *);
    virtual void visit(const Mod *);
    virtual void visit(const Min *);
    virtual void visit(const Max *);
    virtual void visit(const EQ *);
    virtual void visit(const NE *);
    virtual void visit(const LT *);
    virtual void visit(const LE *);
    virtual void visit(const GT *);
    virtual void visit(const GE *);
    virtual void visit(const And *);
    virtual void visit(const Or *);
    virtual void visit(const Not *);
    virtual void visit(const Select *);
    virtual void visit(const Load *);
    virtual void visit(const Ramp *);
    virtual void visit(const Broadcast *);
    virtual void visit(const Call *);
    virtual void visit(const Let *);
    virtual void visit(const LetStmt *);
    virtual void visit(const AssertStmt *);
    virtual void visit(const Pipeline *);
    virtual void visit(const For *);
    virtual void visit(const Store *);
    virtual void visit(const Provide *);
    virtual void visit(const Allocate *);
    virtual void visit(const Free *);
    virtual void visit(const Realize *);
    virtual void visit(const Block *);
    virtual void visit(const IfThenElse *);
    virtual void visit(const Evaluate *);
    // @}
};

}
}

#endif
#ifndef HALIDE_BUFFER_H
#define HALIDE_BUFFER_H

#include <stdint.h>
#ifndef HALIDE_BUFFER_T_H
#define HALIDE_BUFFER_T_H

/** \file
 * Defines the internal runtime representation of an image: buffer_t
 */

/* Generated code must declare buffer_t as well. This conditional bracket
 * prevents multiple definition errors if both Halide.h and the header
 * generated by compile_to_header are included in the same file.
 *
 * TODO: Use a more unique name than "buffer_t"
 * TODO: If possible, ensure all definitions are the same.
 */
#ifndef BUFFER_T_DEFINED
#define BUFFER_T_DEFINED

#ifndef COMPILING_HALIDE_RUNTIME
#include <stdint.h>
#endif

/**
 * The raw representation of an image passed around by generated
 * Halide code. It includes some stuff to track whether the image is
 * not actually in main memory, but instead on a device (like a
 * GPU). */
typedef struct buffer_t {
    /** A device-handle for e.g. GPU memory used to back this buffer. */
    uint64_t dev;

    /** A pointer to the start of the data in main memory. */
    uint8_t* host;

    /** The size of the buffer in each dimension. */
    int32_t extent[4];

    /** Gives the spacing in memory between adjacent elements in the
     * given dimension.  The correct memory address for a load from
     * this buffer at position x, y, z, w is:
     * host + (x * stride[0] + y * stride[1] + z * stride[2] + w * stride[3]) * elem_size
     * By manipulating the strides and extents you can lazily crop,
     * transpose, and even flip buffers without modifying the data.
    */
    int32_t stride[4];

    /** Buffers often represent evaluation of a Func over some
     * domain. The min field encodes the top left corner of the
     * domain. */
    int32_t min[4];

    /** How many bytes does each buffer element take. This may be
     * replaced with a more general type code in the future. */
    int32_t elem_size;

    /** This should be true if there is an existing device allocation
     * mirroring this buffer, and the data has been modified on the
     * host side. */
    bool host_dirty;

    /** This should be true if there is an existing device allocation
    mirroring this buffer, and the data has been modified on the
    device side. */
    bool dev_dirty;
} buffer_t;

#endif

#endif
#ifndef HALIDE_INTRUSIVE_PTR_H
#define HALIDE_INTRUSIVE_PTR_H

/** \file
 *
 * Support classes for reference-counting via intrusive shared
 * pointers.
 */


#include <stdlib.h>
#include <iostream>
namespace Halide {
namespace Internal {

/** A class representing a reference count to be used with IntrusivePtr */
class RefCount {
    int count;
public:
    RefCount() : count(0) {}
    void increment() {count++;}
    void decrement() {count--;}
    bool is_zero() const {return count == 0;}
};

/**
 * Because in this header we don't yet know how client classes store
 * their RefCount (and we don't want to depend on the declarations of
 * the client classes), any class that you want to hold onto via one
 * of these must provide implementations of ref_count and destroy,
 * which we forward-declare here.
 *
 * E.g. if you want to use IntrusivePtr<MyClass>, then you should
 * define something like this in MyClass.cpp (assuming MyClass has
 * a field: mutable RefCount ref_count):
 *
 * template<> RefCount &ref_count<MyClass>(const MyClass *c) {return c->ref_count;}
 * template<> void destroy<MyClass>(const MyClass *c) {delete c;}
 */
// @{
template<typename T> EXPORT RefCount &ref_count(const T *);
template<typename T> EXPORT void destroy(const T *);
// @}

/** Intrusive shared pointers have a reference count (a
 * RefCount object) stored in the class itself. This is perhaps more
 * efficient than storing it externally, but more importantly, it
 * means it's possible to recover a reference-counted handle from the
 * raw pointer, and it's impossible to have two different reference
 * counts attached to the same raw object. Seeing as we pass around
 * raw pointers to concrete IRNodes and Expr's interchangeably, this
 * is a useful property.
 */
template<typename T>
struct IntrusivePtr {
private:

    void incref(T *p) {
        if (p) {
            ref_count(p).increment();
        }
    };

    void decref(T *p) {
        if (p) {
            // Note that if the refcount is already zero, then we're
            // in a recursive destructor due to a self-reference (a
            // cycle), where the ref_count has been adjusted to remove
            // the counts due to the cycle. The next line then makes
            // the ref_count negative, which prevents actually
            // entering the destructor recursively.
            ref_count(p).decrement();
            if (ref_count(p).is_zero()) {
                destroy(p);
            }
        }
    }

public:
    T *ptr;

    ~IntrusivePtr() {
        decref(ptr);
    }

    IntrusivePtr() : ptr(NULL) {
    }

    IntrusivePtr(T *p) : ptr(p) {
        incref(ptr);
    }

    IntrusivePtr(const IntrusivePtr<T> &other) : ptr(other.ptr) {
        incref(ptr);
    }

    IntrusivePtr<T> &operator=(const IntrusivePtr<T> &other) {
        // Other can be inside of something owned by this, so we
        // should be careful to incref other before we decref
        // ourselves.
        T *temp = other.ptr;
        incref(temp);
        decref(ptr);
        ptr = temp;
        return *this;
    }

    /* Handles can be null. This checks that. */
    bool defined() const {
        return ptr != NULL;
    }

    /* Check if two handles point to the same ptr. This is
     * equality of reference, not equality of value. */
    bool same_as(const IntrusivePtr &other) const {
        return ptr == other.ptr;
    }

};

}
}

#endif

/** \file
 * Defines Buffer - A c++ wrapper around a buffer_t.
 */

namespace Halide {
namespace Internal {
struct BufferContents;
struct JITCompiledModule;
}

/** The internal representation of an image, or other dense array
 * data. The Image type provides a typed view onto a buffer for the
 * purposes of direct manipulation. A buffer may be stored in main
 * memory, or some other memory space (e.g. a gpu). If you want to use
 * this as an Image, see the Image class. Casting a Buffer to an Image
 * will do any appropriate copy-back. This class is a fairly thin
 * wrapper on a buffer_t, which is the C-style type Halide uses for
 * passing buffers around.
 */
class Buffer {
private:
    Internal::IntrusivePtr<Internal::BufferContents> contents;

public:
    Buffer() : contents(NULL) {}

    EXPORT Buffer(Type t, int x_size = 0, int y_size = 0, int z_size = 0, int w_size = 0,
                  uint8_t* data = NULL, const std::string &name = "");

    EXPORT Buffer(Type t, const std::vector<int32_t> &sizes,
                  uint8_t* data = NULL, const std::string &name = "");

    EXPORT Buffer(Type t, const buffer_t *buf, const std::string &name = "");

    /** Get a pointer to the host-side memory. */
    EXPORT void *host_ptr() const;

    /** Get a pointer to the raw buffer_t struct that this class wraps. */
    EXPORT buffer_t *raw_buffer() const;

    /** Get the device-side pointer/handle for this buffer. Will be
     * zero if no device was involved in the creation of this
     * buffer. */
    EXPORT uint64_t device_handle() const;

    /** Has this buffer been modified on the cpu since last copied to a
     * device. Not meaningful unless there's a device involved. */
    EXPORT bool host_dirty() const;

    /** Let Halide know that the host-side memory backing this buffer
     * has been externally modified. You shouldn't normally need to
     * call this, because it is done for you when you cast a Buffer to
     * an Image in order to modify it. */
    EXPORT void set_host_dirty(bool dirty = true);

    /** Has this buffer been modified on device since last copied to
     * the cpu. Not meaninful unless there's a device involved. */
    EXPORT bool device_dirty() const;

    /** Let Halide know that the device-side memory backing this
     * buffer has been externally modified, and so the cpu-side memory
     * is invalid. A copy-back will occur the next time you cast this
     * Buffer to an Image, or the next time this buffer is accessed on
     * the host in a halide pipeline. */
    EXPORT void set_device_dirty(bool dirty = true);

    /** Get the dimensionality of this buffer. Uses the convention
     * that the extent field of a buffer_t should contain zero when
     * the dimensions end. */
    EXPORT int dimensions() const;

    /** Get the extent of this buffer in the given dimension. */
    EXPORT int extent(int dim) const;

    /** Get the number of bytes between adjacent elements of this buffer along the given dimension. */
    EXPORT int stride(int dim) const;

    /** Get the coordinate in the function that this buffer represents
     * that corresponds to the base address of the buffer. */
    EXPORT int min(int dim) const;

    /** Set the coordinate in the function that this buffer represents
     * that corresponds to the base address of the buffer. */
    EXPORT void set_min(int m0, int m1 = 0, int m2 = 0, int m3 = 0);

    /** Get the Halide type of the contents of this buffer. */
    EXPORT Type type() const;

    /** Compare two buffers for identity (not equality of data). */
    EXPORT bool same_as(const Buffer &other) const;

    /** Check if this buffer handle actually points to data. */
    EXPORT bool defined() const;

    /** Get the runtime name of this buffer used for debugging. */
    EXPORT const std::string &name() const;

    /** Convert this buffer to an argument to a halide pipeline. */
    EXPORT operator Argument() const;

    /** Declare that this buffer was created by the given jit-compiled
     * module. Used internally for reference counting the module. */
    EXPORT void set_source_module(const Internal::JITCompiledModule &module);

    /** If this buffer was the output of a jit-compiled realization,
     * retrieve the module it came from. Otherwise returns a module
     * struct full of null pointers. */
    EXPORT const Internal::JITCompiledModule &source_module();

    /** If this buffer was created *on-device* by a jit-compiled
     * realization, then copy it back to the cpu-side memory. This is
     * usually achieved by casting the Buffer to an Image. */
    EXPORT int copy_to_host();

    /** If this buffer was created by a jit-compiled realization on a
     * device-aware target (e.g. PTX), then copy the cpu-side data to
     * the device-side allocation. TODO: I believe this currently
     * aborts messily if no device-side allocation exists. You might
     * think you want to do this because you've modified the data
     * manually on the host before calling another Halide pipeline,
     * but what you actually want to do in that situation is set the
     * host_dirty bit so that Halide can manage the copy lazily for
     * you. Casting the Buffer to an Image sets the dirty bit for
     * you. */
    EXPORT int copy_to_dev();

    /** If this buffer was created by a jit-compiled realization on a
     * device-aware target (e.g. PTX), then free the device-side
     * allocation, if there is one. Done automatically when the last
     * reference to this buffer dies. */
    EXPORT int free_dev_buffer();

};

}

#endif

namespace Halide {

namespace Internal {

/** A class representing a type of IR node (e.g. Add, or Mul, or
 * For). We use it for rtti (without having to compile with rtti). */
struct IRNodeType {};

/** The abstract base classes for a node in the Halide IR. */
struct IRNode {

    /** We use the visitor pattern to traverse IR nodes throughout the
     * compiler, so we have a virtual accept method which accepts
     * visitors.
     */
    virtual void accept(IRVisitor *v) const = 0;
    IRNode() {}
    virtual ~IRNode() {}

    /** These classes are all managed with intrusive reference
       counting, so we also track a reference count. It's mutable
       so that we can do reference counting even through const
       references to IR nodes. */
    mutable RefCount ref_count;

    /** Each IR node subclass should return some unique pointer. We
     * can compare these pointers to do runtime type
     * identification. We don't compile with rtti because that
     * injects run-time type identification stuff everywhere (and
     * often breaks when linking external libraries compiled
     * without it), and we only want it for IR nodes. */
    virtual const IRNodeType *type_info() const = 0;
};

template<>
EXPORT inline RefCount &ref_count<IRNode>(const IRNode *n) {return n->ref_count;}

template<>
EXPORT inline void destroy<IRNode>(const IRNode *n) {delete n;}

/** IR nodes are split into expressions and statements. These are
   similar to expressions and statements in C - expressions
   represent some value and have some type (e.g. x + 3), and
   statements are side-effecting pieces of code that do not
   represent a value (e.g. assert(x > 3)) */

/** A base class for statement nodes. They have no properties or
   methods beyond base IR nodes for now */
struct BaseStmtNode : public IRNode {
};

/** A base class for expression nodes. They all contain their types
 * (e.g. Int(32), Float(32)) */
struct BaseExprNode : public IRNode {
    Type type;
};

/** We use the "curiously recurring template pattern" to avoid
   duplicated code in the IR Nodes. These classes live between the
   abstract base classes and the actual IR Nodes in the
   inheritance hierarchy. It provides an implementation of the
   accept function necessary for the visitor pattern to work, and
   a concrete instantiation of a unique IRNodeType per class. */
template<typename T>
struct ExprNode : public BaseExprNode {
    void accept(IRVisitor *v) const {
        v->visit((const T *)this);
    }
    virtual IRNodeType *type_info() const {return &_type_info;}
    static EXPORT IRNodeType _type_info;
};

template<typename T>
struct StmtNode : public BaseStmtNode {
    void accept(IRVisitor *v) const {
        v->visit((const T *)this);
    }
    virtual IRNodeType *type_info() const {return &_type_info;}
    static EXPORT IRNodeType _type_info;
};

/** IR nodes are passed around opaque handles to them. This is a
   base class for those handles. It manages the reference count,
   and dispatches visitors. */
struct IRHandle : public IntrusivePtr<const IRNode> {
    IRHandle() : IntrusivePtr<const IRNode>() {}
    IRHandle(const IRNode *p) : IntrusivePtr<const IRNode>(p) {}

    /** Dispatch to the correct visitor method for this node. E.g. if
     * this node is actually an Add node, then this will call
     * IRVisitor::visit(const Add *) */
    void accept(IRVisitor *v) const {
        ptr->accept(v);
    }

    /** Downcast this ir node to its actual type (e.g. Add, or
     * Select). This returns NULL if the node is not of the requested
     * type. Example usage:
     *
     * if (const Add *add = node->as<Add>()) {
     *   // This is an add node
     * }
     */
    template<typename T> const T *as() const {
        if (ptr->type_info() == &T::_type_info) {
            return (const T *)ptr;
        }
        return NULL;
    }
};

/** Integer constants */
struct IntImm : public ExprNode<IntImm> {
    int value;

    static IntImm *make(int value) {
        if (value >= -8 && value <= 8 &&
            !small_int_cache[value + 8].ref_count.is_zero()) {
            return &small_int_cache[value + 8];
        }
        IntImm *node = new IntImm;
        node->type = Int(32);
        node->value = value;
        return node;
    }

private:
    /** ints from -8 to 8 */
    static IntImm small_int_cache[17];
};

/** Floating point constants */
struct FloatImm : public ExprNode<FloatImm> {
    float value;

    static FloatImm *make(float value) {
        FloatImm *node = new FloatImm;
        node->type = Float(32);
        node->value = value;
        return node;
    }
};

/** String constants */
struct StringImm : public ExprNode<StringImm> {
    std::string value;

    static StringImm *make(const std::string &val) {
        StringImm *node = new StringImm;
        node->type = Handle();
        node->value = val;
        return node;
    }
};

}

/** A fragment of Halide syntax. It's implemented as reference-counted
 * handle to a concrete expression node, but it's immutable, so you
 * can treat it as a value type. */
struct Expr : public Internal::IRHandle {
    /** Make an undefined expression */
    Expr() : Internal::IRHandle() {}

    /** Make an expression from a concrete expression node pointer (e.g. Add) */
    Expr(const Internal::BaseExprNode *n) : IRHandle(n) {}


    /** Make an expression representing a const 32-bit int (i.e. an IntImm) */
    EXPORT Expr(int x) : IRHandle(Internal::IntImm::make(x)) {
    }

    /** Make an expression representing a const 32-bit float (i.e. a FloatImm) */
    EXPORT Expr(float x) : IRHandle(Internal::FloatImm::make(x)) {
    }

    /** Make an expression representing a const 32-bit float, given a
     * double. Also emits a warning due to truncation. */
    EXPORT Expr(double x) : IRHandle(Internal::FloatImm::make((float)x)) {
        user_warning << "Halide cannot represent double constants. "
                     << "Converting " << x << " to float. "
                     << "If you wanted a double, use cast<double>(" << x
                     << (x == (int64_t)(x) ? ".0f" : "f")
                     << ")\n";
    }

    /** Make an expression representing a const string (i.e. a StringImm) */
    EXPORT Expr(const std::string &s) : IRHandle(Internal::StringImm::make(s)) {
    }

    /** Get the type of this expression node */
    Type type() const {
        return ((const Internal::BaseExprNode *)ptr)->type;
    }
};

/** This lets you use an Expr as a key in a map of the form
 * map<Expr, Foo, ExprCompare> */
struct ExprCompare {
    bool operator()(Expr a, Expr b) const {
        return a.ptr < b.ptr;
    }
};

}

// Now that we've defined an Expr, we can include Parameter.h
#ifndef HALIDE_PARAMETER_H
#define HALIDE_PARAMETER_H

/** \file
 * Defines the internal representation of parameters to halide piplines
 */

#include <string>

namespace Halide {
namespace Internal {

struct ParameterContents;

/** A reference-counted handle to a parameter to a halide
 * pipeline. May be a scalar parameter or a buffer */
class Parameter {
    IntrusivePtr<ParameterContents> contents;

    void check_defined() const;
    void check_is_buffer() const;
    void check_is_scalar() const;
    void check_dim_ok(int dim) const;

public:
    /** Construct a new undefined handle */
    Parameter() : contents(NULL) {}

    /** Construct a new parameter of the given type. If the second
     * argument is true, this is a buffer parameter, otherwise, it is
     * a scalar parameter. The parameter will be given a unique
     * auto-generated name. */
    EXPORT Parameter(Type t, bool is_buffer);

    /** Construct a new parameter of the given type with name given by
     * the third argument. If the second argument is true, this is a
     * buffer parameter, otherwise, it is a scalar parameter. The
     * parameter will be given a unique auto-generated name. */
    EXPORT Parameter(Type t, bool is_buffer, const std::string &name);

    /** Get the type of this parameter */
    EXPORT Type type() const;

    /** Get the name of this parameter */
    EXPORT const std::string &name() const;

    /** Does this parameter refer to a buffer/image? */
    EXPORT bool is_buffer() const;

    /** If the parameter is a scalar parameter, get its currently
     * bound value. Only relevant when jitting */
    template<typename T>
    NO_INLINE T get_scalar() {
        user_assert(type() == type_of<T>())
            << "Can't get Param<" << type()
            << "> as scalar of type " << type_of<T>() << "\n";
        return *((T *)(get_scalar_address()));
    }

    /** If the parameter is a scalar parameter, set its current
     * value. Only relevant when jitting */
    template<typename T>
    NO_INLINE void set_scalar(T val) {
        user_assert(type() == type_of<T>())
            << "Can't set Param<" << type()
            << "> to scalar of type " << type_of<T>() << "\n";
        *((T *)(get_scalar_address())) = val;
    }

    /** If the parameter is a buffer parameter, get its currently
     * bound buffer. Only relevant when jitting */
    EXPORT Buffer get_buffer() const;

    /** If the parameter is a buffer parameter, set its current
     * value. Only relevant when jitting */
    EXPORT void set_buffer(Buffer b);

    /** Get the pointer to the current value of the scalar
     * parameter. For a given parameter, this address will never
     * change. Only relevant when jitting. */
    EXPORT void *get_scalar_address() const;

    /** Tests if this handle is the same as another handle */
    EXPORT bool same_as(const Parameter &other) const;

    /** Tests if this handle is non-NULL */
    EXPORT bool defined() const;

    /** Get and set constraints for the min, extent, and stride (see
     * ImageParam::set_extent) */
    //@{
    EXPORT void set_min_constraint(int dim, Expr e);
    EXPORT void set_extent_constraint(int dim, Expr e);
    EXPORT void set_stride_constraint(int dim, Expr e);
    EXPORT Expr min_constraint(int dim) const;
    EXPORT Expr extent_constraint(int dim) const;
    EXPORT Expr stride_constraint(int dim) const;
    //@}

    /** Get and set constraints for scalar parameters. These are used
     * directly by Param, so they must be exported. */
    // @{
    EXPORT void set_min_value(Expr e);
    EXPORT Expr get_min_value();
    EXPORT void set_max_value(Expr e);
    EXPORT Expr get_max_value();
    // @}
};

/** Validate arguments to a call to a func, image or imageparam. */
void check_call_arg_types(const std::string &name, std::vector<Expr> *args, int dims);

}
}

#endif

namespace Halide {
namespace Internal {

/** A reference-counted handle to a statement node. */
struct Stmt : public IRHandle {
    Stmt() : IRHandle() {}
    Stmt(const BaseStmtNode *n) : IRHandle(n) {}

    /** This lets you use a Stmt as a key in a map of the form
     * map<Stmt, Foo, Stmt::Compare> */
    struct Compare {
        bool operator()(const Stmt &a, const Stmt &b) const {
            return a.ptr < b.ptr;
        }
    };
};

/** The actual IR nodes begin here. Remember that all the Expr
 * nodes also have a public "type" property */

}

namespace Internal {

/** Cast a node from one type to another */
struct Cast : public ExprNode<Cast> {
    Expr value;

    EXPORT static Expr make(Type t, Expr v);
};

/** The sum of two expressions */
struct Add : public ExprNode<Add> {
    Expr a, b;

    EXPORT static Expr make(Expr a, Expr b);
};

/** The difference of two expressions */
struct Sub : public ExprNode<Sub> {
    Expr a, b;

    EXPORT static Expr make(Expr a, Expr b);
};

/** The product of two expressions */
struct Mul : public ExprNode<Mul> {
    Expr a, b;

    EXPORT static Expr make(Expr a, Expr b);
};

/** The ratio of two expressions */
struct Div : public ExprNode<Div> {
    Expr a, b;

    EXPORT static Expr make(Expr a, Expr b);
};

/** The remainder of a / b. Mostly equivalent to '%' in C, except that
 * the result here is always positive. For floats, this is equivalent
 * to calling fmod. */
struct Mod : public ExprNode<Mod> {
    Expr a, b;

    EXPORT static Expr make(Expr a, Expr b);
};

/** The lesser of two values. */
struct Min : public ExprNode<Min> {
    Expr a, b;

    EXPORT static Expr make(Expr a, Expr b);
};

/** The greater of two values */
struct Max : public ExprNode<Max> {
    Expr a, b;

    EXPORT static Expr make(Expr a, Expr b);
};

/** Is the first expression equal to the second */
struct EQ : public ExprNode<EQ> {
    Expr a, b;

    EXPORT static Expr make(Expr a, Expr b);
};

/** Is the first expression not equal to the second */
struct NE : public ExprNode<NE> {
    Expr a, b;

    EXPORT static Expr make(Expr a, Expr b);
};

/** Is the first expression less than the second. */
struct LT : public ExprNode<LT> {
    Expr a, b;

    EXPORT static Expr make(Expr a, Expr b);
};

/** Is the first expression less than or equal to the second. */
struct LE : public ExprNode<LE> {
    Expr a, b;

    EXPORT static Expr make(Expr a, Expr b);
};

/** Is the first expression greater than the second. */
struct GT : public ExprNode<GT> {
    Expr a, b;

    EXPORT static Expr make(Expr a, Expr b);
};

/** Is the first expression greater than or equal to the second. */
struct GE : public ExprNode<GE> {
    Expr a, b;

    EXPORT static Expr make(Expr a, Expr b);
};

/** Logical and - are both expressions true */
struct And : public ExprNode<And> {
    Expr a, b;

    EXPORT static Expr make(Expr a, Expr b);
};

/** Logical or - is at least one of the expression true */
struct Or : public ExprNode<Or> {
    Expr a, b;

    EXPORT static Expr make(Expr a, Expr b);
};

/** Logical not - true if the expression false */
struct Not : public ExprNode<Not> {
    Expr a;

    EXPORT static Expr make(Expr a);
};

/** A ternary operator. Evalutes 'true_value' and 'false_value',
 * then selects between them based on 'condition'. Equivalent to
 * the ternary operator in C. */
struct Select : public ExprNode<Select> {
    Expr condition, true_value, false_value;

    EXPORT static Expr make(Expr condition, Expr true_value, Expr false_value);
};

/** Load a value from a named buffer. The buffer is treated as an
 * array of the 'type' of this Load node. That is, the buffer has
 * no inherent type. */
struct Load : public ExprNode<Load> {
    std::string name;

    Expr index;

    // If it's a load from an image argument or compiled-in constant
    // image, this will point to that
    Buffer image;

    // If it's a load from an image parameter, this points to that
    Parameter param;

    EXPORT static Expr make(Type type, std::string name, Expr index, Buffer image, Parameter param);
};

/** A linear ramp vector node. This is vector with 'width' elements,
 * where element i is 'base' + i*'stride'. This is a convenient way to
 * pass around vectors without busting them up into individual
 * elements. E.g. a dense vector load from a buffer can use a ramp
 * node with stride 1 as the index. */
struct Ramp : public ExprNode<Ramp> {
    Expr base, stride;
    int width;

    EXPORT static Expr make(Expr base, Expr stride, int width);
};

/** A vector with 'width' elements, in which every element is
 * 'value'. This is a special case of the ramp node above, in which
 * the stride is zero. */
struct Broadcast : public ExprNode<Broadcast> {
    Expr value;
    int width;

    EXPORT static Expr make(Expr value, int width);
};

/** A let expression, like you might find in a functional
 * language. Within the expression \ref Let::body, instances of the Var
 * node \ref Let::name refer to \ref Let::value. */
struct Let : public ExprNode<Let> {
    std::string name;
    Expr value, body;

    EXPORT static Expr make(std::string name, Expr value, Expr body);
};

/** The statement form of a let node. Within the statement 'body',
 * instances of the Var named 'name' refer to 'value' */
struct LetStmt : public StmtNode<LetStmt> {
    std::string name;
    Expr value;
    Stmt body;

    EXPORT static Stmt make(std::string name, Expr value, Stmt body);
};

/** If the 'condition' is false, then bail out printing the
 * message to stderr */
struct AssertStmt : public StmtNode<AssertStmt> {
    // if condition then val else error out with message
    Expr condition;
    Expr message;

    EXPORT static Stmt make(Expr condition, const char *message);
    EXPORT static Stmt make(Expr condition, Expr message);
    EXPORT static Stmt make(Expr condition, const std::vector<Expr> &message);
};

/** This node is a helpful annotation to do with permissions. The
 * three child statements happen in order. In the 'produce'
 * statement 'buffer' is write-only. In 'update' it is
 * read-write. In 'consume' it is read-only. The 'update' node is
 * often NULL. (check update.defined() to find out). None of this
 * is actually enforced, the node is purely for informative
 * purposes to help out our analysis during lowering. */
struct Pipeline : public StmtNode<Pipeline> {
    std::string name;
    Stmt produce, update, consume;

    EXPORT static Stmt make(std::string name, Stmt produce, Stmt update, Stmt consume);
};

/** A for loop. Execute the 'body' statement for all values of the
 * variable 'name' from 'min' to 'min + extent'. There are four
 * types of For nodes. A 'Serial' for loop is a conventional
 * one. In a 'Parallel' for loop, each iteration of the loop
 * happens in parallel or in some unspecified order. In a
 * 'Vectorized' for loop, each iteration maps to one SIMD lane,
 * and the whole loop is executed in one shot. For this case,
 * 'extent' must be some small integer constant (probably 4, 8, or
 * 16). An 'Unrolled' for loop compiles to a completely unrolled
 * version of the loop. Each iteration becomes its own
 * statement. Again in this case, 'extent' should be a small
 * integer constant. */
struct For : public StmtNode<For> {
    std::string name;
    Expr min, extent;
    typedef enum {Serial, Parallel, Vectorized, Unrolled} ForType;
    ForType for_type;
    Stmt body;

    EXPORT static Stmt make(std::string name, Expr min, Expr extent, ForType for_type, Stmt body);
};

/** Store a 'value' to the buffer called 'name' at a given
 * 'index'. The buffer is interpreted as an array of the same type as
 * 'value'. */
struct Store : public StmtNode<Store> {
    std::string name;
    Expr value, index;

    EXPORT static Stmt make(std::string name, Expr value, Expr index);
};

/** This defines the value of a function at a multi-dimensional
 * location. You should think of it as a store to a
 * multi-dimensional array. It gets lowered to a conventional
 * Store node. */
struct Provide : public StmtNode<Provide> {
    std::string name;
    std::vector<Expr> values;
    std::vector<Expr> args;

    EXPORT static Stmt make(std::string name, const std::vector<Expr> &values, const std::vector<Expr> &args);
};

/** Allocate a scratch area called with the given name, type, and
 * size. The buffer lives for at most the duration of the body
 * statement, within which it is freed. It is an error for an allocate
 * node not to contain a free node of the same buffer. */
struct Allocate : public StmtNode<Allocate> {
    std::string name;
    Type type;
    std::vector<Expr> extents;
    Expr condition;
    Stmt body;

    EXPORT static Stmt make(std::string name, Type type, const std::vector<Expr> &extents,
                            Expr condition, Stmt body);
};

/** Free the resources associated with the given buffer. */
struct Free : public StmtNode<Free> {
    std::string name;

    static Stmt make(std::string name);
};

/** A single-dimensional span. Includes all numbers between min and
 * (min + extent - 1) */
struct Range {
    Expr min, extent;
    Range() {}
    Range(Expr min, Expr extent) : min(min), extent(extent) {
        internal_assert(min.type() == extent.type()) << "Region min and extent must have same type\n";
    }
};

/** A multi-dimensional box. The outer product of the elements */
typedef std::vector<Range> Region;

/** Allocate a multi-dimensional buffer of the given type and
 * size. Create some scratch memory that will back the function 'name'
 * over the range specified in 'bounds'. The bounds are a vector of
 * (min, extent) pairs for each dimension. */
struct Realize : public StmtNode<Realize> {
    std::string name;
    std::vector<Type> types;
    Region bounds;
    Expr condition;
    Stmt body;

    EXPORT static Stmt make(const std::string &name, const std::vector<Type> &types, const Region &bounds, Expr condition, Stmt body);
};

/** A sequence of statements to be executed in-order. 'rest' may be
 * NULL. Used rest.defined() to find out. */
struct Block : public StmtNode<Block> {
    Stmt first, rest;

    EXPORT static Stmt make(Stmt first, Stmt rest);
};

/** An if-then-else block. 'else' may be NULL. */
struct IfThenElse : public StmtNode<IfThenElse> {
    Expr condition;
    Stmt then_case, else_case;

    EXPORT static Stmt make(Expr condition, Stmt then_case, Stmt else_case = Stmt());
};

/** Evaluate and discard an expression, presumably because it has some side-effect. */
struct Evaluate : public StmtNode<Evaluate> {
    Expr value;

    EXPORT static Stmt make(Expr v);
};

}
}
// Now that we've defined an Expr and ForType, we can include the definition of a function
#ifndef HALIDE_FUNCTION_H
#define HALIDE_FUNCTION_H

/** \file
 * Defines the internal representation of a halide function and related classes
 */

#ifndef HALIDE_SCHEDULE_H
#define HALIDE_SCHEDULE_H

/** \file
 * Defines the internal representation of the schedule for a function
 */

#include <string>
#include <vector>

namespace Halide {
namespace Internal {

/** A reference to a site in a Halide statement at the top of the
 * body of a particular for loop. Evaluating a region of a halide
 * function is done by generating a loop nest that spans its
 * dimensions. We schedule the inputs to that function by
 * recursively injecting realizations for them at particular sites
 * in this loop nest. A LoopLevel identifies such a site. */
struct LoopLevel {
    std::string func, var;

    /** Identify the loop nest corresponding to some dimension of some function */
    LoopLevel(std::string f, std::string v) : func(f), var(v) {}

    /** Construct an empty LoopLevel, which is interpreted as
     * 'inline'. This is a special LoopLevel value that implies
     * that a function should be inlined away */
    LoopLevel() {}

    /** Test if a loop level corresponds to inlining the function */
    bool is_inline() const {return var.empty();}

    /** root is a special LoopLevel value which represents the
     * location outside of all for loops */
    static LoopLevel root() {
        return LoopLevel("", "__root");
    }
    /** Test if a loop level is 'root', which describes the site
     * outside of all for loops */
    bool is_root() const {return var == "__root";}

    /** Compare this loop level against the variable name of a for
     * loop, to see if this loop level refers to the site
     * immediately inside this loop. */
    bool match(const std::string &loop) const {
        return starts_with(loop, func + ".") && ends_with(loop, "." + var);
    }

    bool match(const LoopLevel &other) const {
        return (func == other.func &&
                (var == other.var ||
                 ends_with(var, "." + other.var) ||
                 ends_with(other.var, "." + var)));
    }

    /** Check if two loop levels are exactly the same. */
    bool operator==(const LoopLevel &other) const {
        return func == other.func && var == other.var;
    }

};

struct Split {
    std::string old_var, outer, inner;
    Expr factor;
    bool exact; // Is it required that the factor divides the extent of the old var. True for splits of RVars.

    enum SplitType {SplitVar = 0, RenameVar, FuseVars};

    // If split_type is Rename, then this is just a renaming of the
    // old_var to the outer and not a split. The inner var should
    // be ignored, and factor should be one. Renames are kept in
    // the same list as splits so that ordering between them is
    // respected.

    // If split_type is Fuse, then this does the opposite of a
    // split, it joins the outer and inner into the old_var.
    SplitType split_type;

    bool is_rename() const {return split_type == RenameVar;}
    bool is_split() const {return split_type == SplitVar;}
    bool is_fuse() const {return split_type == FuseVars;}
};

struct Dim {
    std::string var;
    For::ForType for_type;
    bool pure;
};

struct Bound {
    std::string var;
    Expr min, extent;
};

struct ScheduleContents;

struct Specialization {
    Expr condition;
    IntrusivePtr<ScheduleContents> schedule;
};

class ReductionDomain;

/** A schedule for a single stage of a Halide pipeline. Right now this
 * interface is basically a struct, offering mutable access to its
 * innards. In the future it may become more encapsulated. */
class Schedule {
    IntrusivePtr<ScheduleContents> contents;
public:

    Schedule(IntrusivePtr<ScheduleContents> c) : contents(c) {}
    Schedule(const Schedule &other) : contents(other.contents) {}
    EXPORT Schedule();

    /** This flag is set to true if the schedule is memoized. */
    // @{
    bool &memoized();
    bool memoized() const;
    // @}

    /** This flag is set to true if the dims list has been manipulated
     * by the user (or if a ScheduleHandle was created that could have
     * been used to manipulate it). It controls the warning that
     * occurs if you schedule the vars of the pure step but not the
     * update steps. */
    // @{
    bool &touched();
    bool touched() const;
    // @}

    /** The traversal of the domain of a function can have some of its
     * dimensions split into sub-dimensions. See ScheduleHandle::split */
    // @{
    const std::vector<Split> &splits() const;
    std::vector<Split> &splits();
    // @}

    /** The list and ordering of dimensions used to evaluate this
     * function, after all splits have taken place. The first
     * dimension in the vector corresponds to the innermost for loop,
     * and the last is the outermost. Also specifies what type of for
     * loop to use for each dimension. Does not specify the bounds on
     * each dimension. These get inferred from how the function is
     * used, what the splits are, and any optional bounds in the list below. */
    // @{
    const std::vector<Dim> &dims() const;
    std::vector<Dim> &dims();
    // @}

    /** Any reduction domain associated with this schedule. */
    // @{
    const ReductionDomain &reduction_domain() const;
    void set_reduction_domain(const ReductionDomain &d);
    // @}

    /** The list and order of dimensions used to store this
     * function. The first dimension in the vector corresponds to the
     * innermost dimension for storage (i.e. which dimension is
     * tightly packed in memory) */
    // @{
    const std::vector<std::string> &storage_dims() const;
    std::vector<std::string> &storage_dims();
    // @}

    /** You may explicitly bound some of the dimensions of a
     * function. See \ref Func::bound */
    // @{
    const std::vector<Bound> &bounds() const;
    std::vector<Bound> &bounds();
    // @}

    /** You may create several specialized versions of a func with
     * different schedules. They trigger when the condition is
     * true. See \ref Func::specialize */
    // @{
    const std::vector<Specialization> &specializations() const;
    const Specialization &add_specialization(Expr condition);
    //std::vector<Specialization> &specializations();
    // @}

    /** At what sites should we inject the allocation and the
     * computation of this function? The store_level must be outside
     * of or equal to the compute_level. If the compute_level is
     * inline, the store_level is meaningless. See \ref Func::store_at
     * and \ref Func::compute_at */
    // @{
    const LoopLevel &store_level() const;
    const LoopLevel &compute_level() const;
    LoopLevel &store_level();
    LoopLevel &compute_level();
    // @}

    /** Are race conditions permitted? */
    // @{
    bool allow_race_conditions() const;
    bool &allow_race_conditions();
    // @}

};

}
}

#endif
#ifndef HALIDE_REDUCTION_H
#define HALIDE_REDUCTION_H

/** \file
 * Defines internal classes related to Reduction Domains
 */

#include <string>
#include <vector>

namespace Halide {
namespace Internal {

/** A single named dimension of a reduction domain */
struct ReductionVariable {
    std::string var;
    Expr min, extent;
};

struct ReductionDomainContents {
    mutable RefCount ref_count;
    std::vector<ReductionVariable> domain;
};

/** A reference-counted handle on a reduction domain, which is just a
 * vector of ReductionVariable. */
class ReductionDomain {
    IntrusivePtr<ReductionDomainContents> contents;
public:
    /** Construct a new NULL reduction domain */
    ReductionDomain() : contents(NULL) {}

    /** Construct a reduction domain that spans the outer product of
     * all values of the given ReductionVariable in scanline order,
     * with the start of the vector being innermost, and the end of
     * the vector being outermost. */
    ReductionDomain(const std::vector<ReductionVariable> &domain) :
        contents(new ReductionDomainContents) {
        contents.ptr->domain = domain;
    }

    /** Is this handle non-NULL */
    bool defined() const {
        return contents.defined();
    }

    /** Tests for equality of reference. Only one reduction domain is
     * allowed per reduction function, and this is used to verify
     * that */
    bool same_as(const ReductionDomain &other) const {
        return contents.same_as(other.contents);
    }

    /** Immutable access to the reduction variables. */
    const std::vector<ReductionVariable> &domain() const {
        return contents.ptr->domain;
    }
};

}
}

#endif

#include <string>
#include <vector>
#include <sstream>

namespace Halide {

namespace Internal {
struct FunctionContents;
}

/** An argument to an extern-defined Func. May be a Function, Buffer,
 * ImageParam or Expr. */
struct ExternFuncArgument {
    enum ArgType {UndefinedArg = 0, FuncArg, BufferArg, ExprArg, ImageParamArg};
    ArgType arg_type;
    Internal::IntrusivePtr<Internal::FunctionContents> func;
    Buffer buffer;
    Expr expr;
    Internal::Parameter image_param;

    ExternFuncArgument(Internal::IntrusivePtr<Internal::FunctionContents> f): arg_type(FuncArg), func(f) {}

    ExternFuncArgument(Buffer b): arg_type(BufferArg), buffer(b) {}

    ExternFuncArgument(Expr e): arg_type(ExprArg), expr(e) {}

    ExternFuncArgument(Internal::Parameter p) : arg_type(ImageParamArg), image_param(p) {
        // Scalar params come in via the Expr constructor.
        internal_assert(p.is_buffer());
    }
    ExternFuncArgument() : arg_type(UndefinedArg) {}

    bool is_func() const {return arg_type == FuncArg;}
    bool is_expr() const {return arg_type == ExprArg;}
    bool is_buffer() const {return arg_type == BufferArg;}
    bool is_image_param() const {return arg_type == ImageParamArg;}
    bool defined() const {return arg_type != UndefinedArg;}
};

namespace Internal {

struct UpdateDefinition {
    std::vector<Expr> values, args;
    Schedule schedule;
    ReductionDomain domain;
};

struct FunctionContents {
    mutable RefCount ref_count;
    std::string name;
    std::vector<std::string> args;
    std::vector<Expr> values;
    std::vector<Type> output_types;
    Schedule schedule;

    std::vector<UpdateDefinition> updates;

    std::string debug_file;

    std::vector<Parameter> output_buffers;

    std::vector<ExternFuncArgument> extern_arguments;
    std::string extern_function_name;

    bool trace_loads, trace_stores, trace_realizations;

    bool frozen;

    FunctionContents() : trace_loads(false), trace_stores(false), trace_realizations(false), frozen(false) {}
};

/** A reference-counted handle to Halide's internal representation of
 * a function. Similar to a front-end Func object, but with no
 * syntactic sugar to help with definitions. */
class Function {
private:
    IntrusivePtr<FunctionContents> contents;
public:
    /** Construct a new function with no definitions and no name. This
     * constructor only exists so that you can make vectors of
     * functions, etc.
     */
    Function() : contents(new FunctionContents) {}

    /** Reconstruct a Function from a FunctionContents pointer. */
    Function(const IntrusivePtr<FunctionContents> &c) : contents(c) {}

    /** Add a pure definition to this function. It may not already
     * have a definition. All the free variables in 'value' must
     * appear in the args list. 'value' must not depend on any
     * reduction domain */
    void define(const std::vector<std::string> &args, std::vector<Expr> values);

    /** Add an update definition to this function. It must already
     * have a pure definition but not an update definition, and the
     * length of args must match the length of args used in the pure
     * definition. 'value' must depend on some reduction domain, and
     * may contain variables from that domain as well as pure
     * variables. Any pure variables must also appear as Variables in
     * the args array, and they must have the same name as the pure
     * definition's argument in the same index. */
    void define_update(const std::vector<Expr> &args, std::vector<Expr> values);

    /** Construct a new function with the given name */
    Function(const std::string &n) : contents(new FunctionContents) {
        for (size_t i = 0; i < n.size(); i++) {
            user_assert(n[i] != '.')
                << "Func name \"" << n << "\" is invalid. "
                << "Func names may not contain the character '.', "
                << "as it is used internally by Halide as a separator\n";
        }
        contents.ptr->name = n;
    }

    /** Get the name of the function */
    const std::string &name() const {
        return contents.ptr->name;
    }

    /** Get the pure arguments */
    const std::vector<std::string> &args() const {
        return contents.ptr->args;
    }

    /** Get the dimensionality */
    int dimensions() const {
        return (int)args().size();
    }

    /** Get the number of outputs */
    int outputs() const {
        return (int)output_types().size();
    }

    /** Get the types of the outputs */
    const std::vector<Type> &output_types() const {
        return contents.ptr->output_types;
    }

    /** Get the right-hand-side of the pure definition */
    const std::vector<Expr> &values() const {
        return contents.ptr->values;
    }

    /** Does this function have a pure definition */
    bool has_pure_definition() const {
        return !contents.ptr->values.empty();
    }

    /** Does this function *only* have a pure definition */
    bool is_pure() const {
        return (has_pure_definition() &&
                !has_update_definition() &&
                !has_extern_definition());
    }

    /** Get a handle to the schedule for the purpose of modifying
     * it */
    Schedule &schedule() {
        return contents.ptr->schedule;
    }

    /** Get a const handle to the schedule for inspecting it */
    const Schedule &schedule() const {
        return contents.ptr->schedule;
    }

    /** Get a handle on the output buffer used for setting constraints
     * on it. */
    const std::vector<Parameter> &output_buffers() const {
        return contents.ptr->output_buffers;
    }

    /** Get a mutable handle to the schedule for the update
     * stage */
    Schedule &update_schedule(int idx = 0) {
        return contents.ptr->updates[idx].schedule;
    }

    /** Get a const reference to this function's update definitions. */
    const std::vector<UpdateDefinition> &updates() const {
        return contents.ptr->updates;
    }

    /** Does this function have an update definition */
    bool has_update_definition() const {
        return !contents.ptr->updates.empty();
    }

    /** Check if the function has an extern definition */
    bool has_extern_definition() const {
        return !contents.ptr->extern_function_name.empty();
    }

    /** Add an external definition of this Func */
    void define_extern(const std::string &function_name,
                       const std::vector<ExternFuncArgument> &args,
                       const std::vector<Type> &types,
                       int dimensionality);

    /** Retrive the arguments of the extern definition */
    const std::vector<ExternFuncArgument> &extern_arguments() const {
        return contents.ptr->extern_arguments;
    }

    /** Get the name of the extern function called for an extern
     * definition. */
    const std::string &extern_function_name() const {
        return contents.ptr->extern_function_name;
    }

    /** Equality of identity */
    bool same_as(const Function &other) const {
        return contents.same_as(other.contents);
    }

    /** Get a const handle to the debug filename */
    const std::string &debug_file() const {
        return contents.ptr->debug_file;
    }

    /** Get a handle to the debug filename */
    std::string &debug_file() {
        return contents.ptr->debug_file;
    }

    /** Use an an extern argument to another function. */
    operator ExternFuncArgument() const {
        return ExternFuncArgument(contents);
    }

    /** Tracing calls and accessors, passed down from the Func
     * equivalents. */
    // @{
    void trace_loads() {
        contents.ptr->trace_loads = true;
    }
    void trace_stores() {
        contents.ptr->trace_stores = true;
    }
    void trace_realizations() {
        contents.ptr->trace_realizations = true;
    }
    bool is_tracing_loads() {
        return contents.ptr->trace_loads;
    }
    bool is_tracing_stores() {
        return contents.ptr->trace_stores;
    }
    bool is_tracing_realizations() {
        return contents.ptr->trace_realizations;
    }
    // @}

    /** Mark function as frozen, which means it cannot accept new
     * definitions. */
    void freeze() {
        contents.ptr->frozen = true;
    }

    /** Check if a function has been frozen. If so, it is an error to
     * add new definitions. */
    bool frozen() const {
        return contents.ptr->frozen;
    }
};

}}

#endif

// And the definition of a reduction domain

namespace Halide {
namespace Internal {

/** A function call. This can represent a call to some extern
 * function (like sin), but it's also our multi-dimensional
 * version of a Load, so it can be a load from an input image, or
 * a call to another halide function. The latter two types of call
 * nodes don't survive all the way down to code generation - the
 * lowering process converts them to Load nodes. */
struct Call : public ExprNode<Call> {
    std::string name;
    std::vector<Expr> args;
    typedef enum {Image, Extern, Halide, Intrinsic} CallType;
    CallType call_type;

    // Halide uses calls internally to represent certain operations
    // (instead of IR nodes). These are matched by name.
    EXPORT static const std::string debug_to_file,
        shuffle_vector,
        interleave_vectors,
        reinterpret,
        bitwise_and,
        bitwise_not,
        bitwise_xor,
        bitwise_or,
        shift_left,
        shift_right,
        abs,
        rewrite_buffer,
        profiling_timer,
        random,
        lerp,
        create_buffer_t,
        extract_buffer_min,
        extract_buffer_max,
        set_host_dirty,
        set_dev_dirty,
        popcount,
        count_leading_zeros,
        count_trailing_zeros,
        undef,
        null_handle,
        address_of,
        return_second,
        if_then_else,
        trace,
        trace_expr,
        glsl_texture_load,
        glsl_texture_store,
        make_struct,
        stringify,
        memoize_expr,
        copy_memory;

    // If it's a call to another halide function, this call node
    // holds onto a pointer to that function.
    Function func;

    // If that function has multiple values, which value does this
    // call node refer to?
    int value_index;

    // If it's a call to an image, this call nodes hold a
    // pointer to that image's buffer
    Buffer image;

    // If it's a call to an image parameter, this call node holds a
    // pointer to that
    Parameter param;

    EXPORT static Expr make(Type type, std::string name, const std::vector<Expr> &args, CallType call_type,
                            Function func = Function(), int value_index = 0,
                            Buffer image = Buffer(), Parameter param = Parameter());

    /** Convenience constructor for calls to other halide functions */
    static Expr make(Function func, const std::vector<Expr> &args, int idx = 0) {
        internal_assert(idx >= 0 &&
                        idx < func.outputs())
            << "Value index out of range in call to halide function\n";
        internal_assert(func.has_pure_definition() || func.has_extern_definition())
            << "Call to undefined halide function\n";
        return make(func.output_types()[idx], func.name(), args, Halide, func, idx, Buffer(), Parameter());
    }

    /** Convenience constructor for loads from concrete images */
    static Expr make(Buffer image, const std::vector<Expr> &args) {
        return make(image.type(), image.name(), args, Image, Function(), 0, image, Parameter());
    }

    /** Convenience constructor for loads from images parameters */
    static Expr make(Parameter param, const std::vector<Expr> &args) {
        return make(param.type(), param.name(), args, Image, Function(), 0, Buffer(), param);
    }

};

/** A named variable. Might be a loop variable, function argument,
 * parameter, reduction variable, or something defined by a Let or
 * LetStmt node. */
struct Variable : public ExprNode<Variable> {
    std::string name;

    /** References to scalar parameters, or to the dimensions of buffer
     * parameters hang onto those expressions. */
    Parameter param;

    /** References to properties of literal image parameters. */
    Buffer image;

    /** Reduction variables hang onto their domains */
    ReductionDomain reduction_domain;

    static Expr make(Type type, std::string name) {
        return make(type, name, Buffer(), Parameter(), ReductionDomain());
    }

    static Expr make(Type type, std::string name, Parameter param) {
        return make(type, name, Buffer(), param, ReductionDomain());
    }

    static Expr make(Type type, std::string name, Buffer image) {
        return make(type, name, image, Parameter(), ReductionDomain());
    }

    static Expr make(Type type, std::string name, ReductionDomain reduction_domain) {
        return make(type, name, Buffer(), Parameter(), reduction_domain);
    }

    EXPORT static Expr make(Type type, std::string name, Buffer image, Parameter param, ReductionDomain reduction_domain);
};

}
}

#endif
#ifndef HALIDE_IR_OPERATOR_H
#define HALIDE_IR_OPERATOR_H

/** \file
 *
 * Defines various operator overloads and utility functions that make
 * it more pleasant to work with Halide expressions.
 */


namespace Halide {

namespace Internal {
/** Is the expression either an IntImm, a FloatImm, or a Cast of the
 * same, or a Ramp or Broadcast of the same. Doesn't do any constant
 * folding. */
EXPORT bool is_const(Expr e);

/** Is the expression an IntImm, FloatImm of a particular value, or a
 * Cast, or Broadcast of the same. */
EXPORT bool is_const(Expr e, int v);

/** If an expression is an IntImm, return a pointer to its
 * value. Otherwise returns NULL. */
EXPORT const int *as_const_int(Expr e);

/** If an expression is a FloatImm, return a pointer to its
 * value. Otherwise returns NULL. */
EXPORT const float *as_const_float(Expr e);

/** Is the expression a constant integer power of two. Also returns
 * log base two of the expression if it is. */
EXPORT bool is_const_power_of_two(Expr e, int *bits);

/** Is the expression a const (as defined by is_const), and also
 * strictly greater than zero (in all lanes, if a vector expression) */
EXPORT bool is_positive_const(Expr e);

/** Is the expression a const (as defined by is_const), and also
 * strictly less than zero (in all lanes, if a vector expression) */
EXPORT bool is_negative_const(Expr e);

/** Is the expression a const (as defined by is_const), and also
 * strictly less than zero (in all lanes, if a vector expression) and
 * is its negative value representable. (This excludes the most
 * negative value of the Expr's type from inclusion. Intended to be
 * used when the value will be negated as part of simplification.)
 */
EXPORT bool is_negative_negatable_const(Expr e);

/** Is the expression a const (as defined by is_const), and also equal
 * to zero (in all lanes, if a vector expression) */
EXPORT bool is_zero(Expr e);

/** Is the expression a const (as defined by is_const), and also equal
 * to one (in all lanes, if a vector expression) */
EXPORT bool is_one(Expr e);

/** Is the expression a const (as defined by is_const), and also equal
 * to two (in all lanes, if a vector expression) */
EXPORT bool is_two(Expr e);

/** Given an integer value, cast it into a designated integer type
 * and return the bits as int. Unsigned types are returned as bits in the int
 * and should be cast to unsigned int for comparison.
 * int_cast_constant implements bit manipulations to wrap val into the
 * value range of the Type t.
 * For example, int_cast_constant(UInt(16), -1) returns 65535
 * int_cast_constant(Int(8), 128) returns -128
 */
EXPORT int int_cast_constant(Type t, int val);

/** Construct a const of the given type */
EXPORT Expr make_const(Type t, int val);

/** Construct a boolean constant from a C++ boolean value.
 * May also be a vector if width is given.
 * It is not possible to coerce a C++ boolean to Expr because
 * if we provide such a path then char objects can ambiguously
 * be converted to Halide Expr or to std::string.  The problem
 * is that C++ does not have a real bool type - it is in fact
 * close enough to char that C++ does not know how to distinguish them.
 * make_bool is the explicit coercion. */
EXPORT Expr make_bool(bool val, int width = 1);

/** Construct the representation of zero in the given type */
EXPORT Expr make_zero(Type t);

/** Construct the representation of one in the given type */
EXPORT Expr make_one(Type t);

/** Construct the representation of two in the given type */
EXPORT Expr make_two(Type t);

/** Construct the constant boolean true. May also be a vector of
 * trues, if a width argument is given. */
EXPORT Expr const_true(int width = 1);

/** Construct the constant boolean false. May also be a vector of
 * falses, if a width argument is given. */
EXPORT Expr const_false(int width = 1);

/** Coerce the two expressions to have the same type, using C-style
 * casting rules. For the purposes of casting, a boolean type is
 * UInt(1). We use the following procedure:
 *
 * If the types already match, do nothing.
 *
 * Then, if one type is a vector and the other is a scalar, the scalar
 * is broadcast to match the vector width, and we continue.
 *
 * Then, if one type is floating-point and the other is not, the
 * non-float is cast to the floating-point type, and we're done.
 *
 * Then, if neither is a float but one of the two is a constant, the
 * constant is cast to match the non-const type and we're done. For
 * example, e has type UInt(8), then (e*32) also has type UInt(8),
 * despite the overflow that may occur. Note that this also means that
 * (e*(-1)) is positive, and is equivalent to (e*255) - i.e. the (-1)
 * is cast to a UInt(8) before the multiplication.
 *
 * Then, if both types are unsigned ints, the one with fewer bits is
 * cast to match the one with more bits and we're done.
 *
 * Then, if both types are signed ints, the one with fewer bits is
 * cast to match the one with more bits and we're done.
 *
 * Finally, if one type is an unsigned int and the other type is a signed
 * int, both are cast to a signed int with the greater of the two
 * bit-widths. For example, matching an Int(8) with a UInt(16) results
 * in an Int(16).
 *
 */
EXPORT void match_types(Expr &a, Expr &b);

/** Halide's vectorizable transcendentals. */
// @{
EXPORT Expr halide_log(Expr a);
EXPORT Expr halide_exp(Expr a);
EXPORT Expr halide_erf(Expr a);
// @}

/** Raise an expression to an integer power by repeatedly multiplying
 * it by itself. */
EXPORT Expr raise_to_integer_power(Expr a, int b);


}

/** Cast an expression to the halide type corresponding to the C++ type T. */
template<typename T>
inline Expr cast(Expr a) {
    return cast(type_of<T>(), a);
}

/** Cast an expression to a new type. */
inline Expr cast(Type t, Expr a) {
    user_assert(a.defined()) << "cast of undefined Expr\n";
    if (a.type() == t) return a;

    if (t.is_handle() && !a.type().is_handle()) {
        user_error << "Can't cast \"" << a << "\" to a handle. "
                   << "The only legal cast from scalar types to a handle is: "
                   << "reinterpret(Handle(), cast<uint64_t>(" << a << "));\n";
    } else if (a.type().is_handle() && !t.is_handle()) {
        user_error << "Can't cast handle \"" << a << "\" to type " << t << ". "
                   << "The only legal cast from handles to scalar types is: "
                   << "reinterpret(UInt64(), " << a << ");\n";
    }

    if (t.is_vector()) {
        if (a.type().is_scalar()) {
            return Internal::Broadcast::make(cast(t.element_of(), a), t.width);
        } else if (const Internal::Broadcast *b = a.as<Internal::Broadcast>()) {
            internal_assert(b->width == t.width);
            return Internal::Broadcast::make(cast(t.element_of(), b->value), t.width);
        }
    }
    return Internal::Cast::make(t, a);
}

/** Return the sum of two expressions, doing any necessary type
 * coercion using \ref Internal::match_types */
inline Expr operator+(Expr a, Expr b) {
    user_assert(a.defined() && b.defined()) << "operator+ of undefined Expr\n";
    Internal::match_types(a, b);
    return Internal::Add::make(a, b);
}

/** Modify the first expression to be the sum of two expressions,
 * without changing its type. This casts the second argument to match
 * the type of the first. */
inline Expr &operator+=(Expr &a, Expr b) {
    user_assert(a.defined() && b.defined()) << "operator+= of undefined Expr\n";
    a = Internal::Add::make(a, cast(a.type(), b));
    return a;
}

/** Return the difference of two expressions, doing any necessary type
 * coercion using \ref Internal::match_types */
inline Expr operator-(Expr a, Expr b) {
    user_assert(a.defined() && b.defined()) << "operator- of undefined Expr\n";
    Internal::match_types(a, b);
    return Internal::Sub::make(a, b);
}

/** Return the negative of the argument. Does no type casting, so more
 * formally: return that number which when added to the original,
 * yields zero of the same type. For unsigned integers the negative is
 * still an unsigned integer. E.g. in UInt(8), the negative of 56 is
 * 200, because 56 + 200 == 0 */
inline Expr operator-(Expr a) {
    user_assert(a.defined()) << "operator- of undefined Expr\n";
    return Internal::Sub::make(Internal::make_zero(a.type()), a);
}

/** Modify the first expression to be the difference of two expressions,
 * without changing its type. This casts the second argument to match
 * the type of the first. */
inline Expr &operator-=(Expr &a, Expr b) {
    user_assert(a.defined() && b.defined()) << "operator-= of undefined Expr\n";
    a = Internal::Sub::make(a, cast(a.type(), b));
    return a;
}

/** Return the product of two expressions, doing any necessary type
 * coercion using \ref Internal::match_types */
inline Expr operator*(Expr a, Expr b) {
    user_assert(a.defined() && b.defined()) << "operator* of undefined Expr\n";
    Internal::match_types(a, b);
    return Internal::Mul::make(a, b);
}

/** Modify the first expression to be the product of two expressions,
 * without changing its type. This casts the second argument to match
 * the type of the first. */
inline Expr &operator*=(Expr &a, Expr b) {
    user_assert(a.defined() && b.defined()) << "operator*= of undefined Expr\n";
    a = Internal::Mul::make(a, cast(a.type(), b));
    return a;
}

/** Return the ratio of two expressions, doing any necessary type
 * coercion using \ref Internal::match_types */
inline Expr operator/(Expr a, Expr b) {
    user_assert(a.defined() && b.defined()) << "operator/ of undefined Expr\n";
    user_assert(!Internal::is_const(b, 0)) << "operator/ with constant 0 divisor\n";
    Internal::match_types(a, b);
    return Internal::Div::make(a, b);
}

/** Modify the first expression to be the ratio of two expressions,
 * without changing its type. This casts the second argument to match
 * the type of the first. */
inline Expr &operator/=(Expr &a, Expr b) {
    user_assert(a.defined() && b.defined()) << "operator/= of undefined Expr\n";
    user_assert(!Internal::is_const(b, 0)) << "operator/= with constant 0 divisor\n";
    a = Internal::Div::make(a, cast(a.type(), b));
    return a;
}

/** Return the first argument reduced modulo the second, doing any
 * necessary type coercion using \ref Internal::match_types */
inline Expr operator%(Expr a, Expr b) {
    user_assert(a.defined() && b.defined()) << "operator% of undefined Expr\n";
    user_assert(!Internal::is_const(b, 0)) << "operator% with constant 0 modulus\n";
    Internal::match_types(a, b);
    return Internal::Mod::make(a, b);
}

/** Return a boolean expression that tests whether the first argument
 * is greater than the second, after doing any necessary type coercion
 * using \ref Internal::match_types */
inline Expr operator>(Expr a, Expr b) {
    user_assert(a.defined() && b.defined()) << "operator> of undefined Expr\n";
    Internal::match_types(a, b);
    return Internal::GT::make(a, b);
}

/** Return a boolean expression that tests whether the first argument
 * is less than the second, after doing any necessary type coercion
 * using \ref Internal::match_types */
inline Expr operator<(Expr a, Expr b) {
    user_assert(a.defined() && b.defined()) << "operator< of undefined Expr\n";
    Internal::match_types(a, b);
    return Internal::LT::make(a, b);
}

/** Return a boolean expression that tests whether the first argument
 * is less than or equal to the second, after doing any necessary type
 * coercion using \ref Internal::match_types */
inline Expr operator<=(Expr a, Expr b) {
    user_assert(a.defined() && b.defined()) << "operator<= of undefined Expr\n";
    Internal::match_types(a, b);
    return Internal::LE::make(a, b);
}

/** Return a boolean expression that tests whether the first argument
 * is greater than or equal to the second, after doing any necessary
 * type coercion using \ref Internal::match_types */

inline Expr operator>=(Expr a, Expr b) {
    user_assert(a.defined() && b.defined()) << "operator>= of undefined Expr\n";
    Internal::match_types(a, b);
    return Internal::GE::make(a, b);
}

/** Return a boolean expression that tests whether the first argument
 * is equal to the second, after doing any necessary type coercion
 * using \ref Internal::match_types */
inline Expr operator==(Expr a, Expr b) {
    user_assert(a.defined() && b.defined()) << "operator== of undefined Expr\n";
    Internal::match_types(a, b);
    return Internal::EQ::make(a, b);
}

/** Return a boolean expression that tests whether the first argument
 * is not equal to the second, after doing any necessary type coercion
 * using \ref Internal::match_types */
inline Expr operator!=(Expr a, Expr b) {
    user_assert(a.defined() && b.defined()) << "operator!= of undefined Expr\n";
    Internal::match_types(a, b);
    return Internal::NE::make(a, b);
}

/** Returns the logical and of the two arguments */
inline Expr operator&&(Expr a, Expr b) {
    return Internal::And::make(a, b);
}

/** Returns the logical or of the two arguments */
inline Expr operator||(Expr a, Expr b) {
    return Internal::Or::make(a, b);
}

/** Returns the logical not the argument */
inline Expr operator!(Expr a) {
    return Internal::Not::make(a);
}

/** Returns an expression representing the greater of the two
 * arguments, after doing any necessary type coercion using
 * \ref Internal::match_types. Vectorizes cleanly on most platforms
 * (with the exception of integer types on x86 without SSE4). */
inline Expr max(Expr a, Expr b) {
    user_assert(a.defined() && b.defined())
        << "max of undefined Expr\n";
    Internal::match_types(a, b);
    return Internal::Max::make(a, b);
}

/** Returns an expression representing the lesser of the two
 * arguments, after doing any necessary type coercion using
 * \ref Internal::match_types. Vectorizes cleanly on most platforms
 * (with the exception of integer types on x86 without SSE4). */
inline Expr min(Expr a, Expr b) {
    user_assert(a.defined() && b.defined())
        << "min of undefined Expr\n";
    Internal::match_types(a, b);
    return Internal::Min::make(a, b);
}

/** Clamps an expression to lie within the given bounds. The bounds
 * are type-cast to match the expression. Vectorizes as well as min/max. */
inline Expr clamp(Expr a, Expr min_val, Expr max_val) {
    user_assert(a.defined() && min_val.defined() && max_val.defined())
        << "clamp of undefined Expr\n";
    min_val = cast(a.type(), min_val);
    max_val = cast(a.type(), max_val);
    return Internal::Max::make(Internal::Min::make(a, max_val), min_val);
}

/** Returns the absolute value of a signed integer or floating-point
 * expression. Vectorizes cleanly. Unlike in C, abs of a signed
 * integer returns an unsigned integer of the same bit width. This
 * means that abs of the most negative integer doesn't overflow. */
inline Expr abs(Expr a) {
    user_assert(a.defined())
        << "abs of undefined Expr\n";
    Type t = a.type();
    if (t.is_int()) {
        t.code = Type::UInt;
    } else if (t.is_uint()) {
        user_warning << "Warning: abs of an unsigned type is a no-op\n";
        return a;
    }
    return Internal::Call::make(t, Internal::Call::abs,
                                vec(a), Internal::Call::Intrinsic);
}

/** Returns an expression similar to the ternary operator in C, except
 * that it always evaluates all arguments. If the first argument is
 * true, then return the second, else return the third. Typically
 * vectorizes cleanly, but benefits from SSE41 or newer on x86. */
inline Expr select(Expr condition, Expr true_value, Expr false_value) {

    if (as_const_int(condition)) {
        // Why are you doing this? We'll preserve the select node until constant folding for you.
        condition = cast(Bool(), condition);
    }

    // Coerce int literals to the type of the other argument
    if (as_const_int(true_value)) {
        true_value = cast(false_value.type(), true_value);
    }
    if (as_const_int(false_value)) {
        false_value = cast(true_value.type(), false_value);
    }

    return Internal::Select::make(condition, true_value, false_value);
}

/** A multi-way variant of select similar to a switch statement in C,
 * which can accept multiple conditions and values in pairs. Evaluates
 * to the first value for which the condition is true. Returns the
 * final value if all conditions are false. */
// @{
inline Expr select(Expr c1, Expr v1,
                   Expr c2, Expr v2,
                   Expr default_val) {
    return select(c1, v1,
                  select(c2, v2, default_val));
}
inline Expr select(Expr c1, Expr v1,
                   Expr c2, Expr v2,
                   Expr c3, Expr v3,
                   Expr default_val) {
    return select(c1, v1,
                  c2, v2,
                  select(c3, v3, default_val));
}
inline Expr select(Expr c1, Expr v1,
                   Expr c2, Expr v2,
                   Expr c3, Expr v3,
                   Expr c4, Expr v4,
                   Expr default_val) {
    return select(c1, v1,
                  c2, v2,
                  c3, v3,
                  select(c4, v4, default_val));
}
inline Expr select(Expr c1, Expr v1,
                   Expr c2, Expr v2,
                   Expr c3, Expr v3,
                   Expr c4, Expr v4,
                   Expr c5, Expr v5,
                   Expr default_val) {
    return select(c1, v1,
                  c2, v2,
                  c3, v3,
                  c4, v4,
                  select(c5, v5, default_val));
}
inline Expr select(Expr c1, Expr v1,
                   Expr c2, Expr v2,
                   Expr c3, Expr v3,
                   Expr c4, Expr v4,
                   Expr c5, Expr v5,
                   Expr c6, Expr v6,
                   Expr default_val) {
    return select(c1, v1,
                  c2, v2,
                  c3, v3,
                  c4, v4,
                  c5, v5,
                  select(c6, v6, default_val));
}
inline Expr select(Expr c1, Expr v1,
                   Expr c2, Expr v2,
                   Expr c3, Expr v3,
                   Expr c4, Expr v4,
                   Expr c5, Expr v5,
                   Expr c6, Expr v6,
                   Expr c7, Expr v7,
                   Expr default_val) {
    return select(c1, v1,
                  c2, v2,
                  c3, v3,
                  c4, v4,
                  c5, v5,
                  c6, v6,
                  select(c7, v7, default_val));
}
inline Expr select(Expr c1, Expr v1,
                   Expr c2, Expr v2,
                   Expr c3, Expr v3,
                   Expr c4, Expr v4,
                   Expr c5, Expr v5,
                   Expr c6, Expr v6,
                   Expr c7, Expr v7,
                   Expr c8, Expr v8,
                   Expr default_val) {
    return select(c1, v1,
                  c2, v2,
                  c3, v3,
                  c4, v4,
                  c5, v5,
                  c6, v6,
                  c7, v7,
                  select(c8, v8, default_val));
}
inline Expr select(Expr c1, Expr v1,
                   Expr c2, Expr v2,
                   Expr c3, Expr v3,
                   Expr c4, Expr v4,
                   Expr c5, Expr v5,
                   Expr c6, Expr v6,
                   Expr c7, Expr v7,
                   Expr c8, Expr v8,
                   Expr c9, Expr v9,
                   Expr default_val) {
    return select(c1, v1,
                  c2, v2,
                  c3, v3,
                  c4, v4,
                  c5, v5,
                  c6, v6,
                  c7, v7,
                  c8, v8,
                  select(c9, v9, default_val));
}
inline Expr select(Expr c1, Expr v1,
                   Expr c2, Expr v2,
                   Expr c3, Expr v3,
                   Expr c4, Expr v4,
                   Expr c5, Expr v5,
                   Expr c6, Expr v6,
                   Expr c7, Expr v7,
                   Expr c8, Expr v8,
                   Expr c9, Expr v9,
                   Expr c10, Expr v10,
                   Expr default_val) {
    return select(c1, v1,
                  c2, v2,
                  c3, v3,
                  c4, v4,
                  c5, v5,
                  c6, v6,
                  c7, v7,
                  c8, v8,
                  c9, v9,
                  select(c10, v10, default_val));
}
// @}

/** Return the sine of a floating-point expression. If the argument is
 * not floating-point, it is cast to Float(32). Does not vectorize
 * well. */
inline Expr sin(Expr x) {
    user_assert(x.defined()) << "sin of undefined Expr\n";
    if (x.type() == Float(64)) {
        return Internal::Call::make(Float(64), "sin_f64", vec(x), Internal::Call::Extern);
    } else {
        return Internal::Call::make(Float(32), "sin_f32", vec(cast<float>(x)), Internal::Call::Extern);
    }
}

/** Return the arcsine of a floating-point expression. If the argument
 * is not floating-point, it is cast to Float(32). Does not vectorize
 * well. */
inline Expr asin(Expr x) {
    user_assert(x.defined()) << "asin of undefined Expr\n";
    if (x.type() == Float(64)) {
        return Internal::Call::make(Float(64), "asin_f64", vec(x), Internal::Call::Extern);
    } else {
        return Internal::Call::make(Float(32), "asin_f32", vec(cast<float>(x)), Internal::Call::Extern);
    }
}

/** Return the cosine of a floating-point expression. If the argument
 * is not floating-point, it is cast to Float(32). Does not vectorize
 * well. */
inline Expr cos(Expr x) {
    user_assert(x.defined()) << "cos of undefined Expr\n";
    if (x.type() == Float(64)) {
        return Internal::Call::make(Float(64), "cos_f64", vec(x), Internal::Call::Extern);
    } else {
        return Internal::Call::make(Float(32), "cos_f32", vec(cast<float>(x)), Internal::Call::Extern);
    }
}

/** Return the arccosine of a floating-point expression. If the
 * argument is not floating-point, it is cast to Float(32). Does not
 * vectorize well. */
inline Expr acos(Expr x) {
    user_assert(x.defined()) << "acos of undefined Expr\n";
    if (x.type() == Float(64)) {
        return Internal::Call::make(Float(64), "acos_f64", vec(x), Internal::Call::Extern);
    } else {
        return Internal::Call::make(Float(32), "acos_f32", vec(cast<float>(x)), Internal::Call::Extern);
    }
}

/** Return the tangent of a floating-point expression. If the argument
 * is not floating-point, it is cast to Float(32). Does not vectorize
 * well. */
inline Expr tan(Expr x) {
    user_assert(x.defined()) << "tan of undefined Expr\n";
    if (x.type() == Float(64)) {
        return Internal::Call::make(Float(64), "tan_f64", vec(x), Internal::Call::Extern);
    } else {
        return Internal::Call::make(Float(32), "tan_f32", vec(cast<float>(x)), Internal::Call::Extern);
    }
}

/** Return the arctangent of a floating-point expression. If the
 * argument is not floating-point, it is cast to Float(32). Does not
 * vectorize well. */
inline Expr atan(Expr x) {
    user_assert(x.defined()) << "atan of undefined Expr\n";
    if (x.type() == Float(64)) {
        return Internal::Call::make(Float(64), "atan_f64", vec(x), Internal::Call::Extern);
    } else {
        return Internal::Call::make(Float(32), "atan_f32", vec(cast<float>(x)), Internal::Call::Extern);
    }
}

/** Return the angle of a floating-point gradient. If the argument is
 * not floating-point, it is cast to Float(32). Does not vectorize
 * well. */
inline Expr atan2(Expr y, Expr x) {
    user_assert(x.defined() && y.defined()) << "atan2 of undefined Expr\n";

    if (y.type() == Float(64)) {
        x = cast<double>(x);
        return Internal::Call::make(Float(64), "atan2_f64", vec(y, x), Internal::Call::Extern);
    } else {
        y = cast<float>(y);
        x = cast<float>(x);
        return Internal::Call::make(Float(32), "atan2_f32", vec(y, x), Internal::Call::Extern);
    }
}

/** Return the hyperbolic sine of a floating-point expression.  If the
 *  argument is not floating-point, it is cast to Float(32). Does not
 *  vectorize well. */
inline Expr sinh(Expr x) {
    user_assert(x.defined()) << "sinh of undefined Expr\n";
    if (x.type() == Float(64)) {
        return Internal::Call::make(Float(64), "sinh_f64", vec(x), Internal::Call::Extern);
    } else {
        return Internal::Call::make(Float(32), "sinh_f32", vec(cast<float>(x)), Internal::Call::Extern);
    }
}

/** Return the hyperbolic arcsinhe of a floating-point expression.  If
 * the argument is not floating-point, it is cast to Float(32). Does
 * not vectorize well. */
inline Expr asinh(Expr x) {
    user_assert(x.defined()) << "asinh of undefined Expr\n";
    if (x.type() == Float(64)) {
        return Internal::Call::make(Float(64), "asinh_f64", vec(x), Internal::Call::Extern);
    } else {
        return Internal::Call::make(Float(32), "asinh_f32", vec(cast<float>(x)), Internal::Call::Extern);
    }
}

/** Return the hyperbolic cosine of a floating-point expression.  If
 * the argument is not floating-point, it is cast to Float(32). Does
 * not vectorize well. */
inline Expr cosh(Expr x) {
    user_assert(x.defined()) << "cosh of undefined Expr\n";
    if (x.type() == Float(64)) {
        return Internal::Call::make(Float(64), "cosh_f64", vec(x), Internal::Call::Extern);
    } else {
        return Internal::Call::make(Float(32), "cosh_f32", vec(cast<float>(x)), Internal::Call::Extern);
    }
}

/** Return the hyperbolic arccosine of a floating-point expression.
 * If the argument is not floating-point, it is cast to
 * Float(32). Does not vectorize well. */
inline Expr acosh(Expr x) {
    user_assert(x.defined()) << "acosh of undefined Expr\n";
    if (x.type() == Float(64)) {
        return Internal::Call::make(Float(64), "acosh_f64", vec(x), Internal::Call::Extern);
    } else {
        return Internal::Call::make(Float(32), "acosh_f32", vec(cast<float>(x)), Internal::Call::Extern);
    }
}

/** Return the hyperbolic tangent of a floating-point expression.  If
 * the argument is not floating-point, it is cast to Float(32). Does
 * not vectorize well. */
inline Expr tanh(Expr x) {
    user_assert(x.defined()) << "tanh of undefined Expr\n";
    if (x.type() == Float(64)) {
        return Internal::Call::make(Float(64), "tanh_f64", vec(x), Internal::Call::Extern);
    } else {
        return Internal::Call::make(Float(32), "tanh_f32", vec(cast<float>(x)), Internal::Call::Extern);
    }
}

/** Return the hyperbolic arctangent of a floating-point expression.
 * If the argument is not floating-point, it is cast to
 * Float(32). Does not vectorize well. */
inline Expr atanh(Expr x) {
    user_assert(x.defined()) << "atanh of undefined Expr\n";
    if (x.type() == Float(64)) {
        return Internal::Call::make(Float(64), "atanh_f64", vec(x), Internal::Call::Extern);
    } else {
        return Internal::Call::make(Float(32), "atanh_f32", vec(cast<float>(x)), Internal::Call::Extern);
    }
}

/** Return the square root of a floating-point expression. If the
 * argument is not floating-point, it is cast to Float(32). Typically
 * vectorizes cleanly. */
inline Expr sqrt(Expr x) {
    user_assert(x.defined()) << "sqrt of undefined Expr\n";
    if (x.type() == Float(64)) {
        return Internal::Call::make(Float(64), "sqrt_f64", vec(x), Internal::Call::Extern);
    } else {
        return Internal::Call::make(Float(32), "sqrt_f32", vec(cast<float>(x)), Internal::Call::Extern);
    }
}

/** Return the square root of the sum of the squares of two
 * floating-point expressions. If the argument is not floating-point,
 * it is cast to Float(32). Vectorizes cleanly. */
inline Expr hypot(Expr x, Expr y) {
    return sqrt(x*x + y*y);
}

/** Return the exponential of a floating-point expression. If the
 * argument is not floating-point, it is cast to Float(32). For
 * Float(64) arguments, this calls the system exp function, and does
 * not vectorize well. For Float(32) arguments, this function is
 * vectorizable, does the right thing for extremely small or extremely
 * large inputs, and is accurate up to the last bit of the
 * mantissa. Vectorizes cleanly. */
inline Expr exp(Expr x) {
    user_assert(x.defined()) << "exp of undefined Expr\n";
    if (x.type() == Float(64)) {
        return Internal::Call::make(Float(64), "exp_f64", vec(x), Internal::Call::Extern);
    } else {
        return Internal::Call::make(Float(32), "exp_f32", vec(cast<float>(x)), Internal::Call::Extern);
    }
}

/** Return the logarithm of a floating-point expression. If the
 * argument is not floating-point, it is cast to Float(32). For
 * Float(64) arguments, this calls the system log function, and does
 * not vectorize well. For Float(32) arguments, this function is
 * vectorizable, does the right thing for inputs <= 0 (returns -inf or
 * nan), and is accurate up to the last bit of the
 * mantissa. Vectorizes cleanly. */
inline Expr log(Expr x) {
    user_assert(x.defined()) << "log of undefined Expr\n";
    if (x.type() == Float(64)) {
        return Internal::Call::make(Float(64), "log_f64", vec(x), Internal::Call::Extern);
    } else {
        return Internal::Call::make(Float(32), "log_f32", vec(cast<float>(x)), Internal::Call::Extern);
    }
}

/** Return one floating point expression raised to the power of
 * another. The type of the result is given by the type of the first
 * argument. If the first argument is not a floating-point type, it is
 * cast to Float(32). For Float(32), cleanly vectorizable, and
 * accurate up to the last few bits of the mantissa. Gets worse when
 * approaching overflow. Vectorizes cleanly. */
inline Expr pow(Expr x, Expr y) {
    user_assert(x.defined() && y.defined()) << "pow of undefined Expr\n";

    if (const int *i = as_const_int(y)) {
        return raise_to_integer_power(x, *i);
    }

    if (x.type() == Float(64)) {
        y = cast<double>(y);
        return Internal::Call::make(Float(64), "pow_f64", vec(x, y), Internal::Call::Extern);
    } else {
        x = cast<float>(x);
        y = cast<float>(y);
        return Internal::Call::make(Float(32), "pow_f32", vec(x, y), Internal::Call::Extern);
    }
}

/** Evaluate the error function erf. Only available for
 * Float(32). Accurate up to the last three bits of the
 * mantissa. Vectorizes cleanly. */
inline Expr erf(Expr x) {
    user_assert(x.defined()) << "erf of undefined Expr\n";
    user_assert(x.type() == Float(32)) << "erf only takes float arguments\n";
    return Internal::halide_erf(x);
}

/** Fast approximate cleanly vectorizable log for Float(32). Returns
 * nonsense for x <= 0.0f. Accurate up to the last 5 bits of the
 * mantissa. Vectorizes cleanly. */
EXPORT Expr fast_log(Expr x);

/** Fast approximate cleanly vectorizable exp for Float(32). Returns
 * nonsense for inputs that would overflow or underflow. Typically
 * accurate up to the last 5 bits of the mantissa. Gets worse when
 * approaching overflow. Vectorizes cleanly. */
EXPORT Expr fast_exp(Expr x);

/** Fast approximate cleanly vectorizable pow for Float(32). Returns
 * nonsense for x < 0.0f. Accurate up to the last 5 bits of the
 * mantissa for typical exponents. Gets worse when approaching
 * overflow. Vectorizes cleanly. */
inline Expr fast_pow(Expr x, Expr y) {
    if (const int *i = as_const_int(y)) {
        return raise_to_integer_power(x, *i);
    }

    x = cast<float>(x);
    y = cast<float>(y);
    return select(x == 0.0f, 0.0f, fast_exp(fast_log(x) * y));
}

/** Return the greatest whole number less than or equal to a
 * floating-point expression. If the argument is not floating-point,
 * it is cast to Float(32). The return value is still in floating
 * point, despite being a whole number. Vectorizes cleanly. */
inline Expr floor(Expr x) {
    user_assert(x.defined()) << "floor of undefined Expr\n";
    if (x.type().element_of() == Float(64)) {
        return Internal::Call::make(x.type(), "floor_f64", vec(x), Internal::Call::Extern);
    } else {
        Type t = Float(32, x.type().width);
        return Internal::Call::make(t, "floor_f32", vec(cast(t, x)), Internal::Call::Extern);
    }
}

/** Return the least whole number greater than or equal to a
 * floating-point expression. If the argument is not floating-point,
 * it is cast to Float(32). The return value is still in floating
 * point, despite being a whole number. Vectorizes cleanly. */
inline Expr ceil(Expr x) {
    user_assert(x.defined()) << "ceil of undefined Expr\n";
    if (x.type().element_of() == Float(64)) {
        return Internal::Call::make(x.type(), "ceil_f64", vec(x), Internal::Call::Extern);
    } else {
        Type t = Float(32, x.type().width);
        return Internal::Call::make(t, "ceil_f32", vec(cast(t, x)), Internal::Call::Extern);
    }
}

/** Return the whole number closest to a floating-point expression. If the
 * argument is not floating-point, it is cast to Float(32). The return value
 * is still in floating point, despite being a whole number. On ties, we
 * follow IEEE754 conventions and round to the nearest even number. Vectorizes
 * cleanly. */
inline Expr round(Expr x) {
    user_assert(x.defined()) << "round of undefined Expr\n";
    if (x.type().element_of() == Float(64)) {
        return Internal::Call::make(Float(64), "round_f64", vec(x), Internal::Call::Extern);
    } else {
        Type t = Float(32, x.type().width);
        return Internal::Call::make(t, "round_f32", vec(cast(t, x)), Internal::Call::Extern);
    }
}

/** Return the integer part of a floating-point expression. If the argument is
 * not floating-point, it is cast to Float(32). The return value is still in
 * floating point, despite being a whole number. Vectorizes cleanly. */
inline Expr trunc(Expr x) {
    user_assert(x.defined()) << "trunc of undefined Expr\n";
    if (x.type().element_of() == Float(64)) {
        return Internal::Call::make(Float(64), "trunc_f64", vec(x), Internal::Call::Extern);
    } else {
        Type t = Float(32, x.type().width);
        return Internal::Call::make(t, "trunc_f32", vec(cast(t, x)), Internal::Call::Extern);
    }
}

/** Return the fractional part of a floating-point expression. If the argument
 *  is not floating-point, it is cast to Float(32). The return value has the
 *  same sign as the original expression. Vectorizes cleanly. */
inline Expr fract(Expr x) {
    user_assert(x.defined()) << "fract of undefined Expr\n";
    return x - trunc(x);
}

/** Reinterpret the bits of one value as another type. */
inline Expr reinterpret(Type t, Expr e) {
    user_assert(e.defined()) << "reinterpret of undefined Expr\n";
    int from_bits = e.type().bits * e.type().width;
    int to_bits = t.bits * t.width;
    user_assert(from_bits == to_bits)
        << "Reinterpret cast from type " << e.type()
        << " which has " << from_bits
        << " bits, to type " << t
        << " which has " << to_bits << " bits\n";
    return Internal::Call::make(t, Internal::Call::reinterpret, vec(e), Internal::Call::Intrinsic);
}

template<typename T>
inline Expr reinterpret(Expr e) {
    return reinterpret(type_of<T>(), e);
}

/** Return the bitwise and of two expressions (which need not have the
 * same type). The type of the result is the type of the first
 * argument. */
inline Expr operator&(Expr x, Expr y) {
    user_assert(x.defined() && y.defined()) << "bitwise and of undefined Expr\n";
    // First widen or narrow, then bitcast.
    if (y.type().bits != x.type().bits) {
        Type t = y.type();
        t.bits = x.type().bits;
        y = cast(t, y);
    }
    if (y.type() != x.type()) {
        y = reinterpret(x.type(), y);
    }
    return Internal::Call::make(x.type(), Internal::Call::bitwise_and, vec(x, y), Internal::Call::Intrinsic);
}

/** Return the bitwise or of two expressions (which need not have the
 * same type). The type of the result is the type of the first
 * argument. */
inline Expr operator|(Expr x, Expr y) {
    user_assert(x.defined() && y.defined()) << "bitwise or of undefined Expr\n";
    // First widen or narrow, then bitcast.
    if (y.type().bits != x.type().bits) {
        Type t = y.type();
        t.bits = x.type().bits;
        y = cast(t, y);
    }
    if (y.type() != x.type()) {
        y = reinterpret(x.type(), y);
    }
    return Internal::Call::make(x.type(), Internal::Call::bitwise_or, vec(x, y), Internal::Call::Intrinsic);
}

/** Return the bitwise exclusive or of two expressions (which need not
 * have the same type). The type of the result is the type of the
 * first argument. */
inline Expr operator^(Expr x, Expr y) {
    user_assert(x.defined() && y.defined()) << "bitwise or of undefined Expr\n";
    // First widen or narrow, then bitcast.
    if (y.type().bits != x.type().bits) {
        Type t = y.type();
        t.bits = x.type().bits;
        y = cast(t, y);
    }
    if (y.type() != x.type()) {
        y = reinterpret(x.type(), y);
    }
    return Internal::Call::make(x.type(), Internal::Call::bitwise_xor, vec(x, y), Internal::Call::Intrinsic);
}

/** Return the bitwise not of an expression. */
inline Expr operator~(Expr x) {
    user_assert(x.defined()) << "bitwise or of undefined Expr\n";
    return Internal::Call::make(x.type(), Internal::Call::bitwise_not, vec(x), Internal::Call::Intrinsic);
}

/** Shift the bits of an integer value left. This is actually less
 * efficient than multiplying by 2^n, because Halide's optimization
 * passes understand multiplication, and will compile it to
 * shifting. This operator is only for if you really really need bit
 * shifting (e.g. because the exponent is a run-time parameter). The
 * type of the result is equal to the type of the first argument. Both
 * arguments must have integer type. */
inline Expr operator<<(Expr x, Expr y) {
    user_assert(x.defined() && y.defined()) << "shift left of undefined Expr\n";
    user_assert(!x.type().is_float()) << "First argument to shift left is a float: " << x << "\n";
    user_assert(!y.type().is_float()) << "Second argument to shift left is a float: " << y << "\n";
    Internal::match_types(x, y);
    return Internal::Call::make(x.type(), Internal::Call::shift_left, vec(x, y), Internal::Call::Intrinsic);
}

/** Shift the bits of an integer value right. Does sign extension for
 * signed integers. This is less efficient than dividing by a power of
 * two. Halide's definition of division (always round to negative
 * infinity) means that all divisions by powers of two get compiled to
 * bit-shifting, and Halide's optimization routines understand
 * division and can work with it. The type of the result is equal to
 * the type of the first argument. Both arguments must have integer
 * type. */
inline Expr operator>>(Expr x, Expr y) {
    user_assert(x.defined() && y.defined()) << "shift right of undefined Expr\n";
    user_assert(!x.type().is_float()) << "First argument to shift right is a float: " << x << "\n";
    user_assert(!y.type().is_float()) << "Second argument to shift right is a float: " << y << "\n";
    Internal::match_types(x, y);
    return Internal::Call::make(x.type(), Internal::Call::shift_right, vec(x, y), Internal::Call::Intrinsic);
}

/** Linear interpolate between the two values according to a weight.
 * \param zero_val The result when weight is 0
 * \param one_val The result when weight is 1
 * \param weight The interpolation amount
 *
 * Both zero_val and one_val must have the same type. All types are
 * supported, including bool.
 *
 * The weight is treated as its own type and must be float or an
 * unsigned integer type. It is scaled to the bit-size of the type of
 * x and y if they are integer, or converted to float if they are
 * float. Integer weights are converted to float via division by the
 * full-range value of the weight's type. Floating-point weights used
 * to interpolate between integer values must be between 0.0f and
 * 1.0f, and an error may be signaled if it is not provably so. (clamp
 * operators can be added to provide proof. Currently an error is only
 * signalled for constant weights.)
 *
 * For integer linear interpolation, out of range values cannot be
 * represented. In particular, weights that are conceptually less than
 * 0 or greater than 1.0 are not representable. As such the result is
 * always between x and y (inclusive of course). For lerp with
 * floating-point values and floating-point weight, the full range of
 * a float is valid, however underflow and overflow can still occur.
 *
 * Ordering is not required between zero_val and one_val:
 *     lerp(42, 69, .5f) == lerp(69, 42, .5f) == 56
 *
 * Results for integer types are for exactly rounded arithmetic. As
 * such, there are cases where 16-bit and float differ because 32-bit
 * floating-point (float) does not have enough precision to produce
 * the exact result. (Likely true for 32-bit integer
 * vs. double-precision floating-point as well.)
 *
 * At present, double precision and 64-bit integers are not supported.
 *
 * Generally, lerp will vectorize as if it were an operation on a type
 * twice the bit size of the inferred type for x and y.
 *
 * Some examples:
 * \code
 *
 *     // Since Halide does not have direct type delcarations, casts
 *     // below are used to indicate the types of the parameters.
 *     // Such casts not required or expected in actual code where types
 *     // are inferred.
 *
 *     lerp(cast<float>(x), cast<float>(y), cast<float>(w)) ->
 *       x * (1.0f - w) + y * w
 *
 *     lerp(cast<uint8_t>(x), cast<uint8_t>(y), cast<uint8_t>(w)) ->
 *       cast<uint8_t>(cast<uint8_t>(x) * (1.0f - cast<uint8_t>(w) / 255.0f) +
 *                     cast<uint8_t>(y) * cast<uint8_t>(w) / 255.0f + .5f)
 *
 *     // Note addition in Halide promoted uint8_t + int8_t to int16_t already,
 *     // the outer cast is added for clarity.
 *     lerp(cast<uint8_t>(x), cast<int8_t>(y), cast<uint8_t>(w)) ->
 *       cast<int16_t>(cast<uint8_t>(x) * (1.0f - cast<uint8_t>(w) / 255.0f) +
 *                     cast<int8_t>(y) * cast<uint8_t>(w) / 255.0f + .5f)
 *
 *     lerp(cast<int8_t>(x), cast<int8_t>(y), cast<float>(w)) ->
 *       cast<int8_t>(cast<int8_t>(x) * (1.0f - cast<float>(w)) +
 *                    cast<int8_t>(y) * cast<uint8_t>(w))
 *
 * \endcode
 * */
inline Expr lerp(Expr zero_val, Expr one_val, Expr weight) {
    user_assert(zero_val.defined()) << "lerp with undefined zero value";
    user_assert(one_val.defined()) << "lerp with undefined one value";
    user_assert(weight.defined()) << "lerp with undefined weight";

    // We allow integer constants through, so that you can say things
    // like lerp(0, cast<uint8_t>(x), alpha) and produce an 8-bit
    // result. Note that lerp(0.0f, cast<uint8_t>(x), alpha) will
    // produce an error, as will lerp(0.0f, cast<double>(x),
    // alpha). lerp(0, cast<float>(x), alpha) is also allowed and will
    // produce a float result.
    if (as_const_int(zero_val)) {
        zero_val = cast(one_val.type(), zero_val);
    }
    if (as_const_int(one_val)) {
        one_val = cast(zero_val.type(), one_val);
    }

    user_assert(zero_val.type() == one_val.type())
        << "Can't lerp between " << zero_val << " of type " << zero_val.type()
        << " and " << one_val << " of different type " << one_val.type() << "\n";
    user_assert((weight.type().is_uint() || weight.type().is_float()))
        << "A lerp weight must be an unsigned integer or a float, but "
        << "lerp weight " << weight << " has type " << weight.type() << ".\n";
    user_assert((zero_val.type().is_float() || zero_val.type().width <= 32))
        << "Lerping between 64-bit integers is not supported\n";
    // Compilation error for constant weight that is out of range for integer use
    // as this seems like an easy to catch gotcha.
    if (!zero_val.type().is_float()) {
        const float *const_weight = as_const_float(weight);
        if (const_weight) {
            user_assert(*const_weight >= 0.0f && *const_weight <= 1.0f)
                << "Floating-point weight for lerp with integer arguments is "
                << *const_weight << ", which is not in the range [0.0f, 1.0f].\n";
        }
    }
    return Internal::Call::make(zero_val.type(), Internal::Call::lerp,
                                vec(zero_val, one_val, weight),
                                Internal::Call::Intrinsic);
}

/** Count the number of set bits in an expression. */
inline Expr popcount(Expr x) {
    user_assert(x.defined()) << "popcount of undefined Expr\n";
    return Internal::Call::make(x.type(), Internal::Call::popcount,
                                vec(x), Internal::Call::Intrinsic);
}

/** Count the number of leading zero bits in an expression. The result is
 *  undefined if the value of the expression is zero. */
inline Expr count_leading_zeros(Expr x) {
    user_assert(x.defined()) << "count leading zeros of undefined Expr\n";
    return Internal::Call::make(x.type(), Internal::Call::count_leading_zeros,
                                vec(x), Internal::Call::Intrinsic);
}

/** Count the number of trailing zero bits in an expression. The result is
 *  undefined if the value of the expression is zero. */
inline Expr count_trailing_zeros(Expr x) {
    user_assert(x.defined()) << "count trailing zeros of undefined Expr\n";
    return Internal::Call::make(x.type(), Internal::Call::count_trailing_zeros,
                                vec(x), Internal::Call::Intrinsic);
}

/** Return a random variable representing a uniformly distributed
 * float in the half-open interval [0.0f, 1.0f). For random numbers of
 * other types, use lerp with a random float as the last parameter.
 *
 * Optionally takes a seed.
 *
 * Note that:
 \code
 Expr x = random_float();
 Expr y = x + x;
 \endcode
 *
 * is very different to
 *
 \code
 Expr y = random_float() + random_float();
 \endcode
 *
 * The first doubles a random variable, and the second adds two
 * independent random variables.
 *
 * A given random variable takes on a unique value that depends
 * deterministically on the pure variables of the function they belong
 * to, the identity of the function itself, and which definition of
 * the function it is used in. They are, however, shared across tuple
 * elements.
 *
 * This function vectorizes cleanly.
 */
inline Expr random_float(Expr seed = Expr()) {
    // Random floats get even IDs
    static int counter = -2;
    counter += 2;

    std::vector<Expr> args;
    if (seed.defined()) {
        user_assert(seed.type() == Int(32))
            << "The seed passed to random_float must have type Int(32), but instead is "
            << seed << " of type " << seed.type() << "\n";
        args.push_back(seed);
    }
    args.push_back(counter);

    return Internal::Call::make(Float(32), Internal::Call::random,
                                args, Internal::Call::Intrinsic);
}

/** Return a random variable representing a uniformly distributed
 * 32-bit integer. See \ref random_float. Vectorizes cleanly. */
inline Expr random_int(Expr seed = Expr()) {
    // Random ints get odd IDs
    static int counter = -1;
    counter += 2;

    std::vector<Expr> args;
    if (seed.defined()) {
        user_assert(seed.type() == Int(32))
            << "The seed passed to random_int must have type Int(32), but instead is "
            << seed << " of type " << seed.type() << "\n";
        args.push_back(seed);
    }
    args.push_back(counter);

    return Internal::Call::make(Int(32), Internal::Call::random,
                                args, Internal::Call::Intrinsic);
}

// For the purposes of a call to print, const char * can convert
// silently to an Expr
struct PrintArg {
    Expr expr;
    PrintArg(const char *str) : expr(std::string(str)) {}
    template<typename T> PrintArg(T e) : expr(e) {}
    operator Expr() {return expr;}
};

/** Create an Expr that prints out its value whenever it is
 * evaluated. It also prints out everything else in the arguments
 * list, separated by spaces. This can include string literals. */
// @{
EXPORT Expr print(const std::vector<Expr> &values);
inline Expr print(PrintArg a) {
    return print(Internal::vec<Expr>(a));
}
inline Expr print(PrintArg a, PrintArg b) {
    return print(Internal::vec<Expr>(a, b));
}
inline Expr print(PrintArg a, PrintArg b, PrintArg c) {
    return print(Internal::vec<Expr>(a, b, c));
}
inline Expr print(PrintArg a, PrintArg b, PrintArg c, PrintArg d) {
    return print(Internal::vec<Expr>(a, b, c, d));
}
inline Expr print(PrintArg a, PrintArg b, PrintArg c, PrintArg d, PrintArg e) {
    return print(Internal::vec<Expr>(a, b, c, d, e));
}
inline Expr print(PrintArg a, PrintArg b, PrintArg c, PrintArg d, PrintArg e, PrintArg f) {
    return print(Internal::vec<Expr>(a, b, c, d, e, f));
}
inline Expr print(PrintArg a, PrintArg b, PrintArg c, PrintArg d, PrintArg e, PrintArg f, PrintArg g) {
    return print(Internal::vec<Expr>(a, b, c, d, e, f, g));
}
inline Expr print(PrintArg a, PrintArg b, PrintArg c, PrintArg d, PrintArg e, PrintArg f, PrintArg g, PrintArg h) {
    return print(Internal::vec<Expr>(a, b, c, d, e, f, g, h));
}
// @}

/** Create an Expr that prints whenever it is evaluated, provided that
 * the condition is true. */
// @{
EXPORT Expr print_when(Expr condition, const std::vector<Expr> &values);
inline Expr print_when(Expr condition, PrintArg a) {
    return print_when(condition, Internal::vec<Expr>(a));
}
inline Expr print_when(Expr condition, PrintArg a, PrintArg b) {
    return print_when(condition, Internal::vec<Expr>(a, b));
}
inline Expr print_when(Expr condition, PrintArg a, PrintArg b, PrintArg c) {
    return print_when(condition, Internal::vec<Expr>(a, b, c));
}
inline Expr print_when(Expr condition, PrintArg a, PrintArg b, PrintArg c, PrintArg d) {
    return print_when(condition, Internal::vec<Expr>(a, b, c, d));
}
inline Expr print_when(Expr condition, PrintArg a, PrintArg b, PrintArg c, PrintArg d, PrintArg e) {
    return print_when(condition, Internal::vec<Expr>(a, b, c, d, e));
}
inline Expr print_when(Expr condition, PrintArg a, PrintArg b, PrintArg c, PrintArg d, PrintArg e, PrintArg f) {
    return print_when(condition, Internal::vec<Expr>(a, b, c, d, e, f));
}
inline Expr print_when(Expr condition, PrintArg a, PrintArg b, PrintArg c, PrintArg d, PrintArg e, PrintArg f, PrintArg g) {
    return print_when(condition, Internal::vec<Expr>(a, b, c, d, e, f, g));
}
inline Expr print_when(Expr condition, PrintArg a, PrintArg b, PrintArg c, PrintArg d, PrintArg e, PrintArg f, PrintArg g, PrintArg h) {
    return print_when(condition, Internal::vec<Expr>(a, b, c, d, e, f, g, h));
}
// @}


/** Return an undef value of the given type. Halide skips stores that
 * depend on undef values, so you can use this to mean "do not modify
 * this memory location". This is an escape hatch that can be used for
 * several things:
 *
 * You can define a reduction with no pure step, by setting the pure
 * step to undef. Do this only if you're confident that the update
 * steps are sufficient to correctly fill in the domain.
 *
 * For a tuple-valued reduction, you can write an update step that
 * only updates some tuple elements.
 *
 * You can define single-stage pipeline that only has update steps,
 * and depends on the values already in the output buffer.
 *
 * Use this feature with great caution, as you can use it to load from
 * uninitialized memory.
 */
inline Expr undef(Type t) {
    return Internal::Call::make(t, Internal::Call::undef,
                                std::vector<Expr>(),
                                Internal::Call::Intrinsic);
}

template<typename T>
inline Expr undef() {
    return undef(type_of<T>());
}

/** Control the values used in the memoization cache key for memoize.
 * Normally parameters and other external dependencies are
 * automatically inferred and added to the cache key. The memoize_tag
 * operator allows computing one expression and using either the
 * computed value, or one or more other expressions in the cache key
 * instead of the parameter dependencies of the computation. The
 * single argument version is completely safe in that the cache key
 * will use the actual computed value -- it is difficult or imposible
 * to produce erroneous caching this way. The more-than-one argument
 * version allows generating cache keys that do not uniquely identify
 * the computation and thus can result in caching errors.
 *
 * A potential use for the single argument version is to handle a
 * floating-point parameter that is quantized to a small
 * integer. Mutliple values of the float will produce the same integer
 * and moving the caching to using the integer for the key is more
 * efficient.
 *
 * The main use for the more-than-one argument version is to provide
 * cache key information for Handles and ImageParams, which otherwise
 * are not allowed inside compute_cached operations. E.g. when passing
 * a group of parameters to an external array function via a Handle,
 * memoize_tag can be used to isolate the actual values used by that
 * computation. If an ImageParam is a constant image with a persistent
 * digest, memoize_tag can be used to key computations using that image
 * on the digest. */
// @{
EXPORT Expr memoize_tag(Expr result, const std::vector<Expr> &cache_key_values);
inline Expr memoize_tag(Expr result) {
    return memoize_tag(result, std::vector<Expr>());
}
inline Expr memoize_tag(Expr result, Expr a) {
    return memoize_tag(result, Internal::vec<Expr>(a));
}
inline Expr memoize_tag(Expr result, Expr a, Expr b) {
    return memoize_tag(result, Internal::vec<Expr>(a, b));
}
inline Expr memoize_tag(Expr result, Expr a, Expr b, Expr c) {
    return memoize_tag(result, Internal::vec<Expr>(a, b, c));
}
inline Expr memoize_tag(Expr result, Expr a, Expr b, Expr c, Expr d) {
    return memoize_tag(result, Internal::vec<Expr>(a, b, c, d));
}
inline Expr memoize_tag(Expr result, Expr a, Expr b, Expr c, Expr d, Expr e) {
    return memoize_tag(result, Internal::vec<Expr>(a, b, c, d, e));
}
inline Expr memoize_tag(Expr result, Expr a, Expr b, Expr c, Expr d, Expr e, Expr f) {
    return memoize_tag(result, Internal::vec<Expr>(a, b, c, d, e, f));
}
inline Expr memoize_tag(Expr result, Expr a, Expr b, Expr c, Expr d, Expr e, Expr f, Expr g) {
    return memoize_tag(result, Internal::vec<Expr>(a, b, c, d, e, f, g));
}
inline Expr memoize_tag(Expr result, Expr a, Expr b, Expr c, Expr d, Expr e, Expr f, Expr g, Expr h) {
    return memoize_tag(result, Internal::vec<Expr>(a, b, c, d, e, f, g, h));
}
// @}

}

#endif
#ifndef HALIDE_SCOPE_H
#define HALIDE_SCOPE_H

#include <string>
#include <map>
#include <stack>
#include <utility>
#include <iostream>


/** \file
 * Defines the Scope class, which is used for keeping track of names in a scope while traversing IR
 */

namespace Halide {
namespace Internal {

/** A stack which can store one item very efficiently. Using this
 * instead of std::stack speeds up Scope substantially. */
template<typename T>
class SmallStack {
private:
    T _top;
    std::vector<T> _rest;
    bool _empty;

public:
    SmallStack() : _empty(true) {}

    void pop() {
        if (_rest.empty()) {
            _empty = true;
            _top = T();
        } else {
            _top = _rest.back();
            _rest.pop_back();
        }
    }

    void push(const T &t) {
        if (_empty) {
            _empty = false;
        } else {
            _rest.push_back(_top);
        }
        _top = t;
    }

    T top() const {
        return _top;
    }

    T &top_ref() {
        return _top;
    }

    bool empty() const {
        return _empty;
    }
};

/** A common pattern when traversing Halide IR is that you need to
 * keep track of stuff when you find a Let or a LetStmt, and that it
 * should hide previous values with the same name until you leave the
 * Let or LetStmt nodes This class helps with that. */
template<typename T>
class Scope {
private:
    std::map<std::string, SmallStack<T> > table;

    // Copying a scope object copies a large table full of strings and
    // stacks. Bad idea.
    Scope(const Scope<T> &);
    Scope<T> &operator=(const Scope<T> &);

    const Scope<T> *containing_scope;


public:
    Scope() : containing_scope(NULL) {}

    /** Set the parent scope. If lookups fail in this scope, they
     * check the containing scope before returning an error. Caller is
     * responsible for managing the memory of the containing scope. */
    void set_containing_scope(const Scope<T> *s) {
        containing_scope = s;
    }

    /** A const ref to an empty scope. Useful for default function
     * arguments, which would otherwise require a copy constructor
     * (with llvm in c++98 mode) */
    static const Scope<T> &empty_scope() {
        static Scope<T> _empty_scope;
        return _empty_scope;
    }

    /** Retrieve the value referred to by a name */
    T get(const std::string &name) const {
        typename std::map<std::string, SmallStack<T> >::const_iterator iter = table.find(name);
        if (iter == table.end() || iter->second.empty()) {
            if (containing_scope) {
                return containing_scope->get(name);
            } else {
                internal_error << "Symbol '" << name << "' not found\n";
            }
        }
        return iter->second.top();
    }

    /** Return a reference to an entry. Does not consider the containing scope. */
    T &ref(const std::string &name) {
        typename std::map<std::string, SmallStack<T> >::iterator iter = table.find(name);
        if (iter == table.end() || iter->second.empty()) {
            internal_error << "Symbol '" << name << "' not found\n";
        }
        return iter->second.top_ref();
    }

    /** Tests if a name is in scope */
    bool contains(const std::string &name) const {
        typename std::map<std::string, SmallStack<T> >::const_iterator iter = table.find(name);
        if (iter == table.end() || iter->second.empty()) {
            if (containing_scope) {
                return containing_scope->contains(name);
            } else {
                return false;
            }
        }
        return true;
    }

    /** Add a new (name, value) pair to the current scope. Hide old
     * values that have this name until we pop this name.
     */
    void push(const std::string &name, const T &value) {
        table[name].push(value);
    }

    /** A name goes out of scope. Restore whatever its old value
     * was (or remove it entirely if there was nothing else of the
     * same name in an outer scope) */
    void pop(const std::string &name) {
        typename std::map<std::string, SmallStack<T> >::iterator iter = table.find(name);
        internal_assert(iter != table.end()) << "Name not in symbol table: " << name << "\n";
        iter->second.pop();
        if (iter->second.empty()) {
            table.erase(iter);
        }
    }

    /** Iterate through the scope. Does not capture any containing scope. */
    class const_iterator {
        typename std::map<std::string, SmallStack<T> >::const_iterator iter;
    public:
        explicit const_iterator(const typename std::map<std::string, SmallStack<T> >::const_iterator &i) :
            iter(i) {
        }

        const_iterator() {}

        bool operator!=(const const_iterator &other) {
            return iter != other.iter;
        }

        void operator++() {
            ++iter;
        }

        const std::string &name() {
            return iter->first;
        }

        const SmallStack<T> &stack() {
            return iter->second;
        }

        const T &value() {
            return iter->second.top();
        }
    };

    const_iterator cbegin() const {
        return const_iterator(table.begin());
    }

    const_iterator cend() const {
        return const_iterator(table.end());
    }

    class iterator {
        typename std::map<std::string, SmallStack<T> >::iterator iter;
    public:
        explicit iterator(typename std::map<std::string, SmallStack<T> >::iterator i) :
            iter(i) {
        }

        iterator() {}

        bool operator!=(const iterator &other) {
            return iter != other.iter;
        }

        void operator++() {
            ++iter;
        }

        const std::string &name() {
            return iter->first;
        }

        SmallStack<T> &stack() {
            return iter->second;
        }

        T &value() {
            return iter->second.top_ref();
        }
    };

    iterator begin() {
        return iterator(table.begin());
    }

    iterator end() {
        return iterator(table.end());
    }

    void swap(Scope<T> &other) {
        table.swap(other.table);
        std::swap(containing_scope, other.containing_scope);
    }
};

template<typename T>
std::ostream &operator<<(std::ostream &stream, const Scope<T>& s) {
    stream << "{\n";
    typename Scope<T>::const_iterator iter;
    for (iter = s.cbegin(); iter != s.cend(); ++iter) {
        stream << "  " << iter.name() << "\n";
    }
    stream << "}";
    return stream;
}

}
}

#endif
#include <vector>

/** \file
 * Methods for computing the upper and lower bounds of an expression,
 * and the regions of a function read or written by a statement.
 */

namespace Halide {
namespace Internal {

struct Interval {
    Expr min, max;
    Interval() {}
    Interval(Expr min, Expr max) : min(min), max(max) {}
};

typedef std::map<std::pair<std::string, int>, Interval> FuncValueBounds;

/** Given an expression in some variables, and a map from those
 * variables to their bounds (in the form of (minimum possible value,
 * maximum possible value)), compute two expressions that give the
 * minimum possible value and the maximum possible value of this
 * expression. Max or min may be undefined expressions if the value is
 * not bounded above or below.
 *
 * This is for tasks such as deducing the region of a buffer
 * loaded by a chunk of code.
 */
Interval bounds_of_expr_in_scope(Expr expr,
                                 const Scope<Interval> &scope,
                                 const FuncValueBounds &func_bounds = FuncValueBounds());

/** Represents the bounds of a region of arbitrary dimension. Zero
 * dimensions corresponds to a scalar region. */
struct Box {
    /** The conditions under which this region may be touched. */
    Expr used;

    /** The bounds if it is touched. */
    std::vector<Interval> bounds;

    Box() {}
    Box(size_t sz) : bounds(sz) {}
    Box(const std::vector<Interval> &b) : bounds(b) {}

    size_t size() const {return bounds.size();}
    bool empty() const {return bounds.empty();}
    Interval &operator[](int i) {return bounds[i];}
    const Interval &operator[](int i) const {return bounds[i];}
    void resize(size_t sz) {bounds.resize(sz);}
    void push_back(const Interval &i) {bounds.push_back(i);}

    /** Check if the used condition is defined and not trivially true. */
    bool maybe_unused() const {return used.defined() && !is_one(used);}
};

// Expand box a to encompass box b
void merge_boxes(Box &a, const Box &b);
// Test if box a could possibly overlap box b.
bool boxes_overlap(const Box &a, const Box &b);

/** Compute rectangular domains large enough to cover all the 'Call's
 * to each function that occurs within a given statement or
 * expression. This is useful for figuring out what regions of things
 * to evaluate. */
// @{
std::map<std::string, Box> boxes_required(Expr e,
                                          const Scope<Interval> &scope = Scope<Interval>::empty_scope(),
                                          const FuncValueBounds &func_bounds = FuncValueBounds());
std::map<std::string, Box> boxes_required(Stmt s,
                                          const Scope<Interval> &scope = Scope<Interval>::empty_scope(),
                                          const FuncValueBounds &func_bounds = FuncValueBounds());
// @}

/** Compute rectangular domains large enough to cover all the
 * 'Provides's to each function that occurs within a given statement
 * or expression. */
// @{
std::map<std::string, Box> boxes_provided(Expr e,
                                          const Scope<Interval> &scope = Scope<Interval>::empty_scope(),
                                          const FuncValueBounds &func_bounds = FuncValueBounds());
std::map<std::string, Box> boxes_provided(Stmt s,
                                          const Scope<Interval> &scope = Scope<Interval>::empty_scope(),
                                          const FuncValueBounds &func_bounds = FuncValueBounds());
// @}

/** Compute rectangular domains large enough to cover all the 'Call's
 * and 'Provides's to each function that occurs within a given
 * statement or expression. */
// @{
std::map<std::string, Box> boxes_touched(Expr e,
                                         const Scope<Interval> &scope = Scope<Interval>::empty_scope(),
                                         const FuncValueBounds &func_bounds = FuncValueBounds());
std::map<std::string, Box> boxes_touched(Stmt s,
                                         const Scope<Interval> &scope = Scope<Interval>::empty_scope(),
                                         const FuncValueBounds &func_bounds = FuncValueBounds());
// @}

/** Variants of the above that are only concerned with a single function. */
// @{
Box box_required(Expr e, std::string fn,
                 const Scope<Interval> &scope = Scope<Interval>::empty_scope(),
                 const FuncValueBounds &func_bounds = FuncValueBounds());
Box box_required(Stmt s, std::string fn,
                 const Scope<Interval> &scope = Scope<Interval>::empty_scope(),
                 const FuncValueBounds &func_bounds = FuncValueBounds());

Box box_provided(Expr e, std::string fn,
                 const Scope<Interval> &scope = Scope<Interval>::empty_scope(),
                 const FuncValueBounds &func_bounds = FuncValueBounds());
Box box_provided(Stmt s, std::string fn,
                 const Scope<Interval> &scope = Scope<Interval>::empty_scope(),
                 const FuncValueBounds &func_bounds = FuncValueBounds());

Box box_touched(Expr e, std::string fn,
                const Scope<Interval> &scope = Scope<Interval>::empty_scope(),
                const FuncValueBounds &func_bounds = FuncValueBounds());
Box box_touched(Stmt s, std::string fn,
                const Scope<Interval> &scope = Scope<Interval>::empty_scope(),
                const FuncValueBounds &func_bounds = FuncValueBounds());
// @}

/** Compute the maximum and minimum possible value for each function
 * in an environment. */
FuncValueBounds compute_function_value_bounds(const std::vector<std::string> &order,
                                              const std::map<std::string, Function> &env);

void bounds_test();

}
}

#endif
#ifndef HALIDE_BOUNDS_INFERENCE_H
#define HALIDE_BOUNDS_INFERENCE_H

/** \file
 * Defines the bounds_inference lowering pass.
 */

#include <map>


namespace Halide {
namespace Internal {

/** Take a partially lowered statement that includes symbolic
 * representations of the bounds over which things should be realized,
 * and inject expressions defining those bounds.
 */
Stmt bounds_inference(Stmt,
                      const std::vector<std::string> &realization_order,
                      const std::map<std::string, Function> &environment,
                      const std::map<std::pair<std::string, int>, Interval> &func_bounds);

}
}

#endif
#ifndef HALIDE_CODEGEN_C_H
#define HALIDE_CODEGEN_C_H

/** \file
 *
 * Defines an IRPrinter that emits C++ code equivalent to a halide stmt
 */

#include <string>
#include <vector>
#include <ostream>
#include <map>

#ifndef HALIDE_IR_PRINTER_H
#define HALIDE_IR_PRINTER_H

#include <ostream>


namespace Halide {

/** \file
 * This header file defines operators that let you dump a Halide
 * expression, statement, or type directly into an output stream
 * in a human readable form.
 * E.g:
 \code
 Expr foo = ...
 std::cout << "Foo is " << foo << std::endl;
 \endcode
 *
 * These operators are implemented using \ref Halide::Internal::IRPrinter
 */

/** Emit an expression on an output stream (such as std::cout) in a
 * human-readable form */
EXPORT std::ostream &operator<<(std::ostream &stream, const Expr &);

/** Emit a halide type on an output stream (such as std::cout) in a
 * human-readable form */
EXPORT std::ostream &operator<<(std::ostream &stream, const Type &);

namespace Internal {

/** Emit a halide statement on an output stream (such as std::cout) in
 * a human-readable form */
std::ostream &operator<<(std::ostream &stream, const Stmt &);

/** Emit a halide for loop type (vectorized, serial, etc) in a human
 * readable form */
std::ostream &operator<<(std::ostream &stream, const For::ForType &);

/** An IRVisitor that emits IR to the given output stream in a human
 * readable form. Can be subclassed if you want to modify the way in
 * which it prints.
 */
class IRPrinter : public IRVisitor {
public:
    /** Construct an IRPrinter pointed at a given output stream
     * (e.g. std::cout, or a std::ofstream) */
    IRPrinter(std::ostream &);

    /** emit an expression on the output stream */
    void print(Expr);

    /** emit a statement on the output stream */
    void print(Stmt);

    static void test();

protected:
    /** The stream we're outputting on */
    std::ostream &stream;

    /** The current indentation level, useful for pretty-printing
     * statements */
    int indent;

    /** Emit spaces according to the current indentation level */
    void do_indent();

    void visit(const IntImm *);
    void visit(const FloatImm *);
    void visit(const StringImm *);
    void visit(const Cast *);
    void visit(const Variable *);
    void visit(const Add *);
    void visit(const Sub *);
    void visit(const Mul *);
    void visit(const Div *);
    void visit(const Mod *);
    void visit(const Min *);
    void visit(const Max *);
    void visit(const EQ *);
    void visit(const NE *);
    void visit(const LT *);
    void visit(const LE *);
    void visit(const GT *);
    void visit(const GE *);
    void visit(const And *);
    void visit(const Or *);
    void visit(const Not *);
    void visit(const Select *);
    void visit(const Load *);
    void visit(const Ramp *);
    void visit(const Broadcast *);
    void visit(const Call *);
    void visit(const Let *);
    void visit(const LetStmt *);
    void visit(const AssertStmt *);
    void visit(const Pipeline *);
    void visit(const For *);
    void visit(const Store *);
    void visit(const Provide *);
    void visit(const Allocate *);
    void visit(const Free *);
    void visit(const Realize *);
    void visit(const Block *);
    void visit(const IfThenElse *);
    void visit(const Evaluate *);

};
}
}

#endif

namespace Halide {

struct Argument;

namespace Internal {

/** This class emits C++ code equivalent to a halide Stmt. It's
 * mostly the same as an IRPrinter, but it's wrapped in a function
 * definition, and some things are handled differently to be valid
 * C++.
 */
class CodeGen_C : public IRPrinter {
public:
    /** Initialize a C code generator pointing at a particular output
     * stream (e.g. a file, or std::cout) */
    CodeGen_C(std::ostream &);

    /** Emit source code equivalent to the given statement, wrapped in
     * a function with the given type signature */
    void compile(Stmt stmt, std::string name,
                 const std::vector<Argument> &args,
                 const std::vector<Buffer> &images_to_embed);

    /** Emit a header file defining a halide pipeline with the given
     * type signature */
    void compile_header(const std::string &name, const std::vector<Argument> &args);

    static void test();

protected:
    /** An ID for the most recently generated ssa variable */
    std::string id;

    /** A cache of generated values in scope */
    std::map<std::string, std::string> cache;

    /** Emit an expression as an assignment, then return the id of the
     * resulting var */
    std::string print_expr(Expr);

    /** Emit a statement */
    void print_stmt(Stmt);

    /** Emit the C name for a halide type */
    virtual std::string print_type(Type);

    /** Emit a statement to reinterpret an expression as another type */
    virtual std::string print_reinterpret(Type, Expr);

    /** Emit a version of a string that is a valid identifier in C (. is replaced with _) */
    virtual std::string print_name(const std::string &);

    /** Emit an SSA-style assignment, and set id to the freshly generated name. Return id. */
    std::string print_assignment(Type t, const std::string &rhs);

    /** Open a new C scope (i.e. throw in a brace, increase the indent) */
    void open_scope();

    /** Close a C scope (i.e. throw in an end brace, decrease the indent) */
    void close_scope(const std::string &comment);

    /** Unpack a buffer into its constituent parts */
    void unpack_buffer(Type t, const std::string &buffer_name);

    /** Track the types of allocations to avoid unnecessary casts. */
    Scope<Type> allocations;

    /** Track which allocations actually went on the heap. */
    Scope<int> heap_allocations;

    /** True if there is a void * __user_context parameter in the arguments. */
    bool have_user_context;

    using IRPrinter::visit;

    void visit(const Variable *);
    void visit(const IntImm *);
    void visit(const StringImm *);
    void visit(const FloatImm *);
    void visit(const Cast *);
    void visit(const Add *);
    void visit(const Sub *);
    void visit(const Mul *);
    void visit(const Div *);
    void visit(const Mod *);
    void visit(const Max *);
    void visit(const Min *);
    void visit(const EQ *);
    void visit(const NE *);
    void visit(const LT *);
    void visit(const LE *);
    void visit(const GT *);
    void visit(const GE *);
    void visit(const And *);
    void visit(const Or *);
    void visit(const Not *);
    void visit(const Call *);
    void visit(const Select *);
    void visit(const Load *);
    void visit(const Store *);
    void visit(const Let *);
    void visit(const LetStmt *);
    void visit(const AssertStmt *);
    void visit(const Pipeline *);
    void visit(const For *);
    void visit(const Provide *);
    void visit(const Allocate *);
    void visit(const Free *);
    void visit(const Realize *);
    void visit(const IfThenElse *);
    void visit(const Evaluate *);

    void visit_binop(Type t, Expr a, Expr b, const char *op);
};

}
}

#endif
#ifndef HALIDE_CODEGEN_H
#define HALIDE_CODEGEN_H

/** \file
 *
 * Defines the base-class for all architecture-specific code
 * generators that use llvm.
 */

namespace llvm {
class Value;
class Module;
class Function;
template<bool> class IRBuilderDefaultInserter;
class ConstantFolder;
template<bool, typename, typename> class IRBuilder;
class LLVMContext;
class Type;
class StructType;
class Instruction;
class CallInst;
class ExecutionEngine;
class AllocaInst;
class Constant;
class Triple;
class MDNode;
}

#include <map>
#include <stack>
#include <string>
#include <vector>

#ifndef HALIDE_JIT_COMPILED_MODULE_H
#define HALIDE_JIT_COMPILED_MODULE_H

/** \file
 * Defines the struct representing a JIT compiled halide pipeline
 */

#ifndef HALIDE_HALIDERUNTIME_H
#define HALIDE_HALIDERUNTIME_H

#ifndef COMPILING_HALIDE_RUNTIME
#include <stddef.h>
#include <stdint.h>
#else
#ifndef HALIDE_RUNTIME_INTERNAL_H
#define HALIDE_RUNTIME_INTERNAL_H

#if __STDC_HOSTED__
#error "Halide runtime files must be compiled with clang in freestanding mode."
#endif

#ifdef __UINT8_TYPE__
typedef __INT64_TYPE__ int64_t;
typedef __UINT64_TYPE__ uint64_t;
typedef __INT32_TYPE__ int32_t;
typedef __UINT32_TYPE__ uint32_t;
typedef __INT16_TYPE__ int16_t;
typedef __UINT16_TYPE__ uint16_t;
typedef __INT8_TYPE__ int8_t;
typedef __UINT8_TYPE__ uint8_t;
#else
typedef signed __INT64_TYPE__ int64_t;
typedef unsigned __INT64_TYPE__ uint64_t;
typedef signed __INT32_TYPE__ int32_t;
typedef unsigned __INT32_TYPE__ uint32_t;
typedef signed __INT16_TYPE__ int16_t;
typedef unsigned __INT16_TYPE__ uint16_t;
typedef signed __INT8_TYPE__ int8_t;
typedef unsigned __INT8_TYPE__ uint8_t;
#endif
typedef __SIZE_TYPE__ size_t;
typedef __PTRDIFF_TYPE__ ptrdiff_t;

typedef ptrdiff_t ssize_t;

#define NULL 0
#define WEAK __attribute__((weak))

#ifdef BITS_64
#define INT64_C(c)  c ## L
#define UINT64_C(c) c ## UL
typedef uint64_t uintptr_t;
typedef int64_t intptr_t;
#endif

#ifdef BITS_32
#define INT64_C(c)  c ## LL
#define UINT64_C(c) c ## ULL
typedef uint32_t uintptr_t;
typedef int32_t intptr_t;
#endif

#define STDOUT_FILENO 1
#define STDERR_FILENO 2

// Commonly-used extern functions
extern "C" {
WEAK int64_t halide_current_time_ns(void *user_context);
WEAK void halide_print(void *user_context, const char *msg);
WEAK void halide_error(void *user_context, const char *msg);

char *getenv(const char *);
void free(void *);
void *malloc(size_t);
const char *strstr(const char *, const char *);
int atoi(const char *);
int strcmp(const char* s, const char* t);
int strncmp(const char* s, const char* t, size_t n);
size_t strlen(const char* s);
char *strchr(const char* s, char c);
void* memcpy(void* s1, const void* s2, size_t n);
int memcmp(const void* s1, const void* s2, size_t n);
void *memset(void *s, int val, size_t n);
int open(const char *filename, int opts, int mode);
int close(int fd);
ssize_t write(int fd, const void *buf, size_t bytes);
void exit(int);

// Similar to strncpy, but with various non-string arguments. Writes
// arg to dst. Does not write to pointer end or beyond. Returns
// pointer to one beyond the last character written so that calls can
// be chained.
WEAK char *halide_string_to_string(char *dst, char *end, const char *arg);
WEAK char *halide_double_to_string(char *dst, char *end, double arg, int scientific);
WEAK char *halide_int64_to_string(char *dst, char *end, int64_t arg, int digits);
WEAK char *halide_uint64_to_string(char *dst, char *end, uint64_t arg, int digits);
WEAK char *halide_pointer_to_string(char *dst, char *end, const void *arg);

}

// A convenient namespace for weak functions that are internal to the
// halide runtime.
namespace Halide { namespace Runtime { namespace Internal {

enum PrinterType {BasicPrinter = 0,
                  ErrorPrinter = 1,
                  StringStreamPrinter = 2};

// A class for constructing debug messages from the runtime. Dumps
// items into a stack array, then prints them when the object leaves
// scope using halide_print. Think of it as a stringstream that prints
// when it dies. Use it like this:

// debug(user_context) << "A" << b << c << "\n";

// If you use it like this:

// debug d(user_context);
// d << "A";
// d << b;
// d << c << "\n";

// Then remember the print only happens when the debug object leaves
// scope, which may print at a confusing time.

template<int type>
class Printer {
public:
    char buf[1024];
    char *dst, *end;
    void *user_context;

    Printer(void *ctx) : dst(buf), end(buf + 1023), user_context(ctx) {
        *end = 0;
    }

    Printer &operator<<(const char *arg) {
        dst = halide_string_to_string(dst, end, arg);
        return *this;
    }

    Printer &operator<<(int64_t arg) {
        dst = halide_int64_to_string(dst, end, arg, 1);
        return *this;
    }

    Printer &operator<<(int32_t arg) {
        dst = halide_int64_to_string(dst, end, arg, 1);
        return *this;
    }

    Printer &operator<<(uint64_t arg) {
        dst = halide_uint64_to_string(dst, end, arg, 1);
        return *this;
    }

    Printer &operator<<(uint32_t arg) {
        dst = halide_uint64_to_string(dst, end, arg, 1);
        return *this;
    }

    Printer &operator<<(double arg) {
        dst = halide_double_to_string(dst, end, arg, 1);
        return *this;
    }

    Printer &operator<<(float arg) {
        dst = halide_double_to_string(dst, end, arg, 0);
        return *this;
    }

    Printer &operator<<(const void *arg) {
        dst = halide_pointer_to_string(dst, end, arg);
        return *this;
    }

    // Use it like a stringstream.
    const char *str() {
        return buf;
    }

    ~Printer() {
        if (type == ErrorPrinter) {
            halide_error(user_context, buf);
        } else if (type == BasicPrinter) {
            halide_print(user_context, buf);
        } else {
            // It's a stringstream. Do nothing.
        }
    }
};

// A class that supports << with all the same types as Printer, but
// does nothing and should compile to a no-op.
class SinkPrinter {
public:
    SinkPrinter(void *user_context) {}
};
template<typename T>
SinkPrinter operator<<(const SinkPrinter &s, T) {
    return s;
}

typedef Printer<BasicPrinter> print;
typedef Printer<ErrorPrinter> error;
typedef Printer<StringStreamPrinter> stringstream;

#ifdef DEBUG_RUNTIME
typedef Printer<BasicPrinter> debug;
#else
typedef SinkPrinter debug;
#endif

}}}

using namespace Halide::Runtime::Internal;

#endif

#endif

#ifdef __cplusplus
extern "C" {
#endif

/** \file
 *
 * This file exports all routines which can be replaced by an
 * application hosting code generated by %Halide. These are used when
 * doing Ahead Of Time (AOT) compilation and must be supplied to the
 * linker to override a routine. I.e., just define your own version of
 * any of these functions with extern "C" linkage, and it should
 * replace the default one.
 *
 * When doing Just In Time (JIT) compilation methods on the Func being
 * compiled must be called instead. The corresponding methods are
 * documented below.
 *
 * All of these functions take a "void *user_context" parameter as their
 * first argument; if the Halide kernel that calls back to any of these
 * funcions has been defined with a "__user_context" parameter XXXXXXXX,
 * then the value of that pointer passed from the code that calls the
 * Halide kernel is piped through to the function.
 *
 * Some of these are also useful to call when using the default
 * implementation. E.g. halide_shutdown_thread_pool.
 *
 * Note that some linker setups may not respect the override you
 * provide. E.g. if the override is in a shared library and the halide
 * object files are linked directly into the output, the builtin
 * versions of the runtime functions will be called. See your linker
 * documentation for more details. On Linux, LD_DYNAMIC_WEAK=1 may
 * help.
 *
 */

/** Print a message to stderr. Main use is to support HL_TRACE
 * functionality, print, and print_when calls. Also called by the default
 * halide_error.  This function can be replaced in JITed code by using
 * halide_custom_print and providing an implementation of halide_print
 * in AOT code. See Func::set_custom_print.
 */
extern void halide_print(void *user_context, const char *);

/** Define halide_error to catch errors messages at runtime (for
 * example bounds checking failures). This function can be replaced in
 * JITed code by using halide_set_error_handler and providing an
 * implementation of halide_error in AOT code. See
 * Func::set_error_handler.
 */
extern void halide_error(void *user_context, const char *);

/** A macro that calls halide_error if the supplied condition is false. */
#define halide_assert(user_context, cond) if (!(cond)) halide_error(user_context, #cond);

/** These are allocated statically inside the runtime, hence the fixed
 * size. They must be initialized with zero. The first time
 * halide_mutex_lock is called, the lock must be initialized in a
 * thread safe manner. This incurs a small overhead for a once
 * mechanism, but makes the lock reliably easy to setup and use
 * without depending on e.g. C++ constructor logic.
 */
struct halide_mutex {
    unsigned char _private[64];
};

/** A basic set of mutex functions, which call platform specific code
 * for mutual exclusion.
 */
//@{
extern void halide_mutex_lock(struct halide_mutex *mutex);
extern void halide_mutex_unlock(struct halide_mutex *mutex);
extern void halide_mutex_cleanup(struct halide_mutex *mutex_arg);
//@}

/** Define halide_do_par_for to replace the default thread pool
 * implementation. halide_shutdown_thread_pool can also be called to
 * release resources used by the default thread pool on platforms
 * where it makes sense. (E.g. On Mac OS, Grand Central Dispatch is
 * used so %Halide does not own the threads backing the pool and they
 * cannot be released.)  See Func::set_custom_do_task and
 * Func::set_custom_do_par_for. Should return zero if all the jobs
 * return zero, or an arbitrarily chosen return value from one of the
 * jobs otherwise.
 */
//@{
extern int halide_do_par_for(void *user_context,
                             int (*f)(void *ctx, int, uint8_t *),
                             int min, int size, uint8_t *closure);
extern void halide_shutdown_thread_pool();
//@}

/** Set the number of threads used by Halide's thread pool. No effect
 * on OS X or iOS. If changed after the first use of a parallel Halide
 * routine, shuts down and then reinitializes the thread pool. */
extern void halide_set_num_threads(int n);

/** Define halide_malloc and halide_free to replace the default memory
 * allocator.  See Func::set_custom_allocator. (Specifically note that
 * halide_malloc must return a 32-byte aligned pointer, and it must be
 * safe to read at least 8 bytes before the start and beyond the
 * end.)
 */
//@{
extern void *halide_malloc(void *user_context, size_t x);
extern void halide_free(void *user_context, void *ptr);
//@}

/** Called when debug_to_file is used inside %Halide code.  See
 * Func::debug_to_file for how this is called
 *
 * Cannot be replaced in JITted code at present.
 */
extern int32_t halide_debug_to_file(void *user_context, const char *filename,
                                    uint8_t *data, int32_t s0, int32_t s1, int32_t s2,
                                    int32_t s3, int32_t type_code,
                                    int32_t bytes_per_element);


enum halide_trace_event_code {halide_trace_load = 0,
                              halide_trace_store = 1,
                              halide_trace_begin_realization = 2,
                              halide_trace_end_realization = 3,
                              halide_trace_produce = 4,
                              halide_trace_update = 5,
                              halide_trace_consume = 6,
                              halide_trace_end_consume = 7};

struct halide_trace_event {
    const char *func;
    halide_trace_event_code event;
    int32_t parent_id;
    int32_t type_code;
    int32_t bits;
    int32_t vector_width;
    int32_t value_index;
    void *value;
    int32_t dimensions;
    int32_t *coordinates;
};

/** Called when Funcs are marked as trace_load, trace_store, or
 * trace_realization. See Func::set_custom_trace. The default
 * implementation either prints events via halide_printf, or if
 * HL_TRACE_FILE is defined, dumps the trace to that file in a
 * yet-to-be-documented binary format (see src/runtime/tracing.cpp to
 * reverse engineer the format). If the trace is going to be large,
 * you may want to make the file a named pipe, and then read from that
 * pipe into gzip.
 *
 * halide_trace returns a unique ID which will be passed to future
 * events that "belong" to the earlier event as the parent id. The
 * ownership hierarchy looks like:
 *
 * begin_realization
 *    produce
 *      store
 *      update
 *      load/store
 *      consume
 *      load
 *      end_consume
 *    end_realization
 *
 * Threading means that ownership cannot be inferred from the ordering
 * of events. There can be many active realizations of a given
 * function, or many active productions for a single
 * realization. Within a single production, the ordering of events is
 * meaningful.
 */
extern int32_t halide_trace(void *user_context, const halide_trace_event *event);

/** Set the file descriptor that Halide should write binary trace
 * events to. If called with 0 as the argument, Halide outputs trace
 * information to stdout in a human-readable format. If never called,
 * Halide checks the for existence of an environment variable called
 * HL_TRACE_FILE and opens that file. If HL_TRACE_FILE is not defined,
 * it outputs trace information to stdout in a human-readable
 * format. */
extern void halide_set_trace_file(int fd);

/** Halide calls this to retrieve the file descriptor to write binary
 * trace events to. The default implementation returns the value set
 * by halide_set_trace_file. Implement it yourself if you wish to use
 * a custom file descriptor per user_context. Return zero from your
 * implementation to tell Halide to print human-readable trace
 * information to stdout. */
extern int halide_get_trace_file(void *user_context);

/** If tracing is writing to a file. This call closes that file
 * (flushing the trace). Returns zero on success. */
extern int halide_shutdown_trace();

/** Release all data associated with the current GPU backend, in particular
 * all resources (memory, texture, context handles) allocated by Halide. Must
 * be called explicitly when using AOT compilation. */
extern void halide_release(void *user_context);

/** Copy image data from device memory to host memory. This must be called
 * explicitly to copy back the results of a GPU-based filter. */
extern int halide_copy_to_host(void *user_context, struct buffer_t *buf);

/** Copy image data from host memory to device memory. This should not be
 * called directly; Halide handles copying to the device automatically. */
extern int halide_copy_to_dev(void *user_context, struct buffer_t *buf);

/** Wait for current GPU operations to complete. Calling this explicitly
 * should rarely be necessary, except maybe for profiling. */
extern int halide_dev_sync(void *user_context);

/** Allocate device memory to back a buffer_t. */
extern int halide_dev_malloc(void *user_context, struct buffer_t *buf);

/** Free any device memory associated with a buffer_t. */
extern int halide_dev_free(void *user_context, struct buffer_t *buf);

/** These are forward declared here to ensure they have the same
 * signature across different Halide gpu backends. Do not call
 * them. */
// @{
extern int halide_init_kernels(void *user_context, void **state_ptr,
                               const char *src, int size);
extern int halide_dev_run(void *user_context,
                          void *state_ptr,
                          const char *entry_name,
                          int blocksX, int blocksY, int blocksZ,
                          int threadsX, int threadsY, int threadsZ,
                          int shared_mem_bytes,
                          size_t arg_sizes[],
                          void *args[]);
// @}

/** This function is called to populate the buffer_t.dev field with a constant
 * indicating that the OpenGL object corresponding to the buffer_t is bound by
 * the app and not by the Halide runtime. For example, the buffer_t may be
 * backed by an FBO already bound by the application. */
extern uint64_t halide_opengl_output_client_bound();

/** Forget all state associated with the previous OpenGL context.  This is
 * similar to halide_opengl_release, except that we assume that all OpenGL
 * resources have already been reclaimed by the OS. */
extern void halide_opengl_context_lost(void *user_context);

/** Set the platform name for OpenCL to use (e.g. "Intel" or
 * "NVIDIA"). The argument is copied internally. The opencl runtime
 * will select a platform that includes this as a substring. If never
 * called, Halide uses the environment variable HL_OCL_PLATFORM_NAME,
 * or defaults to the first available platform. */
extern void halide_set_ocl_platform_name(const char *n);

/** Halide calls this to get the desired OpenCL platform
 * name. Implement this yourself to use a different platform per
 * user_context. The default implementation returns the value set by
 * halide_set_ocl_platform_name, or the value of the environment
 * variable HL_OCL_PLATFORM_NAME. The output is valid until the next
 * call to halide_set_ocl_platform_name. */
extern const char *halide_get_ocl_platform_name(void *user_context);

/** Set the device type for OpenCL to use. The argument is copied
 * internally. It must be "cpu", "gpu", or "acc". If never called,
 * Halide uses the environment variable HL_OCL_DEVICE_TYPE. */
extern void halide_set_ocl_device_type(const char *n);

/** Halide calls this to gets the desired OpenCL device
 * type. Implement this yourself to use a different device type per
 * user_context. The default implementation returns the value set by
 * halide_set_ocl_device_type, or the environment variable
 * HL_OCL_DEVICE_TYPE. The result is valid until the next call to
 * halide_set_ocl_device_type. */
extern const char *halide_get_ocl_device_type(void *user_context);

/** Selects which gpu device to use. 0 is usually the display
 * device. If never called, Halide uses the environment variable
 * HL_GPU_DEVICE. If that variable is unset, Halide uses the last
 * device. Set this to -1 to use the last device. */
extern void halide_set_gpu_device(int n);

/** Halide calls this to get the desired halide gpu device
 * setting. Implement this yourself to use a different gpu device per
 * user_context. The default implementation returns the value set by
 * halide_set_gpu_device, or the environment variable
 * HL_GPU_DEVICE. */
extern int halide_get_gpu_device(void *user_context);

/** Set the soft maximum amount of memory, in bytes, that the LRU
 *  cache will use to memoize Func results.  This is not a strict
 *  maximum in that concurrency and simultaneous use of memoized
 *  reults larger than the cache size can both cause it to
 *  temporariliy be larger than the size specified here.
 */
extern void halide_memoization_cache_set_size(int64_t size);

/** Given a cache key for a memoized result, currently constructed
 *  from the Func name and top-level Func name plus the arguments of
 *  the computation, determine if the result is in the cache and
 *  return it if so. (The internals of the cache key should be
 *  considered opaque by this function.) If this routine returns true,
 *  it is a cache miss. Otherwise, it will return false and the
 *  buffers passed in will be filled, via copying, with memoized
 *  data. The last argument is a list if buffer_t pointers which
 *  represents the outputs of the memoized Func. If the Func does not
 *  return a Tuple, there will only be one buffer_t in the list. The
 *  tuple_count parameters determines the length of the list.
 */
extern bool halide_memoization_cache_lookup(void *user_context, const uint8_t *cache_key, int32_t size,
                                            buffer_t *realized_bounds, int32_t tuple_count, buffer_t **tuple_buffers);

/** Given a cache key for a memoized result, currently constructed
 *  from the Func name and top-level Func name plus the arguments of
 *  the computation, store the result in the cache for futre access by
 *  halide_memoization_cache_lookup. (The internals of the cache key
 *  should be considered opaque by this function.) Data is copied out
 *  from the inputs and inputs are unmodified. The last argument is a
 *  list if buffer_t pointers which represents the outputs of the
 *  memoized Func. If the Func does not return a Tuple, there will
 *  only be one buffer_t in the list. The tuple_count parameters
 *  determines the length of the list.
 */
extern void halide_memoization_cache_store(void *user_context, const uint8_t *cache_key, int32_t size,
                                           buffer_t *realized_bounds, int32_t tuple_count, buffer_t **tuple_buffers);


/** Free all memory and resources associated with the memoization cache.
 * Must be called at a time when no other threads are accessing the cache.
 */
extern void halide_memoization_cache_cleanup();

#ifdef __cplusplus
} // End extern "C"
#endif

#endif // HALIDE_HALIDERUNTIME_H

namespace llvm {
class Module;
}

namespace Halide {
namespace Internal {

class JITModuleHolder;
class CodeGen;

/** Function pointers into a compiled halide module. These function
 * pointers are meaningless once the last copy of a JITCompiledModule
 * is deleted, so don't cache them. */
struct JITCompiledModule {
    /** A pointer to the raw halide function. It's true type depends
     * on the Argument vector passed to CodeGen::compile. Image
     * parameters become (buffer_t *), and scalar parameters become
     * pointers to the appropriate values. The final argument is a
     * pointer to the buffer_t defining the output. */
    void *function;

    /** A slightly more type-safe wrapper around the raw halide
     * module. Takes it arguments as an array of pointers that
     * correspond to the arguments to \ref function */
    int (*wrapped_function)(const void **);

    /** JITed helpers to interact with device-mapped buffer_t
     * objects. These pointers may be NULL if not compiling for a
     * gpu-like target. */
    // @{
    int (*copy_to_host)(void *user_context, struct buffer_t*);
    int (*copy_to_dev)(void *user_context, struct buffer_t*);
    int (*free_dev_buffer)(void *user_context, struct buffer_t*);
    // @}

    /** The type of a halide runtime error handler function */
    typedef void (*ErrorHandler)(void *user_context, const char *);

    /** Set the runtime error handler for this module */
    void (*set_error_handler)(ErrorHandler);

    /** Set a custom malloc and free for this module to use. See
     * \ref Func::set_custom_allocator */
    void (*set_custom_allocator)(void *(*malloc)(void *user_context, size_t),
                                 void (*free)(void *user_context, void *ptr));

    /** Set a custom parallel for loop launcher. See
     * \ref Func::set_custom_do_par_for */
    typedef int (*HalideTask)(void *user_context, int, uint8_t *);
    void (*set_custom_do_par_for)(int (*custom_do_par_for)(void *user_context, HalideTask,
                                                           int, int, uint8_t *));

    /** Set a custom do parallel task. See
     * \ref Func::set_custom_do_task */
    void (*set_custom_do_task)(int (*custom_do_task)(void *user_context, HalideTask,
                                                     int, uint8_t *));

    /** Set a custom trace function. See \ref Func::set_custom_trace. */
    typedef int (*TraceFn)(void *, const halide_trace_event *);
    void (*set_custom_trace)(TraceFn);

    /** Set a custom print function for this module. See
     * \ref Func::set_custom_print. */
    void (*set_custom_print)(void (*custom_print)(void *, const char *));

    /** Shutdown the thread pool maintained by this JIT module. This
     * is also done automatically when the last reference to this
     * module is destroyed. */
    void (*shutdown_thread_pool)();

    /** Set the maximum number of bytes occupied by the cache for compute_cached. */
    void (*memoization_cache_set_size)(uint64_t size);

    // The JIT Module Allocator holds onto the memory storing the functions above.
    IntrusivePtr<JITModuleHolder> module;

    JITCompiledModule() :
        function(NULL),
        wrapped_function(NULL),
        copy_to_host(NULL),
        copy_to_dev(NULL),
        free_dev_buffer(NULL),
        set_error_handler(NULL),
        set_custom_allocator(NULL),
        set_custom_do_par_for(NULL),
        set_custom_do_task(NULL),
        set_custom_trace(NULL),
        set_custom_print(NULL),
        shutdown_thread_pool(NULL),
        memoization_cache_set_size(NULL) {}

    /** Take an llvm module and compile it. Populates the function
     * pointer members above with the result. */
    void compile_module(CodeGen *cg, llvm::Module *mod, const std::string &function_name);

    /** Holds a cleanup routine and context parameter. */
    struct CleanupRoutine {
        void (*fn)(void *);
        void *context;

        CleanupRoutine() : fn(NULL), context(NULL) {}
        CleanupRoutine(void (*fn)(void *), void *context) : fn(fn), context(context) {}
    };
};

}
}


#endif
#ifndef HALIDE_MODULUS_REMAINDER_H
#define HALIDE_MODULUS_REMAINDER_H

/** \file
 * Routines for statically determining what expressions are divisible by.
 */


namespace Halide {
namespace Internal {

/** The result of modulus_remainder analysis */
struct ModulusRemainder {
    ModulusRemainder() : modulus(0), remainder(0) {}
    ModulusRemainder(int m, int r) : modulus(m), remainder(r) {}
    int modulus, remainder;
};

/** For things like alignment analysis, often it's helpful to know
 * if an integer expression is some multiple of a constant plus
 * some other constant. For example, it is straight-forward to
 * deduce that ((10*x + 2)*(6*y - 3) - 1) is congruent to five
 * modulo six.
 *
 * We get the most information when the modulus is large. E.g. if
 * something is congruent to 208 modulo 384, then we also know it's
 * congruent to 0 mod 8, and we can possibly use it as an index for an
 * aligned load. If all else fails, we can just say that an integer is
 * congruent to zero modulo one.
 */
EXPORT ModulusRemainder modulus_remainder(Expr e);

/** If we have alignment information about external variables, we can
 * let the analysis know about that using this version of
 * modulus_remainder: */
ModulusRemainder modulus_remainder(Expr e, const Scope<ModulusRemainder> &scope);

/** Reduce an expression modulo some integer. Returns true and assigns
 * to remainder if an answer could be found. */
bool reduce_expr_modulo(Expr e, int modulus, int *remainder);

void modulus_remainder_test();

/** The greatest common divisor of two integers */
int gcd(int, int);

/** The least common multiple of two integers */
int lcm(int, int);

}
}

#endif
#ifndef HALIDE_TARGET_H
#define HALIDE_TARGET_H

/** \file
 * Defines the structure that describes a Halide target.
 */

#include <bitset>
#include <stdint.h>
#include <string>

namespace llvm {
class Module;
class LLVMContext;
}

namespace Halide {

/** A struct representing a target machine and os to generate code for. */
struct Target {
    /** The operating system used by the target. Determines which
     * system calls to generate. */
    enum OS {OSUnknown = 0, Linux, Windows, OSX, Android, IOS, NaCl} os;

    /** The architecture used by the target. Determines the
     * instruction set to use. For the PNaCl target, the "instruction
     * set" is actually llvm bitcode. */
    enum Arch {ArchUnknown = 0, X86, ARM, PNaCl, MIPS} arch;

    /** The bit-width of the target machine. Must be 0 for unknown, or 32 or 64. */
    int bits;

    /** Optional features a target can have. */
    enum Feature {
        JIT,  ///< Generate code that will run immediately inside the calling process.
        Debug,  ///< Turn on debug info and output for runtime code.
        NoAsserts,  ///< Disable all runtime checks, for slightly tighter code.
        NoBoundsQuery, ///< Disable the bounds querying functionality.

        SSE41,  ///< Use SSE 4.1 and earlier instructions. Only relevant on x86.
        AVX,  ///< Use AVX 1 instructions. Only relevant on x86.
        AVX2,  ///< Use AVX 2 instructions. Only relevant on x86.
        FMA,  ///< Enable x86 FMA instruction
        FMA4,  ///< Enable x86 (AMD) FMA4 instruction set
        F16C,  ///< Enable x86 16-bit float support

        ARMv7s,  ///< Generate code for ARMv7s. Only relevant for 32-bit ARM.

        CUDA,  ///< Enable the CUDA runtime. Defaults to compute capability 2.0 (Fermi)
        CUDACapability30,  ///< Enable CUDA compute capability 3.0 (Kepler)
        CUDACapability32,  ///< Enable CUDA compute capability 3.2 (Tegra K1)
        CUDACapability35,  ///< Enable CUDA compute capability 3.5 (Kepler)
        CUDACapability50,  ///< Enable CUDA compute capability 5.0 (Maxwell)

        OpenCL,  ///< Enable the OpenCL runtime.
        CLDoubles,  ///< Enable double support on OpenCL targets

        OpenGL,  ///< Enable the OpenGL runtime.

        FeatureEnd
        // NOTE: Changes to this enum must be reflected in the definition of
        // to_string()!
    };

    Target() : os(OSUnknown), arch(ArchUnknown), bits(0) {}
    Target(OS o, Arch a, int b, std::vector<Feature> initial_features = std::vector<Feature>())
        : os(o), arch(a), bits(b) {
        for (size_t i = 0; i < initial_features.size(); i++) {
            set_feature(initial_features[i]);
        }
    }

    void set_feature(Feature f, bool value = true) {
        user_assert(f < FeatureEnd) << "Invalid Target feature.\n";
        features.set(f, value);
    }

    void set_features(std::vector<Feature> features_to_set, bool value = true) {
        for (size_t i = 0; i < features_to_set.size(); i++) {
            set_feature(features_to_set[i], value);
        }
    }

    bool has_feature(Feature f) const {
        user_assert(f < FeatureEnd) << "Invalid Target feature.\n";
        return features[f];
    }

    bool features_any_of(std::vector<Feature> test_features) const {
        for (size_t i = 0; i < test_features.size(); i++) {
            user_assert(test_features[i] < FeatureEnd) << "Invalid Target feature.\n";

            if (features[test_features[i]]) {
                return true;
            }
        }
        return false;
    }

    bool features_all_of(std::vector<Feature> test_features) const {
        for (size_t i = 0; i < test_features.size(); i++) {
            user_assert(test_features[i] < FeatureEnd) << "Invalid Target feature.\n";

            if (!features[test_features[i]]) {
                return false;
            }
        }
        return true;
    }

    /** Return a copy of the target with the given feature set.
     * This is convenient when enabling certain features (e.g. NoBoundsQuery)
     * in an initialization list, where the target to be mutated may be
     * a const reference. */
    Target with_feature(Feature f) const {
        Target copy = *this;
        copy.set_feature(f);
        return copy;
    }

    /** Return a copy of the target with the given feature cleared.
     * This is convenient when disabling certain features (e.g. NoBoundsQuery)
     * in an initialization list, where the target to be mutated may be
     * a const reference. */
    Target without_feature(Feature f) const {
        Target copy = *this;
        copy.set_feature(f, false);
        return copy;
    }

    /** Is OpenCL or CUDA enabled in this target? I.e. is
     * Func::gpu_tile and similar going to work? We do not include
     * OpenGL, because it is not capable of gpgpu, and is not
     * scheduled via Func::gpu_tile. */
    bool has_gpu_feature() const {
        return has_feature(CUDA) || has_feature(OpenCL);
    }

    bool operator==(const Target &other) const {
      return os == other.os &&
          arch == other.arch &&
          bits == other.bits &&
          features == other.features;
    }

    bool operator!=(const Target &other) const {
      return !(*this == other);
    }

    /** Convert the Target into a string form that can be reconstituted
     * by merge_string(), which will always be of the form
     *
     *   arch-bits-os-feature1-feature2...featureN.
     *
     * Note that is guaranteed that t2.from_string(t1.to_string()) == t1,
     * but not that from_string(s).to_string() == s (since there can be
     * multiple strings that parse to the same Target)...
     * *unless* t1 contains 'unknown' fields (in which case you'll get a string
     * that can't be parsed, which is intentional).
     */
    EXPORT std::string to_string() const;

    /**
     * Parse the contents of 'target' and merge into 'this',
     * replacing only the parts that are specified. (e.g., if 'target' specifies
     * only an arch, only the arch field of 'this' will be changed, leaving
     * the other fields untouched). Any features specified in 'target'
     * are added to 'this', whether or not originally present.
     *
     * If the string contains unknown tokens, or multiple tokens of the
     * same category (e.g. multiple arch values), return false
     * (possibly leaving 'this' munged). (Multiple feature specifications
     * will not cause a failure.)
     *
     * If 'target' contains "host" as the first token, it replaces the entire
     * contents of 'this' with get_host_target(), then proceeds to parse the
     * remaining tokens (allowing for things like "host-opencl" to mean
     * "host configuration, but with opencl added").
     *
     * Note that unlike parse_from_string(), this will never print to cerr or
     * assert in the event of a parse failure. Note also that an empty target
     * string is essentially a no-op, leaving 'this' unaffected.
     */
    EXPORT bool merge_string(const std::string &target);

    /**
     * Like merge_string(), but reset the contents of 'this' first.
     */
    EXPORT bool from_string(const std::string &target) {
        *this = Target();
        return merge_string(target);
    }

private:
    /** A bitmask that stores the active features. */
    std::bitset<FeatureEnd> features;
};

/** Return the target corresponding to the host machine. */
EXPORT Target get_host_target();

/** Return the target that Halide will use. If HL_TARGET is set it
 * uses that. Otherwise calls \ref get_host_target */
EXPORT Target get_target_from_environment();

/** Return the target that Halide will use for jit-compilation. If
 * HL_JIT_TARGET is set it uses that. Otherwise calls \ref
 * get_host_target. Throws an error if the architecture, bit width,
 * and OS of the target do not match the host target, so this is only
 * useful for controlling the feature set. */
EXPORT Target get_jit_target_from_environment();

/** Given a string of the form used in HL_TARGET (e.g. "x86-64-avx"),
 * return the Target it specifies. Note that this always starts with
 * the result of get_host_target(), replacing only the parts found in the
 * target string, so if you omit (say) an OS specification, the host OS
 * will be used instead. An empty string is exactly equivalent to get_host_target().
 */
EXPORT Target parse_target_string(const std::string &target);

namespace Internal {

/** Create an llvm module containing the support code for a given target. */
llvm::Module *get_initial_module_for_target(Target, llvm::LLVMContext *);

/** Create an llvm module containing the support code for ptx device. */
llvm::Module *get_initial_module_for_ptx_device(Target, llvm::LLVMContext *c);

}

}


#endif

namespace Halide {
namespace Internal {

/** A code generator abstract base class. Actual code generators
 * (e.g. CodeGen_X86) inherit from this. This class is responsible
 * for taking a Halide Stmt and producing llvm bitcode, machine
 * code in an object file, or machine code accessible through a
 * function pointer.
 */
class CodeGen : public IRVisitor {
public:
    mutable RefCount ref_count;

    CodeGen(Target t);
    virtual ~CodeGen();

    /** Take a halide statement and compiles it to an llvm module held
     * internally. Call this before calling compile_to_bitcode or
     * compile_to_native. */
    virtual void compile(Stmt stmt, std::string name,
                         const std::vector<Argument> &args,
                         const std::vector<Buffer> &images_to_embed);

    /** Emit a compiled halide statement as llvm bitcode. Call this
     * after calling compile. */
    virtual void compile_to_bitcode(const std::string &filename);

    /** Emit a compiled halide statement as either an object file, or
     * as raw assembly, depending on the value of the second
     * argument. Call this after calling compile. */
    virtual void compile_to_native(const std::string &filename, bool assembly = false);

    /** Compile to machine code stored in memory, and return some
     * functions pointers into that machine code. */
    JITCompiledModule compile_to_function_pointers();

    /** What should be passed as -mcpu, -mattrs, and related for
     * compilation. The architecture-specific code generator should
     * define these. */
    // @{
    virtual std::string mcpu() const = 0;
    virtual std::string mattrs() const = 0;
    virtual bool use_soft_float_abi() const = 0;
    // @}

    /** Do any required target-specific things to the execution engine
     * and the module prior to jitting. Called by JITCompiledModule
     * just before it jits. Does nothing by default. */
    virtual void jit_init(llvm::ExecutionEngine *, llvm::Module *) {}

    /** Do any required target-specific things to the execution engine
     * and the module after jitting. Called by JITCompiledModule just
     * after it jits. Does nothing by default. The third argument
     * gives the target a chance to inject calls to target-specific
     * module cleanup routines. */
    virtual void jit_finalize(llvm::ExecutionEngine *, llvm::Module *,
                              std::vector<JITCompiledModule::CleanupRoutine> *);

    /** Initialize internal llvm state for the enabled targets. */
    static void initialize_llvm();

    /** Which built-in functions require a user-context first argument? */
    static bool function_takes_user_context(const std::string &name);

protected:

    /** State needed by llvm for code generation, including the
     * current module, function, context, builder, and most recently
     * generated llvm value. */
    //@{
    static bool llvm_initialized;
    static bool llvm_X86_enabled;
    static bool llvm_ARM_enabled;
    static bool llvm_AArch64_enabled;
    static bool llvm_NVPTX_enabled;
    static bool llvm_Mips_enabled;

    llvm::Module *module;
    bool owns_module;
    llvm::Function *function;
    llvm::LLVMContext *context;
    llvm::IRBuilder<true, llvm::ConstantFolder, llvm::IRBuilderDefaultInserter<true> > *builder;
    llvm::Value *value;
    llvm::MDNode *very_likely_branch;
    //@}

    /** The target we're generating code for */
    Halide::Target target;

    /** Initialize the CodeGen internal state to compile a fresh
     * module. This allows reuse of one CodeGen object to compiled
     * multiple related modules (e.g. multiple device kernels). */
    void init_module();

    /** Run all of llvm's optimization passes on the module. */
    void optimize_module();

    /** Add an entry to the symbol table, hiding previous entries with
     * the same name. Call this when new values come into scope. */
    void sym_push(const std::string &name, llvm::Value *value);

    /** Remove an entry for the symbol table, revealing any previous
     * entries with the same name. Call this when values go out of
     * scope. */
    void sym_pop(const std::string &name);

    /** Fetch an entry from the symbol table. If the symbol is not
     * found, it either errors out (if the second arg is true), or
     * returns NULL. */
    llvm::Value* sym_get(const std::string &name,
                         bool must_succeed = true) const;

    /** Test if an item exists in the symbol table. */
    bool sym_exists(const std::string &name) const;

    /** Some useful llvm types */
    // @{
    llvm::Type *void_t, *i1, *i8, *i16, *i32, *i64, *f16, *f32, *f64;
    llvm::StructType *buffer_t_type;
    // @}

    /** The name of the function being generated. */
    std::string function_name;

    /** Emit code that evaluates an expression, and return the llvm
     * representation of the result of the expression. */
    llvm::Value *codegen(Expr);

    /** Emit code that runs a statement. */
    void codegen(Stmt);

    /** Codegen a vector Expr by codegenning each lane and combining. */
    void scalarize(Expr);

    /** Take an llvm Value representing a pointer to a buffer_t,
     * and populate the symbol table with its constituent parts.
     */
    void unpack_buffer(std::string name, llvm::Value *buffer);

    /** Add a definition of buffer_t to the module if it isn't already there. */
    void define_buffer_t();

    /** Codegen an assertion. If false, it bails out and calls the
     * error handler. Either set message to non-NULL *or* pass a
     * vector of Expr arguments to print.  */
    // @{
    void create_assertion(llvm::Value *condition, Expr message);
    void create_assertion(llvm::Value *condition, const char *message) {
        create_assertion(condition, StringImm::make(message));
    }
    // @}

    /** Put a string constant in the module as a global variable and return a pointer to it. */
    llvm::Constant *create_string_constant(const std::string &str);

    /** Put a binary blob in the module as a global variable and return a pointer to it. */
    llvm::Constant *create_constant_binary_blob(const std::vector<char> &data, const std::string &name);

    /** Widen an llvm scalar into an llvm vector with the given number of lanes. */
    llvm::Value *create_broadcast(llvm::Value *, int width);

    /** Given an llvm value representing a pointer to a buffer_t, extract various subfields.
     * The *_ptr variants return a pointer to the struct element, while the basic variants
     * load the actual value. */
    // @{
    llvm::Value *buffer_host(llvm::Value *);
    llvm::Value *buffer_dev(llvm::Value *);
    llvm::Value *buffer_host_dirty(llvm::Value *);
    llvm::Value *buffer_dev_dirty(llvm::Value *);
    llvm::Value *buffer_min(llvm::Value *, int);
    llvm::Value *buffer_extent(llvm::Value *, int);
    llvm::Value *buffer_stride(llvm::Value *, int);
    llvm::Value *buffer_elem_size(llvm::Value *);
    llvm::Value *buffer_host_ptr(llvm::Value *);
    llvm::Value *buffer_dev_ptr(llvm::Value *);
    llvm::Value *buffer_host_dirty_ptr(llvm::Value *);
    llvm::Value *buffer_dev_dirty_ptr(llvm::Value *);
    llvm::Value *buffer_min_ptr(llvm::Value *, int);
    llvm::Value *buffer_extent_ptr(llvm::Value *, int);
    llvm::Value *buffer_stride_ptr(llvm::Value *, int);
    llvm::Value *buffer_elem_size_ptr(llvm::Value *);
    // @}

    /** Generate a pointer into a named buffer at a given index, of a
     * given type. The index counts according to the scalar type of
     * the type passed in. */
    // @{
    llvm::Value *codegen_buffer_pointer(std::string buffer, Type type, llvm::Value *index);
    llvm::Value *codegen_buffer_pointer(std::string buffer, Type type, Expr index);
    // @}

    /** Mark a load or store with type-based-alias-analysis metadata
     * so that llvm knows it can reorder loads and stores across
     * different buffers */
    void add_tbaa_metadata(llvm::Instruction *inst, std::string buffer, Expr index);

    using IRVisitor::visit;

    /** Generate code for various IR nodes. These can be overridden by
     * architecture-specific code to perform peephole
     * optimizations. The result of each is stored in \ref value */
    // @{
    virtual void visit(const IntImm *);
    virtual void visit(const FloatImm *);
    virtual void visit(const StringImm *);
    virtual void visit(const Cast *);
    virtual void visit(const Variable *);
    virtual void visit(const Add *);
    virtual void visit(const Sub *);
    virtual void visit(const Mul *);
    virtual void visit(const Div *);
    virtual void visit(const Mod *);
    virtual void visit(const Min *);
    virtual void visit(const Max *);
    virtual void visit(const EQ *);
    virtual void visit(const NE *);
    virtual void visit(const LT *);
    virtual void visit(const LE *);
    virtual void visit(const GT *);
    virtual void visit(const GE *);
    virtual void visit(const And *);
    virtual void visit(const Or *);
    virtual void visit(const Not *);
    virtual void visit(const Select *);
    virtual void visit(const Load *);
    virtual void visit(const Ramp *);
    virtual void visit(const Broadcast *);
    virtual void visit(const Call *);
    virtual void visit(const Let *);
    virtual void visit(const LetStmt *);
    virtual void visit(const AssertStmt *);
    virtual void visit(const Pipeline *);
    virtual void visit(const For *);
    virtual void visit(const Store *);
    virtual void visit(const Block *);
    virtual void visit(const IfThenElse *);
    virtual void visit(const Evaluate *);
    // @}

    /** Generate code for an allocate node. It has no default
     * implementation - it must be handled in an architecture-specific
     * way. */
    virtual void visit(const Allocate *) = 0;

    /** Generate code for a free node. It has no default
     * implementation and must be handled in an architecture-specific
     * way. */
    virtual void visit(const Free *) = 0;

    /** These IR nodes should have been removed during
     * lowering. CodeGen will error out if they are present */
    // @{
    virtual void visit(const Provide *);
    virtual void visit(const Realize *);
    // @}

    /** If we have to bail out of a pipeline midway, this should
     * inject the appropriate target-specific cleanup code. */
    virtual void prepare_for_early_exit() {}

    /** Get the llvm type equivalent to the given halide type in the
     * current context. */
    llvm::Type *llvm_type_of(Type);

    /** Perform an alloca at the function entrypoint. Will be cleaned
     * on function exit. */
    llvm::Value *create_alloca_at_entry(llvm::Type *type, int n, const std::string &name = "");

    /** Which buffers came in from the outside world (and so we can't
     * guarantee their alignment) */
    std::set<std::string> might_be_misaligned;

    llvm::Value *get_user_context() const;

    /** Implementation of the intrinsic call to
     * interleave_vectors. This implementation allows for interleaving
     * an arbitrary number of vectors.*/
    llvm::Value *interleave_vectors(Type, const std::vector<Expr>&);

private:

    /** All the values in scope at the current code location during
     * codegen. Use sym_push and sym_pop to access. */
    Scope<llvm::Value *> symbol_table;

    /** Alignment info for Int(32) variables in scope. */
    Scope<ModulusRemainder> alignment_info;

    /** String constants already emitted to the module. Tracked to
     * prevent emitting the same string many times. */
    std::map<std::string, llvm::Constant *> string_constants;
};

}}

#endif
#ifndef HALIDE_CODEGEN_X86_H
#define HALIDE_CODEGEN_X86_H

/** \file
 * Defines the code-generator for producing x86 machine code
 */

#ifndef HALIDE_CODEGEN_POSIX_H
#define HALIDE_CODEGEN_POSIX_H

/** \file
 * Defines a base-class for code-generators on posixy cpu platforms
 */


namespace Halide {
namespace Internal {

/** A code generator that emits posix code from a given Halide stmt. */
class CodeGen_Posix : public CodeGen {
public:

    /** Create an posix code generator. Processor features can be
     * enabled using the appropriate arguments */
    CodeGen_Posix(Target t);

protected:

    /** Some useful llvm types for subclasses */
    // @{
    llvm::Type *i8x8, *i8x16, *i8x32;
    llvm::Type *i16x4, *i16x8, *i16x16;
    llvm::Type *i32x2, *i32x4, *i32x8;
    llvm::Type *i64x2, *i64x4;
    llvm::Type *f32x2, *f32x4, *f32x8;
    llvm::Type *f64x2, *f64x4;
    // @}

    /** Some wildcard variables used for peephole optimizations in
     * subclasses */
    // @{
    Expr wild_i8x8, wild_i16x4, wild_i32x2; // 64-bit signed ints
    Expr wild_u8x8, wild_u16x4, wild_u32x2; // 64-bit unsigned ints
    Expr wild_i8x16, wild_i16x8, wild_i32x4, wild_i64x2; // 128-bit signed ints
    Expr wild_u8x16, wild_u16x8, wild_u32x4, wild_u64x2; // 128-bit unsigned ints
    Expr wild_i8x32, wild_i16x16, wild_i32x8, wild_i64x4; // 256-bit signed ints
    Expr wild_u8x32, wild_u16x16, wild_u32x8, wild_u64x4; // 256-bit unsigned ints
    Expr wild_f32x2; // 64-bit floats
    Expr wild_f32x4, wild_f64x2; // 128-bit floats
    Expr wild_f32x8, wild_f64x4; // 256-bit floats
    Expr min_i8, max_i8, max_u8;
    Expr min_i16, max_i16, max_u16;
    Expr min_i32, max_i32, max_u32;
    Expr min_i64, max_i64, max_u64;
    Expr min_f32, max_f32, min_f64, max_f64;
    // @}

    using CodeGen::visit;

    /** Posix implementation of Allocate. Small constant-sized allocations go
     * on the stack. The rest go on the heap by calling "halide_malloc"
     * and "halide_free" in the standard library. */
    // @{
    void visit(const Allocate *);
    void visit(const Free *);
    // @}

    /** A struct describing heap or stack allocations. */
    struct Allocation {
        llvm::Value *ptr;

        /** How many bytes this allocation is, or 0 if not
         * constant. */
        int constant_bytes;

        /** How many bytes of stack space used. 0 implies it was a
         * heap allocation. */
        int stack_bytes;
    };

    /** The allocations currently in scope. The stack gets pushed when
     * we enter a new function. */
    Scope<Allocation> allocations;

    /** Free all heap allocations in scope. */
    void prepare_for_early_exit();

    /** Initialize the CodeGen internal state to compile a fresh module */
    void init_module();

private:

    /** Stack allocations that were freed, but haven't gone out of
     * scope yet.  This allows us to re-use stack allocations when
     * they aren't being used. */
    std::vector<Allocation> free_stack_allocs;

    /** Generates code for computing the size of an allocation from a
     * list of its extents and its size. Fires a runtime assert
     * (halide_error) if the size overflows 2^31 -1, the maximum
     * positive number an int32_t can hold. */
    llvm::Value *codegen_allocation_size(const std::string &name, Type type, const std::vector<Expr> &extents);

    /** Allocates some memory on either the stack or the heap, and
     * returns an Allocation object describing it. For heap
     * allocations this calls halide_malloc in the runtime, and for
     * stack allocations it either reuses an existing block from the
     * free_stack_blocks list, or it saves the stack pointer and calls
     * alloca.
     *
     * This call returns the allocation, pushes it onto the
     * 'allocations' map, and adds an entry to the symbol table called
     * name.host that provides the base pointer.
     *
     * When the allocation can be freed call 'free_allocation', and
     * when it goes out of scope call 'destroy_allocation'. */
    Allocation create_allocation(const std::string &name, Type type,
                                 const std::vector<Expr> &extents,
                                 Expr condition);

    /** Free the memory backing an allocation and pop it from the
     * symbol table and the allocations map. For heap allocations it
     * calls halide_free in the runtime, for stack allocations it
     * marks the block as free so it can be reused. */
    void free_allocation(const std::string &name);
};

}}

#endif

namespace llvm {
class JITEventListener;
}

namespace Halide {
namespace Internal {

/** A code generator that emits x86 code from a given Halide stmt. */
class CodeGen_X86 : public CodeGen_Posix {
public:
    /** Create an x86 code generator. Processor features can be
     * enabled using the appropriate flags in the target struct. */
    CodeGen_X86(Target);

    /** Compile to an internally-held llvm module. Takes a halide
     * statement, the name of the function produced, and the arguments
     * to the function produced. After calling this, call
     * CodeGen::compile_to_file or
     * CodeGen::compile_to_function_pointer to get at the x86 machine
     * code. */
    void compile(Stmt stmt, std::string name,
                 const std::vector<Argument> &args,
                 const std::vector<Buffer> &images_to_embed);

    static void test();

    void jit_init(llvm::ExecutionEngine *, llvm::Module *);
    void jit_finalize(llvm::ExecutionEngine *, llvm::Module *, std::vector<JITCompiledModule::CleanupRoutine> *);

protected:

    llvm::Triple get_target_triple() const;

    /** Generate a call to an sse or avx intrinsic */
    // @{
    llvm::Value *call_intrin(Type t, const std::string &name, std::vector<Expr>);
    llvm::Value *call_intrin(llvm::Type *t, const std::string &name, std::vector<llvm::Value *>);
    // @}

    using CodeGen_Posix::visit;

    /** Nodes for which we want to emit specific sse/avx intrinsics */
    // @{
    void visit(const Add *);
    void visit(const Sub *);
    void visit(const Cast *);
    void visit(const Div *);
    void visit(const Min *);
    void visit(const Max *);
    // @}

    std::string mcpu() const;
    std::string mattrs() const;
    bool use_soft_float_abi() const;

private:
    llvm::JITEventListener* jitEventListener;
};

}}

#endif
#ifndef HALIDE_CODEGEN_GPU_HOST_H
#define HALIDE_CODEGEN_GPU_HOST_H

/** \file
 * Defines the code-generator for producing GPU host code
 */

#ifndef HALIDE_CODEGEN_ARM_H
#define HALIDE_CODEGEN_ARM_H

/** \file
 * Defines the code-generator for producing ARM machine code
 */


namespace Halide {
namespace Internal {

/** A code generator that emits ARM code from a given Halide stmt. */
class CodeGen_ARM : public CodeGen_Posix {
public:
    /** Create an ARM code generator for the given arm target. */
    CodeGen_ARM(Target);

    /** Compile to an internally-held llvm module. Takes a halide
     * statement, the name of the function produced, and the arguments
     * to the function produced. After calling this, call
     * CodeGen::compile_to_file or
     * CodeGen::compile_to_function_pointer to get at the ARM machine
     * code. */
    void compile(Stmt stmt, std::string name,
                 const std::vector<Argument> &args,
                 const std::vector<Buffer> &images_to_embed);

    static void test();

protected:

    llvm::Triple get_target_triple() const;

    /** Generate a call to a neon intrinsic */
    // @{
    llvm::Value *call_intrin(Type t, const std::string &name, std::vector<Expr>);
    llvm::Value *call_intrin(llvm::Type *t, const std::string &name, std::vector<llvm::Value *>);
    llvm::Instruction *call_void_intrin(const std::string &name, std::vector<Expr>);
    llvm::Instruction *call_void_intrin(const std::string &name, std::vector<llvm::Value *>);
    // @}

    using CodeGen_Posix::visit;

    /** Nodes for which we want to emit specific neon intrinsics */
    // @{
    void visit(const Cast *);
    void visit(const Add *);
    void visit(const Sub *);
    void visit(const Div *);
    void visit(const Mul *);
    void visit(const Min *);
    void visit(const Max *);
    void visit(const LT *);
    void visit(const LE *);
    void visit(const Select *);
    void visit(const Store *);
    void visit(const Load *);
    void visit(const Call *);
    // @}

    /** Various patterns to peephole match against */
    struct Pattern {
        std::string intrin;
        Expr pattern;
        enum PatternType {Simple = 0, LeftShift, RightShift, NarrowArgs};
        PatternType type;
        Pattern() {}
        Pattern(std::string i, Expr p, PatternType t = Simple) : intrin(i), pattern(p), type(t) {}
    };
    std::vector<Pattern> casts, left_shifts, averagings, negations;


    std::string mcpu() const;
    std::string mattrs() const;
    bool use_soft_float_abi() const;
};

}}

#endif
#ifndef HALIDE_CODEGEN_MIPS_H
#define HALIDE_CODEGEN_MIPS_H

/** \file
 * Defines the code-generator for producing MIPS machine code.
 */


namespace Halide {
namespace Internal {

/** A code generator that emits mips code from a given Halide stmt. */
class CodeGen_MIPS : public CodeGen_Posix {
public:
    /** Create a mips code generator. Processor features can be
     * enabled using the appropriate flags in the target struct. */
    CodeGen_MIPS(Target);

    /** Compile to an internally-held llvm module. Takes a halide
     * statement, the name of the function produced, and the arguments
     * to the function produced. After calling this, call
     * CodeGen::compile_to_file or
     * CodeGen::compile_to_function_pointer to get at the mips machine
     * code. */
    void compile(Stmt stmt, std::string name,
                 const std::vector<Argument> &args,
                 const std::vector<Buffer> &images_to_embed);

    static void test();

protected:

    llvm::Triple get_target_triple() const;

    using CodeGen_Posix::visit;

    std::string mcpu() const;
    std::string mattrs() const;
    bool use_soft_float_abi() const;
};

}}

#endif
#ifndef HALIDE_CODEGEN_PNACL_H
#define HALIDE_CODEGEN_PNACL_H

/** \file
 * Defines the code-generator for producing pnacl bitcode.
 */


namespace Halide {
namespace Internal {

/** A code generator that emits pnacl bitcode from a given Halide stmt. */
class CodeGen_PNaCl : public CodeGen_Posix {
public:
    /** Create a pnacl code generator. Processor features can be
     * enabled using the appropriate flags in the target struct. */
    CodeGen_PNaCl(Target);

    /** Compile to an internally-held llvm module. Takes a halide
     * statement, the name of the function produced, and the arguments
     * to the function produced. After calling this, call
     * CodeGen::compile_to_file or CodeGen::compile_to_bitcode to get
     * at the pnacl bitcode. */
    void compile(Stmt stmt, std::string name,
                 const std::vector<Argument> &args,
                 const std::vector<Buffer> &images_to_embed);

    /** The PNaCl backend overrides compile_to_native to
     * compile_to_bitcode instead. It does *not* run the pnacl
     * sandboxing passes, because these must be run after linking
     * (They change linkage qualifiers on everything, marking
     * everything as internal, including weak symbols that Halide
     * relies on being weak). The final linking stage (e.g. using
     * pnacl-clang++) handles the sandboxing. */
    void compile_to_native(const std::string &filename, bool /*assembly*/) {
        // TODO: Emit .ll when assembly is true
        compile_to_bitcode(filename);
    }

protected:

    using CodeGen_Posix::visit;

    std::string mcpu() const;
    std::string mattrs() const;
    bool use_soft_float_abi() const;
};

}}

#endif

namespace Halide {
namespace Internal {

struct CodeGen_GPU_Dev;
struct GPU_Argument;

/** A code generator that emits GPU code from a given Halide stmt. */
template<typename CodeGen_CPU>
class CodeGen_GPU_Host : public CodeGen_CPU {
public:

    /** Create a GPU code generator. GPU target is selected via
     * CodeGen_GPU_Options. Processor features can be enabled using the
     * appropriate flags from Target */
    CodeGen_GPU_Host(Target);

    virtual ~CodeGen_GPU_Host();

    /** Compile to an internally-held llvm module. Takes a halide
     * statement, the name of the function produced, and the arguments
     * to the function produced. After calling this, call
     * CodeGen::compile_to_file or
     * CodeGen::compile_to_function_pointer to get at the generated machine
     * code. */
    void compile(Stmt stmt, std::string name,
                 const std::vector<Argument> &args,
                 const std::vector<Buffer> &images_to_embed);

protected:
    /** Declare members of the base class that must exist to help the
     * compiler do name lookup. Annoying but necessary, because the
     * compiler doesn't know that CodeGen_CPU will in fact inherit
     * from CodeGen for every instantiation of this template. */
    using CodeGen_CPU::module;
    using CodeGen_CPU::init_module;
    using CodeGen_CPU::target;
    using CodeGen_CPU::builder;
    using CodeGen_CPU::context;
    using CodeGen_CPU::function;
    using CodeGen_CPU::get_user_context;
    using CodeGen_CPU::visit;
    using CodeGen_CPU::codegen;
    using CodeGen_CPU::sym_push;
    using CodeGen_CPU::sym_pop;
    using CodeGen_CPU::sym_get;
    using CodeGen_CPU::sym_exists;
    using CodeGen_CPU::buffer_dev_dirty_ptr;
    using CodeGen_CPU::buffer_host_dirty_ptr;
    using CodeGen_CPU::buffer_elem_size_ptr;
    using CodeGen_CPU::buffer_min_ptr;
    using CodeGen_CPU::buffer_stride_ptr;
    using CodeGen_CPU::buffer_extent_ptr;
    using CodeGen_CPU::buffer_host_ptr;
    using CodeGen_CPU::buffer_dev;
    using CodeGen_CPU::buffer_dev_ptr;
    using CodeGen_CPU::llvm_type_of;
    using CodeGen_CPU::create_alloca_at_entry;
    using CodeGen_CPU::i8;
    using CodeGen_CPU::i32;
    using CodeGen_CPU::i64;
    using CodeGen_CPU::buffer_t_type;
    using CodeGen_CPU::allocations;

    /** Nodes for which we need to override default behavior for the GPU runtime */
    // @{
    void visit(const For *);
    // @}

    /** Finds and links in the CUDA runtime symbols prior to jitting */
    void jit_init(llvm::ExecutionEngine *ee, llvm::Module *mod);

    /** Reaches inside the module at sets it to use a single shared
     * cuda context */
    void jit_finalize(llvm::ExecutionEngine *ee, llvm::Module *mod,
                      std::vector<JITCompiledModule::CleanupRoutine> *cleanup_routines);

    static bool lib_cuda_linked;

    static CodeGen_GPU_Dev* make_dev(Target);

    llvm::Value *get_module_state();

private:
    /** Child code generator for device kernels. */
    CodeGen_GPU_Dev *cgdev;
};


}}

#endif
#ifndef HALIDE_CODEGEN_PTX_DEV_H
#define HALIDE_CODEGEN_PTX_DEV_H

/** \file
 * Defines the code-generator for producing CUDA host code
 */

#ifndef HALIDE_CODEGEN_GPU_DEV_H
#define HALIDE_CODEGEN_GPU_DEV_H

/** \file
 * Defines the code-generator interface for producing GPU device code
 */


namespace Halide {
namespace Internal {

/** A bit more information attached to an argument useful for GPU backends. */
struct GPU_Argument : public Argument {
    /** The static size of the argument if known, or zero otherwise. */
    size_t size;

    GPU_Argument() : size(0) {}
    GPU_Argument(const std::string &_name, bool _is_buffer, Type _type) :
        Argument(_name, _is_buffer, _type), size(0) {}
    GPU_Argument(const std::string &_name, bool _is_buffer, Type _type,
                 size_t _size) :
        Argument(_name, _is_buffer, _type), size(_size) {}
};

/** A code generator that emits GPU code from a given Halide stmt. */
struct CodeGen_GPU_Dev {
    virtual ~CodeGen_GPU_Dev();

    /** Compile a GPU kernel into the module. This may be called many times
     * with different kernels, which will all be accumulated into a single
     * source module shared by a given Halide pipeline. */
    virtual void add_kernel(Stmt stmt,
                            const std::string &name,
                            const std::vector<GPU_Argument> &args) = 0;

    /** (Re)initialize the GPU kernel module. This is separate from compile,
     * since a GPU device module will often have many kernels compiled into it
     * for a single pipeline. */
    virtual void init_module() = 0;

    virtual std::vector<char> compile_to_src() = 0;

    virtual std::string get_current_kernel_name() = 0;

    virtual void dump() = 0;

    static bool is_gpu_var(const std::string &name);
    static bool is_gpu_block_var(const std::string &name);
    static bool is_gpu_thread_var(const std::string &name);

    /** Checks if expr is block uniform, i.e. does not depend on a thread
     * var. */
    static bool is_block_uniform(Expr expr);
    /** Checks if the buffer is a candidate for constant storage. Most
     * GPUs (APIs) support a constant memory storage class that cannot be
     * written to and performs well for block uniform accesses. */
    static bool is_buffer_constant(Stmt kernel, const std::string &buffer);
};

}}

#endif

namespace llvm {
class BasicBlock;
}

namespace Halide {
namespace Internal {

/** A code generator that emits GPU code from a given Halide stmt. */
class CodeGen_PTX_Dev : public CodeGen, public CodeGen_GPU_Dev {
public:
    friend class CodeGen_GPU_Host<CodeGen_X86>;
    friend class CodeGen_GPU_Host<CodeGen_ARM>;

    /** Create a PTX device code generator. */
    CodeGen_PTX_Dev(Target host);

    void add_kernel(Stmt stmt,
                    const std::string &name,
                    const std::vector<GPU_Argument> &args);

    /** (Re)initialize the PTX module. This is separate from compile, since
     * a PTX device module will often have many kernels compiled into it for
     * a single pipeline. */
    void init_module();

    static void test();

    std::vector<char> compile_to_src();
    std::string get_current_kernel_name();

    void dump();

protected:
    using CodeGen::visit;

    /** We hold onto the basic block at the start of the device
     * function in order to inject allocas */
    llvm::BasicBlock *entry_block;

    /** Nodes for which we need to override default behavior for the GPU runtime */
    // @{
    void visit(const For *);
    void visit(const Allocate *);
    void visit(const Free *);
    // @}

    std::string march() const;
    std::string mcpu() const;
    std::string mattrs() const;
    bool use_soft_float_abi() const;

    /** Map from simt variable names (e.g. foo.__block_id_x) to the llvm
     * ptx intrinsic functions to call to get them. */
    std::string simt_intrinsic(const std::string &name);
};

}}

#endif
#ifndef HALIDE_CODEGEN_OPENCL_DEV_H
#define HALIDE_CODEGEN_OPENCL_DEV_H

/** \file
 * Defines the code-generator for producing OpenCL C kernel code
 */

#include <sstream>


namespace Halide {
namespace Internal {

class CodeGen_OpenCL_Dev : public CodeGen_GPU_Dev {
public:
    CodeGen_OpenCL_Dev(Target target);

    /** Compile a GPU kernel into the module. This may be called many times
     * with different kernels, which will all be accumulated into a single
     * source module shared by a given Halide pipeline. */
    void add_kernel(Stmt stmt,
                    const std::string &name,
                    const std::vector<GPU_Argument> &args);

    /** (Re)initialize the GPU kernel module. This is separate from compile,
     * since a GPU device module will often have many kernels compiled into it
     * for a single pipeline. */
    void init_module();

    std::vector<char> compile_to_src();

    std::string get_current_kernel_name();

    void dump();

protected:

    class CodeGen_OpenCL_C : public CodeGen_C {
    public:
        CodeGen_OpenCL_C(std::ostream &s) : CodeGen_C(s) {}
        void add_kernel(Stmt stmt,
                        const std::string &name,
                        const std::vector<GPU_Argument> &args);

    protected:
        using CodeGen_C::visit;
        std::string print_type(Type type);
        std::string print_reinterpret(Type type, Expr e);

        std::string get_memory_space(const std::string &);

        void visit(const Div *);
        void visit(const Mod *);
        void visit(const For *);
        void visit(const Ramp *op);
        void visit(const Broadcast *op);
        void visit(const Load *op);
        void visit(const Store *op);
        void visit(const Cast *op);
        void visit(const Allocate *op);
        void visit(const Free *op);
    };

    std::ostringstream src_stream;
    std::string cur_kernel_name;
    CodeGen_OpenCL_C clc;
    Target target;
};

}}

#endif
#ifndef DEINTERLEAVE_H
#define DEINTERLEAVE_H

/** \file
 *
 * Defines methods for splitting up a vector into the even lanes and
 * the odd lanes. Useful for optimizing expressions such as select(x %
 * 2, f(x/2), g(x/2))
 */


namespace Halide {
namespace Internal {

/** Extract the odd-numbered lanes in a vector */
EXPORT Expr extract_odd_lanes(Expr a);

/** Extract the even-numbered lanes in a vector */
EXPORT Expr extract_even_lanes(Expr a);

/** Extract the nth lane of a vector */
EXPORT Expr extract_lane(Expr vec, int lane);

/** Look through a statement for expressions of the form select(ramp %
 * 2 == 0, a, b) and replace them with calls to an interleave
 * intrinsic */
Stmt rewrite_interleavings(Stmt s);

EXPORT void deinterleave_vector_test();

}
}

#endif
#ifndef HALIDE_DERIVATIVE_H
#define HALIDE_DERIVATIVE_H

/** \file
 *
 * Methods for taking derivatives of halide expressions. Not currently used anywhere
 */


namespace Halide {
namespace Internal {

/** Compute the analytic derivative of the expression with respect to
 * the variable. May returned an undefined Expr if it's
 * non-differentiable. */
//Expr derivative(Expr expr, const string &var);

/**
 * Compute the finite difference version of the derivative:
 * expr(var+1) - expr(var). The reason to do this as a derivative,
 * instead of just explicitly constructing expr(var+1) -
 * expr(var), is so that we don't have to do so much
 * simplification later. For example, the finite-difference
 * derivative of 2*x is trivially 2, whereas 2*(x+1) - 2*x may or
 * may not simplify down to 2, depending on the quality of our
 * simplification routine.
 *
 * Most rules for the finite difference and the true derivative
 * are the same. The quotient and product rules are not.
 *
 */
Expr finite_difference(Expr expr, const std::string &var);

/**
 * Detect whether an expression is monotonic increasing in a variable,
 * decreasing, or unknown. Returns -1, 0, or 1 for decreasing,
 * unknown, and increasing.
 */
enum MonotonicResult {Constant, MonotonicIncreasing, MonotonicDecreasing, Unknown};
MonotonicResult is_monotonic(Expr e, const std::string &var);

}
}

#endif
#ifndef HALIDE_ONE_TO_ONE_H
#define HALIDE_ONE_TO_ONE_H

/** \file
 *
 * Methods for determining if an Expr represents a one-to-one function
 * in its Variables.
 */


namespace Halide {
namespace Internal {

/** Conservatively determine whether an integer expression is
 * one-to-one in its variables. For now this means it contains a
 * single variable and its derivative is provably strictly positive or
 * strictly negative. */
bool is_one_to_one(Expr expr);

void is_one_to_one_test();

}
}

#endif
#ifndef HALIDE_EXTERN_H
#define HALIDE_EXTERN_H


/** \file
 *
 * Convenience macros that lift functions that take C types into
 * functions that take and return exprs, and call the original
 * function at runtime under the hood. See test/c_function.cpp for
 * example usage.
 */

#define _halide_check_arg_type(t, name, e, n)                     \
    _halide_user_assert(e.type() == t) << "Type mismatch for argument " << n << " to extern function " << #name << ". Type expected is " << t << " but the argument " << e << " has type " << e.type() << ".\n";

#define HalideExtern_1(rt, name, t1)                                    \
    Halide::Expr name(Halide::Expr a1) {                                \
        _halide_check_arg_type(Halide::type_of<t1>(), name, a1, 1);     \
        return Halide::Internal::Call::make(Halide::type_of<rt>(), #name, vec(a1), Halide::Internal::Call::Extern); \
    }

#define HalideExtern_2(rt, name, t1, t2)                                \
    Halide::Expr name(Halide::Expr a1, Halide::Expr a2) {               \
        _halide_check_arg_type(Halide::type_of<t1>(), name, a1, 1);                        \
        _halide_check_arg_type(Halide::type_of<t2>(), name, a2, 2);                        \
        return Halide::Internal::Call::make(Halide::type_of<rt>(), #name, vec(a1, a2), Halide::Internal::Call::Extern); \
    }

#define HalideExtern_3(rt, name, t1, t2, t3)                            \
    Halide::Expr name(Halide::Expr a1, Halide::Expr a2, Halide::Expr a3) { \
        _halide_check_arg_type(Halide::type_of<t1>(), name, a1, 1);                                \
        _halide_check_arg_type(Halide::type_of<t2>(), name, a2, 2);                                \
        _halide_check_arg_type(Halide::type_of<t3>(), name, a3, 3);                                \
        return Halide::Internal::Call::make(Halide::type_of<rt>(), #name, vec(a1, a2, a3), Halide::Internal::Call::Extern); \
    }

#define HalideExtern_4(rt, name, t1, t2, t3, t4)                        \
    Halide::Expr name(Halide::Expr a1, Halide::Expr a2, Halide::Expr a3, Halide::Expr a4) { \
        _halide_check_arg_type(Halide::type_of<t1>(), name, a1, 1);                                \
        _halide_check_arg_type(Halide::type_of<t2>(), name, a2, 2);                                \
        _halide_check_arg_type(Halide::type_of<t3>(), name, a3, 3);                                \
        _halide_check_arg_type(Halide::type_of<t4>(), name, a4, 4);                                \
        return Halide::Internal::Call::make(Halide::type_of<rt>(), #name, vec(a1, a2, a3, a4), Halide::Internal::Call::Extern); \
  }

#define HalideExtern_5(rt, name, t1, t2, t3, t4, t5)                       \
    Halide::Expr name(Halide::Expr a1, Halide::Expr a2, Halide::Expr a3, Halide::Expr a4, Halide::Expr a5) { \
        _halide_check_arg_type(Halide::type_of<t1>(), name, a1, 1);                                \
        _halide_check_arg_type(Halide::type_of<t2>(), name, a2, 2);                                \
        _halide_check_arg_type(Halide::type_of<t3>(), name, a3, 3);                                \
        _halide_check_arg_type(Halide::type_of<t4>(), name, a4, 4);                                \
        _halide_check_arg_type(Halide::type_of<t5>(), name, a5, 5);                                \
        return Halide::Internal::Call::make(Halide::type_of<rt>(), #name, vec(a1, a2, a3, a4, a5), Halide::Internal::Call::Extern); \
  }

#endif
#ifndef HALIDE_FUNC_H
#define HALIDE_FUNC_H

/** \file
 *
 * Defines Func - the front-end handle on a halide function, and related classes.
 */

#ifndef HALIDE_VAR_H
#define HALIDE_VAR_H

/** \file
 * Defines the Var - the front-end variable
 */

#include <string>
#include <sstream>

namespace Halide {

/** A Halide variable, to be used when defining functions. It is just
 * a name, and can be reused in places where no name conflict will
 * occur. It can be used in the left-hand-side of a function
 * definition, or as an Expr. As an Expr, it always has type
 * Int(32). */
class Var {
    std::string _name;
public:
    /** Construct a Var with the given name */
    Var(const std::string &n) : _name(n) {
        // Make sure we don't get a unique name with the same name as
        // this later:
        Internal::unique_name(n, false);
    }

    /** Construct a Var with an automatically-generated unique name. */
    Var() : _name(Internal::make_entity_name(this, "Halide::Var", 'v')) {}

    /** Get the name of a Var */
    const std::string &name() const {return _name;}

    /** Test if two Vars are the same. This simply compares the names. */
    bool same_as(const Var &other) const {return _name == other._name;}

    /** Implicit var constructor. Implicit variables are injected
     * automatically into a function call if the number of arguments
     * to the function are fewer than its dimensionality and a
     * placeholder ("_") appears in its argument list. Defining a
     * function to equal an expression containing implicit variables
     * similarly appends those implicit variables, in the same order,
     * to the left-hand-side of the definition where the placeholder
     * ('_') appears.
     *
     * For example, consider the definition:
     *
     \code
     Func f, g;
     Var x, y;
     f(x, y) = 3;
     \endcode
     *
     * A call to f with the placeholder symbol \ref _
     * will have implicit arguments injected automatically, so f(2, \ref _)
     * is equivalent to f(2, \ref _0), where \ref _0 = Var::implicit(0), and f(\ref _)
     * (and indeed f when cast to an Expr) is equivalent to f(\ref _0, \ref _1).
     * The following definitions are all equivalent, differing only in the
     * variable names.
     *
     \code
     g(_) = f*3;
     g(_) = f(_)*3;
     g(x, _) = f(x, _)*3;
     g(x, y) = f(x, y)*3;
     \endcode
     *
     * These are expanded internally as follows:
     *
     \code
     g(_0, _1) = f(_0, _1)*3;
     g(_0, _1) = f(_0, _1)*3;
     g(x, _0) = f(x, _0)*3;
     g(x, y) = f(x, y)*3;
     \endcode
     *
     * The following, however, defines g as four dimensional:
     \code
     g(x, y, _) = f*3;
     \endcode
     *
     * It is equivalent to:
     *
     \code
     g(x, y, _0, _1) = f(_0, _1)*3;
     \endcode
     *
     * Expressions requiring differing numbers of implicit variables
     * can be combined. The left-hand-side of a definition injects
     * enough implicit variables to cover all of them:
     *
     \code
     Func h;
     h(x) = x*3;
     g(x) = h + (f + f(x)) * f(x, y);
     \endcode
     *
     * expands to:
     *
     \code
     Func h;
     h(x) = x*3;
     g(x, _0, _1) = h(_0) + (f(_0, _1) + f(x, _0)) * f(x, y);
     \endcode
     *
     * The first ten implicits, _0 through _9, are predeclared in this
     * header and can be used for scheduling. They should never be
     * used as arguments in a declaration or used in a call.
     *
     * While it is possible to use Var::implicit or the predeclared
     * implicits to create expressions that can be treated as small
     * anonymous functions (e.g. Func(_0 + _1)) this is considered
     * poor style. Instead use \ref lambda.
     */
    static Var implicit(int n) {
        std::ostringstream str;
        str << "_" << n;
        return Var(str.str());
    }

    /** Return whether a variable name is of the form for an implicit argument.
     * TODO: This is almost guaranteed to incorrectly fire on user
     * declared variables at some point. We should likely prevent
     * user Var declarations from making names of this form.
     */
    //{
    static bool is_implicit(const std::string &name) {
        return Internal::starts_with(name, "_") &&
            name.find_first_not_of("0123456789", 1) == std::string::npos;
    }
    bool is_implicit() const {
        return is_implicit(name());
    }
    //}

    /** Return the argument index for a placeholder argument given its
     *  name. Returns 0 for \ref _0, 1 for \ref _1, etc. Returns -1 if
     *  the variable is not of implicit form.
     */
    //{
    static int implicit_index(const std::string &name) {
        return is_implicit(name) ? atoi(name.c_str() + 1) : -1;
    }
    int implicit_index() const {
        return implicit_index(name());
    }
    //}

    /** Test if a var is the placeholder variable \ref _ */
    //{
    static bool is_placeholder(const std::string &name) {
        return name == "_";
    }
    bool is_placeholder() const {
        return is_placeholder(name());
    }
    //}

    /** A Var can be treated as an Expr of type Int(32) */
    operator Expr() const {
        return Internal::Variable::make(Int(32), name());
    }

    /** Vars to use for scheduling producer/consumer pairs on the gpu. */
    // @{
    static Var gpu_blocks() {
        return Var("__block_id_x");
    }
    static Var gpu_threads() {
        return Var("__thread_id_x");
    }
    // @}

    /** A Var that represents the location outside the outermost loop. */
    static Var outermost() {
        return Var("__outermost");
    }

};

/** A placeholder variable for infered arguments. See \ref Var::implicit */
EXPORT extern Var _;

/** The first ten implicit Vars for use in scheduling. See \ref Var::implicit */
// @{
EXPORT extern Var _0, _1, _2, _3, _4, _5, _6, _7, _8, _9;
// @}

}

#endif
#ifndef HALIDE_PARAM_H
#define HALIDE_PARAM_H

/** \file
 *
 * Classes for declaring scalar and image parameters to halide pipelines
 */

#include <sstream>
#include <vector>


namespace Halide {

/** A scalar parameter to a halide pipeline. If you're jitting, this
 * should be bound to an actual value of type T using the set method
 * before you realize the function uses this. If you're statically
 * compiling, this param should appear in the argument list. */
template<typename T>
class Param {
    /** A reference-counted handle on the internal parameter object */
    Internal::Parameter param;

public:
    /** Construct a scalar parameter of type T with a unique
     * auto-generated name */
    Param() : param(type_of<T>(), false, Internal::make_entity_name(this, "Halide::Param<?", 'p')) {}

    /** Construct a scalar parameter of type T with the given name */
    Param(const std::string &n) : param(type_of<T>(), false, n) {}

    /** Get the name of this parameter */
    const std::string &name() const {
        return param.name();
    }

    /** Get the current value of this parameter. Only meaningful when jitting. */
    NO_INLINE T get() const {
        return param.get_scalar<T>();
    }

    /** Set the current value of this parameter. Only meaningful when jitting */
    NO_INLINE void set(T val) {
        param.set_scalar<T>(val);
    }

    /** Get a pointer to the location that stores the current value of
     * this parameter. Only meaningful for jitting. */
    NO_INLINE T *get_address() const {
        return (T *)(param.get_scalar_address());
    }

    /** Get the halide type of T */
    Type type() const {
        return type_of<T>();
    }

    /** Get or set the possible range of this parameter. Use undefined
     * Exprs to mean unbounded. */
    // @{
    void set_range(Expr min, Expr max) {
        set_min_value(min);
        set_max_value(max);
    }

    void set_min_value(Expr min) {
        if (min.type() != type_of<T>()) {
            min = Internal::Cast::make(type_of<T>(), min);
        }
        param.set_min_value(min);
    }

    void set_max_value(Expr max) {
        if (max.type() != type_of<T>()) {
            max = Internal::Cast::make(type_of<T>(), max);
        }
        param.set_max_value(max);
    }

    Expr get_min_value() {
        return param.get_min_value();
    }

    Expr get_max_value() {
        return param.get_max_value();
    }
    // @}

    /** You can use this parameter as an expression in a halide
     * function definition */
    operator Expr() const {
        return Internal::Variable::make(type_of<T>(), name(), param);
    }

    /** Using a param as the argument to an external stage treats it
     * as an Expr */
    operator ExternFuncArgument() const {
        return Expr(*this);
    }

    /** Construct the appropriate argument matching this parameter,
     * for the purpose of generating the right type signature when
     * statically compiling halide pipelines. */
    operator Argument() const {
        return Argument(name(), false, type());
    }
};

/** Returns a Param corresponding to a pointer to a user context
 * structure; when the Halide function that takes such a parameter
 * calls a function from the Halide runtime (e.g. halide_printf()), it
 * passes the value of this pointer as the first argument to the
 * runtime function.  */
inline Param<void *> user_context_param() {
  return Param<void *>("__user_context");
}

/** A handle on the output buffer of a pipeline. Used to make static
 * promises about the output size and stride. */
class OutputImageParam {
protected:
    /** A reference-counted handle on the internal parameter object */
    Internal::Parameter param;

    /** The dimensionality of this image. */
    int dims;

    void add_implicit_args_if_placeholder(std::vector<Expr> &args,
                                          Expr last_arg,
                                          int total_args,
                                          bool *placeholder_seen) const;
public:

    /** Construct a NULL image parameter handle. */
    OutputImageParam() : dims(0) {}

    /** Construct an OutputImageParam that wraps an Internal Parameter object. */
    EXPORT OutputImageParam(const Internal::Parameter &p, int d);

    /** Get the name of this Param */
    EXPORT const std::string &name() const;

    /** Get the type of the image data this Param refers to */
    EXPORT Type type() const;

    /** Is this parameter handle non-NULL */
    EXPORT bool defined();

    /** Get an expression representing the minimum coordinates of this image
     * parameter in the given dimension. */
    EXPORT Expr min(int x) const;

    /** Get an expression representing the extent of this image
     * parameter in the given dimension */
    EXPORT Expr extent(int x) const;

    /** Get an expression representing the stride of this image in the
     * given dimension */
    EXPORT Expr stride(int x) const;

    /** Set the extent in a given dimension to equal the given
     * expression. Images passed in that fail this check will generate
     * a runtime error. Returns a reference to the ImageParam so that
     * these calls may be chained.
     *
     * This may help the compiler generate better
     * code. E.g:
     \code
     im.set_extent(0, 100);
     \endcode
     * tells the compiler that dimension zero must be of extent 100,
     * which may result in simplification of boundary checks. The
     * value can be an arbitrary expression:
     \code
     im.set_extent(0, im.extent(1));
     \endcode
     * declares that im is a square image (of unknown size), whereas:
     \code
     im.set_extent(0, (im.extent(0)/32)*32);
     \endcode
     * tells the compiler that the extent is a multiple of 32. */
    EXPORT OutputImageParam &set_extent(int dim, Expr extent);

    /** Set the min in a given dimension to equal the given
     * expression. Setting the mins to zero may simplify some
     * addressing math. */
    EXPORT OutputImageParam &set_min(int dim, Expr min);

    /** Set the stride in a given dimension to equal the given
     * value. This is particularly helpful to set when
     * vectorizing. Known strides for the vectorized dimension
     * generate better code. */
    EXPORT OutputImageParam &set_stride(int dim, Expr stride);

    /** Set the min and extent in one call. */
    EXPORT OutputImageParam &set_bounds(int dim, Expr min, Expr extent);

    /** Get the dimensionality of this image parameter */
    EXPORT int dimensions() const;

    /** Get an expression giving the minimum coordinate in dimension 0, which
     * by convention is the coordinate of the left edge of the image */
    EXPORT Expr left() const;

    /** Get an expression giving the maximum coordinate in dimension 0, which
     * by convention is the coordinate of the right edge of the image */
    EXPORT Expr right() const;

    /** Get an expression giving the minimum coordinate in dimension 1, which
     * by convention is the top of the image */
    EXPORT Expr top() const;

    /** Get an expression giving the maximum coordinate in dimension 1, which
     * by convention is the bottom of the image */
    EXPORT Expr bottom() const;

    /** Get an expression giving the extent in dimension 0, which by
     * convention is the width of the image */
    EXPORT Expr width() const;

    /** Get an expression giving the extent in dimension 1, which by
     * convention is the height of the image */
    EXPORT Expr height() const;

    /** Get an expression giving the extent in dimension 2, which by
     * convention is the channel-count of the image */
    EXPORT Expr channels() const;

    /** Get at the internal parameter object representing this ImageParam. */
    EXPORT Internal::Parameter parameter() const;

    /** Construct the appropriate argument matching this parameter,
     * for the purpose of generating the right type signature when
     * statically compiling halide pipelines. */
    EXPORT operator Argument() const;

    /** Using a param as the argument to an external stage treats it
     * as an Expr */
    EXPORT operator ExternFuncArgument() const;
};

/** An Image parameter to a halide pipeline. E.g., the input image. */
class ImageParam : public OutputImageParam {

public:

    /** Construct a NULL image parameter handle. */
    ImageParam() : OutputImageParam() {}

    /** Construct an image parameter of the given type and
     * dimensionality, with an auto-generated unique name. */
    EXPORT ImageParam(Type t, int d);

    /** Construct an image parameter of the given type and
     * dimensionality, with the given name */
    EXPORT ImageParam(Type t, int d, const std::string &n);

    /** Bind a buffer or image to this ImageParam. Only relevant for jitting */
    EXPORT void set(Buffer b);

    /** Get the buffer bound to this ImageParam. Only relevant for jitting */
    EXPORT Buffer get() const;

    /** Construct an expression which loads from this image
     * parameter. The location is extended with enough implicit
     * variables to match the dimensionality of the image
     * (see \ref Var::implicit)
     */
    // @{
    EXPORT Expr operator()() const;
    EXPORT Expr operator()(Expr x) const;
    EXPORT Expr operator()(Expr x, Expr y) const;
    EXPORT Expr operator()(Expr x, Expr y, Expr z) const;
    EXPORT Expr operator()(Expr x, Expr y, Expr z, Expr w) const;
    EXPORT Expr operator()(std::vector<Expr>) const;
    EXPORT Expr operator()(std::vector<Var>) const;
    // @}

    /** Treating the image parameter as an Expr is equivalent to call
     * it with no arguments. For example, you can say:
     *
     \code
     ImageParam im(UInt(8), 2);
     Func f;
     f = im*2;
     \endcode
     *
     * This will define f as a two-dimensional function with value at
     * position (x, y) equal to twice the value of the image parameter
     * at the same location.
     */
    operator Expr() const {
        return (*this)(_);
    }

};

}

#endif
#ifndef HALIDE_RDOM_H
#define HALIDE_RDOM_H

/** \file
 * Defines the front-end syntax for reduction domains and reduction
 * variables.
 */


namespace Halide {

/** A reduction variable represents a single dimension of a reduction
 * domain (RDom). Don't construct them directly, instead construct an
 * RDom, and use RDom::operator[] to get at the variables. For
 * single-dimensional reduction domains, you can just cast a
 * single-dimensional RDom to an RVar. */
class RVar {
    std::string _name;
    Internal::ReductionDomain _domain;
    int _index;

    const Internal::ReductionVariable &_var() const {
        return _domain.domain().at(_index);
    }

public:
    /** An empty reduction variable. */
    RVar() : _name(Internal::make_entity_name(this, "Halide::RVar", 'r')) {}

    /** Construct an RVar with the given name */
    explicit RVar(const std::string &n) : _name(n) {
        // Make sure we don't get a unique name with the same name as
        // this later:
        Internal::unique_name(n, false);
    }

    /** Construct a reduction variable with the given name and
     * bounds. Must be a member of the given reduction domain. */
    RVar(Internal::ReductionDomain domain, int index) :
        _domain(domain), _index(index) {
    }

    /** The minimum value that this variable will take on */
    EXPORT Expr min() const;

    /** The number that this variable will take on. The maximum value
     * of this variable will be min() + extent() - 1 */
    EXPORT Expr extent() const;

    /** The reduction domain this is associated with. */
    EXPORT Internal::ReductionDomain domain() const {return _domain;}

    /** The name of this reduction variable */
    EXPORT const std::string &name() const;

    /** Reduction variables can be used as expressions. */
    EXPORT operator Expr() const;
};

/** A multi-dimensional domain over which to iterate. Used when
 * defining functions with update definitions.
 *
 * An reduction is a function with a two-part definition. It has an
 * initial value, which looks much like a pure function, and an update
 * definition, which may refer to some RDom. Evaluating such a
 * function first initializes it over the required domain (which is
 * inferred based on usage), and then runs update rule for all points
 * in the RDom. For example:
 *
 \code
 Func f;
 Var x;
 RDom r(0, 10);
 f(x) = x; // the initial value
 f(r) = f(r) * 2;
 Image<int> result = f.realize(10);
 \endcode
 *
 * This function creates a single-dimensional buffer of size 10, in
 * which element x contains the value x*2. Internally, first the
 * initialization rule fills in x at every site, and then the update
 * definition doubles every site.
 *
 * One use of reductions is to build a function recursively (pure
 * functions in halide cannot be recursive). For example, this
 * function fills in an array with the first 20 fibonacci numbers:
 *
 \code
 Func f;
 Var x;
 RDom r(2, 18);
 f(x) = 1;
 f(r) = f(r-1) + f(r-2);
 \endcode
 *
 * Another use of reductions is to perform scattering operations, as
 * unlike a pure function declaration, the left-hand-side of an update
 * definition may contain general expressions:
 *
 \code
 ImageParam input(UInt(8), 2);
 Func histogram;
 Var x;
 RDom r(input); // Iterate over all pixels in the input
 histogram(x) = 0;
 histogram(input(r.x, r.y)) = histogram(input(r.x, r.y)) + 1;
 \endcode
 *
 * An update definition may also be multi-dimensional. This example
 * computes a summed-area table by first summing horizontally and then
 * vertically:
 *
 \code
 ImageParam input(Float(32), 2);
 Func sum_x, sum_y;
 Var x, y;
 RDom r(input);
 sum_x(x, y)     = input(x, y);
 sum_x(r.x, r.y) = sum_x(r.x, r.y) + sum_x(r.x-1, r.y);
 sum_y(x, y)     = sum_x(x, y);
 sum_y(r.x, r.y) = sum_y(r.x, r.y) + sum_y(r.x, r.y-1);
 \endcode
 *
 * You can also mix pure dimensions with reduction variables. In the
 * previous example, note that there's no need for the y coordinate in
 * sum_x to be traversed serially. The sum within each row is entirely
 * independent. The rows could be computed in parallel, or in a
 * different order, without changing the meaning. Therefore, we can
 * instead write this definition as follows:
 *
 \code
 ImageParam input(Float(32), 2);
 Func sum_x, sum_y;
 Var x, y;
 RDom r(input);
 sum_x(x, y)   = input(x, y);
 sum_x(r.x, y) = sum_x(r.x, y) + sum_x(r.x-1, y);
 sum_y(x, y)   = sum_x(x, y);
 sum_y(x, r.y) = sum_y(x, r.y) + sum_y(x, r.y-1);
 \endcode
 *
 * This lets us schedule it more flexibly. You can now parallelize the
 * update step of sum_x over y by calling:
 \code
 sum_x.update().parallel(y).
 \endcode
 *
 * Note that calling sum_x.parallel(y) only parallelizes the
 * initialization step, and not the update step! Scheduling the update
 * step of a reduction must be done using the handle returned by
 * \ref Func::update(). This code parallelizes both the initialization
 * step and the update step:
 *
 \code
 sum_x.parallel(y);
 sum_x.update().parallel(y);
 \endcode
 *
 * When you mix reduction variables and pure dimensions, the reduction
 * domain is traversed outermost. That is, for each point in the
 * reduction domain, the inferred pure domain is traversed in its
 * entirety. For the above example, this means that sum_x walks down
 * the columns, and sum_y walks along the rows. This may not be
 * cache-coherent. You may try reordering these dimensions using the
 * schedule, but Halide will return an error if it decides that this
 * risks changing the meaning of your function. The solution lies in
 * clever scheduling. If we say:
 *
 \code
 sum_x.compute_at(sum_y, r.y);
 \endcode
 *
 * Then the sum in x is computed only as necessary for each scanline
 * of the sum in y. This not only results in sum_x walking along the
 * rows, it also improves the locality of the entire pipeline.
 */
class RDom {
    Internal::ReductionDomain dom;

    void init_vars(std::string name);

public:
    /** Construct an undefined reduction domain. */
    EXPORT RDom() {}

    /** Construct a one-dimensional reduction domain with the given name. If the name
     * is left blank, a unique one is auto-generated. */
    EXPORT RDom(Expr min, Expr extent, std::string name = "");

    /** Construct a two-dimensional reduction domain with the given name. If the name
     * is left blank, a unique one is auto-generated. */
    EXPORT RDom(Expr min0, Expr extent0, Expr min1, Expr extent1, std::string name = "");

    /** Construct a multi-dimensional reduction domain with the given
     * name. If the name is left blank, a unique one is
     * auto-generated. */
    // @{
    EXPORT RDom(Expr min0, Expr extent0, Expr min1, Expr extent1, Expr min2, Expr extent2, std::string name = "");
    EXPORT RDom(Expr min0, Expr extent0, Expr min1, Expr extent1, Expr min2, Expr extent2, Expr min3, Expr extent3,
                std::string name = "");
    EXPORT RDom(Expr min0, Expr extent0, Expr min1, Expr extent1, Expr min2, Expr extent2, Expr min3, Expr extent3,
                Expr min4, Expr extent4, std::string name = "");
    EXPORT RDom(Expr min0, Expr extent0, Expr min1, Expr extent1, Expr min2, Expr extent2, Expr min3, Expr extent3,
                Expr min4, Expr extent4, Expr min5, Expr extent5, std::string name = "");
    EXPORT RDom(Expr min0, Expr extent0, Expr min1, Expr extent1, Expr min2, Expr extent2, Expr min3, Expr extent3,
                Expr min4, Expr extent4, Expr min5, Expr extent5, Expr min6, Expr extent6, std::string name = "");
    EXPORT RDom(Expr min0, Expr extent0, Expr min1, Expr extent1, Expr min2, Expr extent2, Expr min3, Expr extent3,
                Expr min4, Expr extent4, Expr min5, Expr extent5, Expr min6, Expr extent6, Expr min7, Expr extent7,
                std::string name = "");
    // @}

    /** Construct a reduction domain that iterates over all points in
     * a given Buffer, Image, or ImageParam. Has the same
     * dimensionality as the argument. */
    // @{
    EXPORT RDom(Buffer);
    EXPORT RDom(ImageParam);
    // @}

    /** Construct a reduction domain that wraps an Internal ReductionDomain object. */
    EXPORT RDom(Internal::ReductionDomain d);

    /** Get at the internal reduction domain object that this wraps. */
    Internal::ReductionDomain domain() const {return dom;}

    /** Check if this reduction domain is non-NULL */
    bool defined() const {return dom.defined();}

    /** Compare two reduction domains for equality of reference */
    bool same_as(const RDom &other) const {return dom.same_as(other.dom);}

    /** Get the dimensionality of a reduction domain */
    EXPORT int dimensions() const;

    /** Get at one of the dimensions of the reduction domain */
    EXPORT RVar operator[](int) const;

    /** Single-dimensional reduction domains can be used as RVars directly. */
    EXPORT operator RVar() const;

    /** Single-dimensional reduction domains can be also be used as Exprs directly. */
    EXPORT operator Expr() const;

    /** Direct access to the first four dimensions of the reduction
     * domain. Some of these variables may be undefined if the
     * reduction domain has fewer than four dimensions. */
    // @{
    RVar x, y, z, w;
    // @}
};

/** Emit an RVar in a human-readable form */
std::ostream &operator<<(std::ostream &stream, RVar);

/** Emit an RDom in a human-readable form. */
std::ostream &operator<<(std::ostream &stream, RDom);
}

#endif
#ifndef HALIDE_IMAGE_H
#define HALIDE_IMAGE_H

/** \file
 * Defines Halide's Image data type
 */

#ifndef HALIDE_TUPLE_H
#define HALIDE_TUPLE_H

/** \file
 *
 * Defines Tuple - the front-end handle on small arrays of expressions.
 */

#include <vector>

namespace Halide {

class FuncRefVar;
class FuncRefExpr;

/** Create a small array of Exprs for defining and calling functions
 * with multiple outputs. */
class Tuple {
private:
    std::vector<Expr> exprs;
public:
    /** The number of elements in the tuple. */
    size_t size() const { return exprs.size(); }

    /** Get a reference to an element. */
    Expr &operator[](size_t x) {
        user_assert(x < exprs.size()) << "Tuple access out of bounds\n";
        return exprs[x];
    }

    /** Get a copy of an element. */
    Expr operator[](size_t x) const {
        user_assert(x < exprs.size()) << "Tuple access out of bounds\n";
        return exprs[x];
    }

    /** Construct a Tuple from some Exprs. */
    //@{
    Tuple(Expr a, Expr b) :
        exprs(Internal::vec<Expr>(a, b)) {
    }

    Tuple(Expr a, Expr b, Expr c) :
        exprs(Internal::vec<Expr>(a, b, c)) {
    }

    Tuple(Expr a, Expr b, Expr c, Expr d) :
        exprs(Internal::vec<Expr>(a, b, c, d)) {
    }

    Tuple(Expr a, Expr b, Expr c, Expr d, Expr e) :
        exprs(Internal::vec<Expr>(a, b, c, d, e)) {
    }

    Tuple(Expr a, Expr b, Expr c, Expr d, Expr e, Expr f) :
        exprs(Internal::vec<Expr>(a, b, c, d, e, f)) {
    }
    //@}

    /** Construct a Tuple from a vector of Exprs */
    explicit Tuple(const std::vector<Expr> &e) : exprs(e) {
        user_assert(e.size() > 0) << "Tuples must have at least one element\n";
    }

    /** Construct a Tuple from a function reference. */
    // @{
    EXPORT Tuple(const FuncRefVar &);
    EXPORT Tuple(const FuncRefExpr &);
    // @}

    /** Treat the tuple as a vector of Exprs */
    const std::vector<Expr> &as_vector() const {
        return exprs;
    }
};

/** Funcs with Tuple values return multiple buffers when you realize
 * them. Tuples are to Exprs as Realizations are to Buffers. */
class Realization {
private:
    std::vector<Buffer> buffers;
public:
    /** The number of buffers in the Realization. */
    size_t size() const { return buffers.size(); }

    /** Get a reference to one of the buffers. */
    Buffer &operator[](size_t x) {
        user_assert(x < buffers.size()) << "Realization access out of bounds\n";
        return buffers[x];
    }

    /** Get one of the buffers. */
    Buffer operator[](size_t x) const {
        user_assert(x < buffers.size()) << "Realization access out of bounds\n";
        return buffers[x];
    }

    /** Single-element realizations are implicitly castable to Buffers. */
    operator Buffer() const {
        user_assert(buffers.size() == 1) << "Can only cast single-element realizations to buffers or images\n";
        return buffers[0];
    }

    /** Construct a Realization from some Buffers. */
    //@{
    Realization(Buffer a, Buffer b) :
        buffers(Internal::vec<Buffer>(a, b)) {}

    Realization(Buffer a, Buffer b, Buffer c) :
        buffers(Internal::vec<Buffer>(a, b, c)) {}

    Realization(Buffer a, Buffer b, Buffer c, Buffer d) :
        buffers(Internal::vec<Buffer>(a, b, c, d)) {}

    Realization(Buffer a, Buffer b, Buffer c, Buffer d, Buffer e) :
        buffers(Internal::vec<Buffer>(a, b, c, d, e)) {}

    Realization(Buffer a, Buffer b, Buffer c, Buffer d, Buffer e, Buffer f) :
        buffers(Internal::vec<Buffer>(a, b, c, d, e, f)) {}
    //@}

    /** Construct a Realization from a vector of Buffers */
    explicit Realization(const std::vector<Buffer> &e) : buffers(e) {
        user_assert(e.size() > 0) << "Realizations must have at least one element\n";
    }

    /** Treat the Realization as a vector of Buffers */
    const std::vector<Buffer> &as_vector() const {
        return buffers;
    }
};

/** Equivalents of some standard operators for tuples. */
// @{
inline Tuple tuple_select(Tuple condition, const Tuple &true_value, const Tuple &false_value) {
    Tuple result(std::vector<Expr>(condition.size()));
    for (size_t i = 0; i < result.size(); i++) {
        result[i] = select(condition[i], true_value[i], false_value[i]);
    }
    return result;
}

inline Tuple tuple_select(Expr condition, const Tuple &true_value, const Tuple &false_value) {
    Tuple result(std::vector<Expr>(true_value.size()));
    for (size_t i = 0; i < result.size(); i++) {
        result[i] = select(condition, true_value[i], false_value[i]);
    }
    return result;
}
// @}

}

#endif

namespace Halide {

/** A base class for Images, which are typed accessors on
 * Buffers. This exists to make the implementations of certain methods
 * of Image private, so that they can safely throw errors without the
 * risk of being inlined (which in turns messes up reporting of line
 * numbers). */
class ImageBase {
protected:
    /** The underlying memory object */
    Buffer buffer;

    /** These fields are also stored in the buffer, but they're cached
     * here in the handle to make operator() fast. This is safe to do
     * because the buffer is never modified
     */
    // @{
    void *origin;
    int stride_0, stride_1, stride_2, stride_3, dims;
    // @}

    /** Prepare the buffer to be used as an image. Makes sure that the
     * cached strides are correct, and that the image data is on the
     * host. */
    void prepare_for_direct_pixel_access();

    bool add_implicit_args_if_placeholder(std::vector<Expr> &args,
                                          Expr last_arg,
                                          int total_args,
                                          bool placeholder_seen) const;
public:
    /** Construct an undefined image handle */
    ImageBase() : origin(NULL), stride_0(0), stride_1(0), stride_2(0), stride_3(0), dims(0) {}

    /** Allocate an image with the given dimensions. */
    EXPORT ImageBase(Type t, int x, int y = 0, int z = 0, int w = 0, const std::string &name = "");

    /** Wrap a buffer in an Image object, so that we can directly
     * access its pixels in a type-safe way. */
    EXPORT ImageBase(Type t, const Buffer &buf);

    /** Wrap a single-element realization in an Image object. */
    EXPORT ImageBase(Type t, const Realization &r);

    /** Wrap a buffer_t in an Image object, so that we can access its
     * pixels. */
    EXPORT ImageBase(Type t, const buffer_t *b, const std::string &name = "");

    /** Get the name of this image. */
    EXPORT const std::string &name();

    /** Manually copy-back data to the host, if it's on a device. This
     * is done for you if you construct an image from a buffer, but
     * you might need to call this if you realize a gpu kernel into an
     * existing image */
    EXPORT void copy_to_host();

    /** Mark the buffer as dirty-on-host.  is done for you if you
     * construct an image from a buffer, but you might need to call
     * this if you realize a gpu kernel into an existing image, or
     * modify the data via some other back-door. */
    EXPORT void set_host_dirty(bool dirty = true);

    /** Check if this image handle points to actual data */
    EXPORT bool defined() const;

    /** Get the dimensionality of the data. Typically two for grayscale images, and three for color images. */
    EXPORT int dimensions() const;

    /** Get the size of a dimension */
    EXPORT int extent(int dim) const;

    /** Get the min coordinate of a dimension. The top left of the
     * image represents this point in a function that was realized
     * into this image. */
    EXPORT int min(int dim) const;

    /** Set the min coordinates of a dimension. */
    EXPORT void set_min(int m0, int m1 = 0, int m2 = 0, int m3 = 0);

    /** Get the number of elements in the buffer between two adjacent
     * elements in the given dimension. For example, the stride in
     * dimension 0 is usually 1, and the stride in dimension 1 is
     * usually the extent of dimension 0. This is not necessarily true
     * though. */
    EXPORT int stride(int dim) const;

    /** Get the extent of dimension 0, which by convention we use as
     * the width of the image. Unlike extent(0), returns one if the
     * buffer is zero-dimensional. */
    EXPORT int width() const;

    /** Get the extent of dimension 1, which by convention we use as
     * the height of the image. Unlike extent(1), returns one if the
     * buffer has fewer than two dimensions. */
    EXPORT int height() const;

    /** Get the extent of dimension 2, which by convention we use as
     * the number of color channels (often 3). Unlike extent(2),
     * returns one if the buffer has fewer than three dimensions. */
    EXPORT int channels() const;

    /** Get the minimum coordinate in dimension 0, which by convention
     * is the coordinate of the left edge of the image. Returns zero
     * for zero-dimensional images. */
    EXPORT int left() const;

    /** Get the maximum coordinate in dimension 0, which by convention
     * is the coordinate of the right edge of the image. Returns zero
     * for zero-dimensional images. */
    EXPORT int right() const;

    /** Get the minimum coordinate in dimension 1, which by convention
     * is the top of the image. Returns zero for zero- or
     * one-dimensional images. */
    EXPORT int top() const;

    /** Get the maximum coordinate in dimension 1, which by convention
     * is the bottom of the image. Returns zero for zero- or
     * one-dimensional images. */
    EXPORT int bottom() const;

    /** Construct an expression which loads from this image. The
     * location is extended with enough implicit variables to match
     * the dimensionality of the image (see \ref Var::implicit) */
    // @{
    EXPORT Expr operator()() const;
    EXPORT Expr operator()(Expr x) const;
    EXPORT Expr operator()(Expr x, Expr y) const;
    EXPORT Expr operator()(Expr x, Expr y, Expr z) const;
    EXPORT Expr operator()(Expr x, Expr y, Expr z, Expr w) const;
    EXPORT Expr operator()(std::vector<Expr>) const;
    EXPORT Expr operator()(std::vector<Var>) const;
    // @}

    /** Get a pointer to the raw buffer_t that this image holds */
    EXPORT buffer_t *raw_buffer() const;
};

/** A reference-counted handle on a dense multidimensional array
 * containing scalar values of type T. Can be directly accessed and
 * modified. May have up to four dimensions. Color images are
 * represented as three-dimensional, with the third dimension being
 * the color channel. In general we store color images in
 * color-planes, as opposed to packed RGB, because this tends to
 * vectorize more cleanly. */
template<typename T>
class Image : public ImageBase {
public:
    /** Construct an undefined image handle */
    Image() : ImageBase() {}

    /** Allocate an image with the given dimensions. */
    // @{
    NO_INLINE Image(int x, int y = 0, int z = 0, int w = 0, const std::string &name = "") :
        ImageBase(type_of<T>(), x, y, z, w, name) {}

    NO_INLINE Image(int x, int y, int z, const std::string &name) :
        ImageBase(type_of<T>(), x, y, z, 0, name) {}

    NO_INLINE Image(int x, int y, const std::string &name) :
        ImageBase(type_of<T>(), x, y, 0, 0, name) {}

    NO_INLINE Image(int x, const std::string &name) :
        ImageBase(type_of<T>(), x, 0, 0, 0, name) {}
    // @}

    /** Wrap a buffer in an Image object, so that we can directly
     * access its pixels in a type-safe way. */
    NO_INLINE Image(const Buffer &buf) : ImageBase(type_of<T>(), buf) {}

    /** Wrap a single-element realization in an Image object. */
    NO_INLINE Image(const Realization &r) : ImageBase(type_of<T>(), r) {}

    /** Wrap a buffer_t in an Image object, so that we can access its
     * pixels. */
    NO_INLINE Image(const buffer_t *b, const std::string &name = "") :
        ImageBase(type_of<T>(), b, name) {}

    /** Get a pointer to the element at the min location. */
    NO_INLINE T *data() const {
        user_assert(defined()) << "data of undefined Image\n";
        return (T *)buffer.host_ptr();
    }

    using ImageBase::operator();

    /** Assuming this image is one-dimensional, get the value of the
     * element at position x */
    const T &operator()(int x) const {
        return ((T *)origin)[x*stride_0];
    }

    /** Assuming this image is two-dimensional, get the value of the
     * element at position (x, y) */
    const T &operator()(int x, int y) const {
        return ((T *)origin)[x*stride_0 + y*stride_1];
    }

    /** Assuming this image is three-dimensional, get the value of the
     * element at position (x, y, z) */
    const T &operator()(int x, int y, int z) const {
        return ((T *)origin)[x*stride_0 + y*stride_1 + z*stride_2];
    }

    /** Assuming this image is four-dimensional, get the value of the
     * element at position (x, y, z, w) */
    const T &operator()(int x, int y, int z, int w) const {
        return ((T *)origin)[x*stride_0 + y*stride_1 + z*stride_2 + w*stride_3];
    }

    /** Assuming this image is one-dimensional, get a reference to the
     * element at position x */
    T &operator()(int x) {
        return ((T *)origin)[x*stride_0];
    }

    /** Assuming this image is two-dimensional, get a reference to the
     * element at position (x, y) */
    T &operator()(int x, int y) {
        return ((T *)origin)[x*stride_0 + y*stride_1];
    }

    /** Assuming this image is three-dimensional, get a reference to the
     * element at position (x, y, z) */
    T &operator()(int x, int y, int z) {
        return ((T *)origin)[x*stride_0 + y*stride_1 + z*stride_2];
    }

    /** Assuming this image is four-dimensional, get a reference to the
     * element at position (x, y, z, w) */
    T &operator()(int x, int y, int z, int w) {
        return ((T *)origin)[x*stride_0 + y*stride_1 + z*stride_2 + w*stride_3];
    }

    /** Get a handle on the Buffer that this image holds */
    operator Buffer() const {
        return buffer;
    }

    /** Convert this image to an argument to a halide pipeline. */
    operator Argument() const {
        return Argument(buffer);
    }

    /** Convert this image to an argument to an extern stage. */
    operator ExternFuncArgument() const {
        return ExternFuncArgument(buffer);
    }

    /** Treating the image as an Expr is equivalent to call it with no
     * arguments. For example, you can say:
     *
     \code
     Image im(10, 10);
     Func f;
     f = im*2;
     \endcode
     *
     * This will define f as a two-dimensional function with value at
     * position (x, y) equal to twice the value of the image at the
     * same location.
     */
    operator Expr() const {
        return (*this)(_);
    }


};

}

#endif

namespace Halide {

enum GPUAPI {
    GPU_Default,
    GPU_CUDA,
    GPU_OpenCL,
    GPU_GLSL
};

/** A class that can represent Vars or RVars. Used for reorder calls
 * which can accept a mix of either. */
struct VarOrRVar {
    VarOrRVar(const std::string &n, bool r) : var(n), rvar(n), is_rvar(r) {}
    VarOrRVar(const Var &v) : var(v), is_rvar(false) {}
    VarOrRVar(const RVar &r) : rvar(r), is_rvar(true) {}
    VarOrRVar(const RDom &r) : rvar(RVar(r)), is_rvar(true) {}

    const std::string &name() const {
        if (is_rvar) return rvar.name();
        else return var.name();
    }

    const Var var;
    const RVar rvar;
    const bool is_rvar;
};

/** A single definition of a Func. May be a pure or update definition. */
class Stage {
    Internal::Schedule schedule;
    void set_dim_type(VarOrRVar var, Internal::For::ForType t);
    void split(const std::string &old, const std::string &outer, const std::string &inner, Expr factor, bool exact);
    std::string stage_name;
public:
    Stage(Internal::Schedule s, const std::string &n) :
        schedule(s), stage_name(n) {s.touched();}

    /** Return a string describing the current var list taking into
     * account all the splits, reorders, and tiles. */
    EXPORT std::string dump_argument_list() const;

    /** Return the name of this stage, e.g. "f.update(2)" */
    EXPORT const std::string &name() const;

    /** Scheduling calls that control how the domain of this stage is
     * traversed. See the documentation for Func for the meanings. */
    // @{

    EXPORT Stage &split(VarOrRVar old, VarOrRVar outer, VarOrRVar inner, Expr factor);
    EXPORT Stage &fuse(VarOrRVar inner, VarOrRVar outer, VarOrRVar fused);
    EXPORT Stage &serial(VarOrRVar var);
    EXPORT Stage &parallel(VarOrRVar var);
    EXPORT Stage &vectorize(VarOrRVar var);
    EXPORT Stage &unroll(VarOrRVar var);
    EXPORT Stage &parallel(VarOrRVar var, Expr task_size);
    EXPORT Stage &vectorize(VarOrRVar var, int factor);
    EXPORT Stage &unroll(VarOrRVar var, int factor);
    EXPORT Stage &tile(VarOrRVar x, VarOrRVar y,
                                VarOrRVar xo, VarOrRVar yo,
                                VarOrRVar xi, VarOrRVar yi, Expr
                                xfactor, Expr yfactor);
    EXPORT Stage &tile(VarOrRVar x, VarOrRVar y,
                                VarOrRVar xi, VarOrRVar yi,
                                Expr xfactor, Expr yfactor);
    EXPORT Stage &reorder(const std::vector<VarOrRVar> &vars);
    EXPORT Stage &reorder(VarOrRVar x, VarOrRVar y);
    EXPORT Stage &reorder(VarOrRVar x, VarOrRVar y, VarOrRVar z);
    EXPORT Stage &reorder(VarOrRVar x, VarOrRVar y, VarOrRVar z,
                                   VarOrRVar w);
    EXPORT Stage &reorder(VarOrRVar x, VarOrRVar y, VarOrRVar z,
                                   VarOrRVar w, VarOrRVar t);
    EXPORT Stage &reorder(VarOrRVar x, VarOrRVar y, VarOrRVar z,
                                   VarOrRVar w, VarOrRVar t1, VarOrRVar t2);
    EXPORT Stage &reorder(VarOrRVar x, VarOrRVar y, VarOrRVar z,
                                   VarOrRVar w, VarOrRVar t1, VarOrRVar t2,
                                   VarOrRVar t3);
    EXPORT Stage &reorder(VarOrRVar x, VarOrRVar y, VarOrRVar z,
                                   VarOrRVar w, VarOrRVar t1, VarOrRVar t2,
                                   VarOrRVar t3, VarOrRVar t4);
    EXPORT Stage &reorder(VarOrRVar x, VarOrRVar y, VarOrRVar z,
                                   VarOrRVar w, VarOrRVar t1, VarOrRVar t2,
                                   VarOrRVar t3, VarOrRVar t4, VarOrRVar t5);
    EXPORT Stage &reorder(VarOrRVar x, VarOrRVar y, VarOrRVar z,
                                   VarOrRVar w, VarOrRVar t1, VarOrRVar t2,
                                   VarOrRVar t3, VarOrRVar t4, VarOrRVar t5,
                                   VarOrRVar t6);
    EXPORT Stage &rename(VarOrRVar old_name, VarOrRVar new_name);
    EXPORT Stage specialize(Expr condition);

    EXPORT Stage &gpu_threads(VarOrRVar thread_x, GPUAPI gpu_api = GPU_Default);
    EXPORT Stage &gpu_threads(VarOrRVar thread_x, VarOrRVar thread_y, GPUAPI gpu_api = GPU_Default);
    EXPORT Stage &gpu_threads(VarOrRVar thread_x, VarOrRVar thread_y, VarOrRVar thread_z, GPUAPI gpu_api = GPU_Default);
    EXPORT Stage &gpu_single_thread(GPUAPI gpu_api = GPU_Default);

    EXPORT Stage &gpu_blocks(VarOrRVar block_x, GPUAPI gpu_api = GPU_Default);
    EXPORT Stage &gpu_blocks(VarOrRVar block_x, VarOrRVar block_y, GPUAPI gpu_api = GPU_Default);
    EXPORT Stage &gpu_blocks(VarOrRVar block_x, VarOrRVar block_y, VarOrRVar block_z, GPUAPI gpu_api = GPU_Default);

    EXPORT Stage &gpu(VarOrRVar block_x, VarOrRVar thread_x, GPUAPI gpu_api = GPU_Default);
    EXPORT Stage &gpu(VarOrRVar block_x, VarOrRVar block_y,
                               VarOrRVar thread_x, VarOrRVar thread_y,
                               GPUAPI gpu_api = GPU_Default);
    EXPORT Stage &gpu(VarOrRVar block_x, VarOrRVar block_y, VarOrRVar block_z,
                               VarOrRVar thread_x, VarOrRVar thread_y, VarOrRVar thread_z,
                               GPUAPI gpu_api = GPU_Default);
    EXPORT Stage &gpu_tile(VarOrRVar x, Expr x_size, GPUAPI gpu_api = GPU_Default);
    EXPORT Stage &gpu_tile(VarOrRVar x, VarOrRVar y, Expr x_size, Expr y_size,
                                    GPUAPI gpu_api = GPU_Default);
    EXPORT Stage &gpu_tile(VarOrRVar x, VarOrRVar y, VarOrRVar z,
                                    Expr x_size, Expr y_size, Expr z_size, GPUAPI gpu_api = GPU_Default);

    EXPORT Stage &allow_race_conditions();
    // @}

    // These calls are for legacy compatibility only.
    EXPORT Stage &cuda_threads(VarOrRVar thread_x) {
        return gpu_threads(thread_x);
    }
    EXPORT Stage &cuda_threads(VarOrRVar thread_x, VarOrRVar thread_y) {
        return gpu_threads(thread_x, thread_y);
    }
    EXPORT Stage &cuda_threads(VarOrRVar thread_x, VarOrRVar thread_y, VarOrRVar thread_z) {
        return gpu_threads(thread_x, thread_y, thread_z);
    }

    EXPORT Stage &cuda_blocks(VarOrRVar block_x) {
        return gpu_blocks(block_x);
    }
    EXPORT Stage &cuda_blocks(VarOrRVar block_x, VarOrRVar block_y) {
        return gpu_blocks(block_x, block_y);
    }
    EXPORT Stage &cuda_blocks(VarOrRVar block_x, VarOrRVar block_y, VarOrRVar block_z) {
        return gpu_blocks(block_x, block_y, block_z);
    }

    EXPORT Stage &cuda(VarOrRVar block_x, VarOrRVar thread_x) {
        return gpu(block_x, thread_x);
    }
    EXPORT Stage &cuda(VarOrRVar block_x, VarOrRVar block_y,
                                VarOrRVar thread_x, VarOrRVar thread_y) {
        return gpu(block_x, thread_x, block_y, thread_y);
    }
    EXPORT Stage &cuda(VarOrRVar block_x, VarOrRVar block_y, VarOrRVar block_z,
                                VarOrRVar thread_x, VarOrRVar thread_y, VarOrRVar thread_z) {
        return gpu(block_x, thread_x, block_y, thread_y, block_z, thread_z);
    }
    EXPORT Stage &cuda_tile(VarOrRVar x, int x_size) {
        return gpu_tile(x, x_size);
    }
    EXPORT Stage &cuda_tile(VarOrRVar x, VarOrRVar y, int x_size, int y_size) {
        return gpu_tile(x, y, x_size, y_size);
    }
    EXPORT Stage &cuda_tile(VarOrRVar x, VarOrRVar y, VarOrRVar z,
                                     int x_size, int y_size, int z_size) {
        return gpu_tile(x, y, z, x_size, y_size, z_size);
    }
};

// For backwards compatibility, keep the ScheduleHandle name.
typedef Stage ScheduleHandle;

/** A fragment of front-end syntax of the form f(x, y, z), where x,
 * y, z are Vars. It could be the left-hand side of a function
 * definition, or it could be a call to a function. We don't know
 * until we see how this object gets used.
 */
class FuncRefExpr;

class FuncRefVar {
    Internal::Function func;
    int implicit_placeholder_pos;
    std::vector<std::string> args;
    std::vector<std::string> args_with_implicit_vars(const std::vector<Expr> &e) const;
public:
    FuncRefVar(Internal::Function, const std::vector<Var> &, int placeholder_pos = -1);

    /**  Use this as the left-hand-side of a definition. */
    EXPORT Stage operator=(Expr);

    /** Use this as the left-hand-side of a definition for a Func with
     * multiple outputs. */
    EXPORT Stage operator=(const Tuple &);

    /** Define this function as a sum reduction over the given
     * expression. The expression should refer to some RDom to sum
     * over. If the function does not already have a pure definition,
     * this sets it to zero.
     */
    EXPORT Stage operator+=(Expr);

    /** Define this function as a sum reduction over the negative of
     * the given expression. The expression should refer to some RDom
     * to sum over. If the function does not already have a pure
     * definition, this sets it to zero.
     */
    EXPORT Stage operator-=(Expr);

    /** Define this function as a product reduction. The expression
     * should refer to some RDom to take the product over. If the
     * function does not already have a pure definition, this sets it
     * to 1.
     */
    EXPORT Stage operator*=(Expr);

    /** Define this function as the product reduction over the inverse
     * of the expression. The expression should refer to some RDom to
     * take the product over. If the function does not already have a
     * pure definition, this sets it to 1.
     */
    EXPORT Stage operator/=(Expr);

    /** Override the usual assignment operator, so that
     * f(x, y) = g(x, y) defines f.
     */
    // @{
    EXPORT Stage operator=(const FuncRefVar &e);
    EXPORT Stage operator=(const FuncRefExpr &e);
    // @}

    /** Use this FuncRefVar as a call to the function, and not as the
     * left-hand-side of a definition. Only works for single-output
     * funcs.
     */
    EXPORT operator Expr() const;

    /** When a FuncRefVar refers to a function that provides multiple
     * outputs, you can access each output as an Expr using
     * operator[] */
    EXPORT Expr operator[](int) const;

    /** How many outputs does the function this refers to produce. */
    EXPORT size_t size() const;

    /** What function is this calling? */
    EXPORT Internal::Function function() const {return func;}
};

/** A fragment of front-end syntax of the form f(x, y, z), where x, y,
 * z are Exprs. If could be the left hand side of an update
 * definition, or it could be a call to a function. We don't know
 * until we see how this object gets used.
 */
class FuncRefExpr {
    Internal::Function func;
    int implicit_placeholder_pos;
    std::vector<Expr> args;
    std::vector<Expr> args_with_implicit_vars(const std::vector<Expr> &e) const;
public:
    FuncRefExpr(Internal::Function, const std::vector<Expr> &,
                int placeholder_pos = -1);
    FuncRefExpr(Internal::Function, const std::vector<std::string> &,
                int placeholder_pos = -1);

    /** Use this as the left-hand-side of an update definition (see
     * \ref RDom). The function must already have a pure definition.
     */
    EXPORT Stage operator=(Expr);

    /** Use this as the left-hand-side of an update definition for a
     * Func with multiple outputs. */
    EXPORT Stage operator=(const Tuple &);

    /** Define this function as a sum reduction over the negative of
     * the given expression. The expression should refer to some RDom
     * to sum over. If the function does not already have a pure
     * definition, this sets it to zero.
     */
    EXPORT Stage operator+=(Expr);

    /** Define this function as a sum reduction over the given
     * expression. The expression should refer to some RDom to sum
     * over. If the function does not already have a pure definition,
     * this sets it to zero.
     */
    EXPORT Stage operator-=(Expr);

    /** Define this function as a product reduction. The expression
     * should refer to some RDom to take the product over. If the
     * function does not already have a pure definition, this sets it
     * to 1.
     */
    EXPORT Stage operator*=(Expr);

    /** Define this function as the product reduction over the inverse
     * of the expression. The expression should refer to some RDom to
     * take the product over. If the function does not already have a
     * pure definition, this sets it to 1.
     */
    EXPORT Stage operator/=(Expr);

    /* Override the usual assignment operator, so that
     * f(x, y) = g(x, y) defines f.
     */
    // @{
    EXPORT Stage operator=(const FuncRefVar &);
    EXPORT Stage operator=(const FuncRefExpr &);
    // @}

    /** Use this as a call to the function, and not the left-hand-side
     * of a definition. Only works for single-output Funcs. */
    EXPORT operator Expr() const;

    /** When a FuncRefExpr refers to a function that provides multiple
     * outputs, you can access each output as an Expr using
     * operator[].
     */
    EXPORT Expr operator[](int) const;

    /** How many outputs does the function this refers to produce. */
    EXPORT size_t size() const;

    /** What function is this calling? */
    EXPORT Internal::Function function() const {return func;}
};

/**
 * Used to determine if the output printed to file should be as a normal string
 * or as an HTML file which can be opened in a browerser and manipulated via JS and CSS.*/
enum StmtOutputFormat {
     Text,
     HTML
};

/** A halide function. This class represents one stage in a Halide
 * pipeline, and is the unit by which we schedule things. By default
 * they are aggressively inlined, so you are encouraged to make lots
 * of little functions, rather than storing things in Exprs. */
class Func {

    /** A handle on the internal halide function that this
     * represents */
    Internal::Function func;

    /** When you make a reference to this function with fewer
     * arguments than it has dimensions, the argument list is bulked
     * up with 'implicit' vars with canonical names. This lets you
     * pass around partially applied Halide functions. */
    // @{
    int add_implicit_vars(std::vector<Var> &) const;
    int add_implicit_vars(std::vector<Expr> &) const;
    // @}

    /** The lowered imperative form of this function and the target
     * this was lowered for. Cached here so that recompilation doesn't
     * necessarily require re-lowering */
    // @{
    Internal::Stmt lowered;
    Target lowered_target;
    // @}

    /** Lower the func if it hasn't been already. */
    void lower(const Target &t);

    /** A JIT-compiled version of this function that we save so that
     * we don't have to rejit every time we want to evaluated it. */
    Internal::JITCompiledModule compiled_module;

    /** Invalidate the cached lowered stmt and compiled module. */
    void invalidate_cache();

    /** The current error handler used for realizing this
     * function. May be NULL. Only relevant when jitting. */
    void (*error_handler)(void *user_context, const char *);

    /** The current custom allocator used for realizing this
     * function. May be NULL. Only relevant when jitting. */
    // @{
    void *(*custom_malloc)(void *user_context, size_t);
    void (*custom_free)(void *user_context, void *ptr);
    // @}

    /** The current custom parallel task launcher and handler for
     * realizing this function. May be NULL. */
    // @{
    int (*custom_do_par_for)(void *user_context,
                             int (*)(void *, int, uint8_t *),
                             int, int, uint8_t *);
    int (*custom_do_task)(void *user_context, int (*)(void *, int, uint8_t *),
                          int, uint8_t *);
    // @}

    /** The current custom tracing function. May be NULL. */
    // @{
    int32_t (*custom_trace)(void *, const halide_trace_event *);

    // @}

    /** The current print function used for realizing this
     * function. May be NULL. Only relevant when jitting. */
    void (*custom_print)(void *user_context, const char *);

    uint64_t cache_size;

    /** The random seed to use for realizations of this function. */
    uint32_t random_seed;

    /** Pointers to current values of the automatically inferred
     * arguments (buffers and scalars) used to realize this
     * function. Only relevant when jitting. We can hold these things
     * with raw pointers instead of reference-counted handles, because
     * func indirectly holds onto them with reference-counted handles
     * via its value Expr. */
    std::vector<const void *> arg_values;

    /** Some of the arg_values need to be rebound on every call if the
     * image params change. The pointers for the scalar params will
     * still be valid though. */
    std::vector<std::pair<int, Internal::Parameter> > image_param_args;

    /** A context to use for JIT-realizations of this Func. */
    Param<void *> user_context;

    // Some infrastructure that helps Funcs catch and handle runtime errors in JIT-compiled code.
    bool prepare_to_catch_runtime_errors(void *buf);

public:

    EXPORT static void test();

    /** Declare a new undefined function with the given name */
    EXPORT explicit Func(const std::string &name);

    /** Declare a new undefined function with an
     * automatically-generated unique name */
    EXPORT Func();

    /** Declare a new function with an automatically-generated unique
     * name, and define it to return the given expression (which may
     * not contain free variables). */
    EXPORT explicit Func(Expr e);

    /** Construct a new Func to wrap an existing, already-define
     * Function object. */
    EXPORT explicit Func(Internal::Function f);

    /** Evaluate this function over some rectangular domain and return
     * the resulting buffer or buffers. Performs compilation if the
     * Func has not previously been realized and jit_compile has not
     * been called. The returned Buffer should probably be instantly
     * wrapped in an Image class of the appropriate type. That is, do
     * this:
     *
     \code
     f(x) = sin(x);
     Image<float> im = f.realize(...);
     \endcode
     *
     * not this:
     *
     \code
     f(x) = sin(x)
     Buffer im = f.realize(...)
     \endcode
     *
     * If your Func has multiple values, because you defined it using
     * a Tuple, then casting the result of a realize call to a buffer
     * or image will produce a run-time error. Instead you should do the
     * following:
     *
     \code
     f(x) = Tuple(x, sin(x));
     Realization r = f.realize(...);
     Image<int> im0 = r[0];
     Image<float> im1 = r[1];
     \endcode
     *
     */
    // @{
    EXPORT Realization realize(std::vector<int32_t> sizes, const Target &target = get_jit_target_from_environment());
    EXPORT Realization realize(int x_size, int y_size, int z_size, int w_size,
                               const Target &target = get_jit_target_from_environment());
    EXPORT Realization realize(int x_size, int y_size, int z_size,
                               const Target &target = get_jit_target_from_environment());
    EXPORT Realization realize(int x_size, int y_size,
                               const Target &target = get_jit_target_from_environment());
    EXPORT Realization realize(int x_size = 0,
                               const Target &target = get_jit_target_from_environment());
    // @}

    /** Evaluate this function into an existing allocated buffer or
     * buffers. If the buffer is also one of the arguments to the
     * function, strange things may happen, as the pipeline isn't
     * necessarily safe to run in-place. If you pass multiple buffers,
     * they must have matching sizes. */
    // @{
    EXPORT void realize(Realization dst, const Target &target = get_jit_target_from_environment());
    EXPORT void realize(Buffer dst, const Target &target = get_jit_target_from_environment());

    template<typename T>
    void realize(Image<T> dst, const Target &target = get_jit_target_from_environment()) {
        // Images are expected to exist on-host.
        realize(Buffer(dst), target);
        dst.copy_to_host();
    }
    // @}

    /** For a given size of output, or a given output buffer,
     * determine the bounds required of all unbound ImageParams
     * referenced. Communicates the result by allocating new buffers
     * of the appropriate size and binding them to the unbound
     * ImageParams. */
    // @{
    EXPORT void infer_input_bounds(int x_size = 0, int y_size = 0, int z_size = 0, int w_size = 0);
    EXPORT void infer_input_bounds(Realization dst);
    EXPORT void infer_input_bounds(Buffer dst);
    // @}

    /** Statically compile this function to llvm bitcode, with the
     * given filename (which should probably end in .bc), type
     * signature, and C function name (which defaults to the same name
     * as this halide function */
    //@{
    EXPORT void compile_to_bitcode(const std::string &filename, std::vector<Argument>, const std::string &fn_name,
                                   const Target &target = get_target_from_environment());
    EXPORT void compile_to_bitcode(const std::string &filename, std::vector<Argument>,
                                   const Target &target = get_target_from_environment());
    // @}

    /** Statically compile this function to an object file, with the
     * given filename (which should probably end in .o or .obj), type
     * signature, and C function name (which defaults to the same name
     * as this halide function. You probably don't want to use this
     * directly; call compile_to_file instead. */
    //@{
    EXPORT void compile_to_object(const std::string &filename, std::vector<Argument>, const std::string &fn_name,
                                  const Target &target = get_target_from_environment());
    EXPORT void compile_to_object(const std::string &filename, std::vector<Argument>,
                                  const Target &target = get_target_from_environment());
    // @}

    /** Emit a header file with the given filename for this
     * function. The header will define a function with the type
     * signature given by the second argument, and a name given by the
     * third. The name defaults to the same name as this halide
     * function. You don't actually have to have defined this function
     * yet to call this. You probably don't want to use this directly;
     * call compile_to_file instead. */
    EXPORT void compile_to_header(const std::string &filename, std::vector<Argument>, const std::string &fn_name = "");

    /** Statically compile this function to text assembly equivalent
     * to the object file generated by compile_to_object. This is
     * useful for checking what Halide is producing without having to
     * disassemble anything, or if you need to feed the assembly into
     * some custom toolchain to produce an object file (e.g. iOS) */
    //@{
    EXPORT void compile_to_assembly(const std::string &filename, std::vector<Argument>, const std::string &fn_name,
                                    const Target &target = get_target_from_environment());
    EXPORT void compile_to_assembly(const std::string &filename, std::vector<Argument>,
                                    const Target &target = get_target_from_environment());
    // @}
    /** Statically compile this function to C source code. This is
     * useful for providing fallback code paths that will compile on
     * many platforms. Vectorization will fail, and parallelization
     * will produce serial code. */
    EXPORT void compile_to_c(const std::string &filename,
                             std::vector<Argument>,
                             const std::string &fn_name = "",
                             const Target &target = get_target_from_environment());

    /** Write out an internal representation of lowered code. Useful
     * for analyzing and debugging scheduling. Can emit html or plain
     * text. */
    EXPORT void compile_to_lowered_stmt(const std::string &filename,
                                        StmtOutputFormat fmt = Text,
                                        const Target &target = get_target_from_environment());

    /** Write out an internal representation of lowered code as above
     * but simplified using the provided realization bounds and other
     * concrete parameter values. Can emit html or plain text. */
    //@{
    EXPORT void compile_to_simplified_lowered_stmt(const std::string &filename,
                                                   Realization dst,
                                                   const std::map<std::string, Expr> &additional_replacements,
                                                   StmtOutputFormat fmt = Text,
                                                   const Target &t = get_target_from_environment());

    EXPORT void compile_to_simplified_lowered_stmt(const std::string &filename,
                                                   Realization dst,
                                                   StmtOutputFormat fmt = Text,
                                                   const Target &t = get_target_from_environment());

    EXPORT void compile_to_simplified_lowered_stmt(const std::string &filename,
                                                   Buffer dst,
                                                   const std::map<std::string, Expr> &additional_replacements,
                                                   StmtOutputFormat fmt = Text,
                                                   const Target &target = get_target_from_environment());

    EXPORT void compile_to_simplified_lowered_stmt(const std::string &filename,
                                                   Buffer dst,
                                                   StmtOutputFormat fmt = Text,
                                                   const Target &target = get_target_from_environment());

    EXPORT void compile_to_simplified_lowered_stmt(const std::string &filename,
                                                   int x_size, int y_size, int z_size, int w_size,
                                                   const std::map<std::string, Expr> &additional_replacements,
                                                   StmtOutputFormat fmt = Text,
                                                   const Target &t = get_target_from_environment());

    EXPORT void compile_to_simplified_lowered_stmt(const std::string &filename,
                                                   int x_size, int y_size, int z_size, int w_size,
                                                   StmtOutputFormat fmt = Text,
                                                   const Target &t = get_target_from_environment());

    EXPORT void compile_to_simplified_lowered_stmt(const std::string &filename,
                                                   int x_size, int y_size, int z_size,
                                                   const std::map<std::string, Expr> &additional_replacements,
                                                   StmtOutputFormat fmt = Text,
                                                   const Target &t = get_target_from_environment());

    EXPORT void compile_to_simplified_lowered_stmt(const std::string &filename,
                                                   int x_size, int y_size, int z_size,
                                                   StmtOutputFormat fmt = Text,
                                                   const Target &t = get_target_from_environment());

    EXPORT void compile_to_simplified_lowered_stmt(const std::string &filename,
                                                   int x_size, int y_size,
                                                   const std::map<std::string, Expr> &additional_replacements,
                                                   StmtOutputFormat fmt = Text,
                                                   const Target &t = get_target_from_environment());

    EXPORT void compile_to_simplified_lowered_stmt(const std::string &filename,
                                                   int x_size, int y_size,
                                                   StmtOutputFormat fmt = Text,
                                                   const Target &t = get_target_from_environment());

    EXPORT void compile_to_simplified_lowered_stmt(const std::string &filename,
                                                   int x_size,
                                                   const std::map<std::string, Expr> &additional_replacements,
                                                   StmtOutputFormat fmt = Text,
                                                   const Target &t = get_target_from_environment());

    EXPORT void compile_to_simplified_lowered_stmt(const std::string &filename,
                                                   int x_size,
                                                   StmtOutputFormat fmt = Text,
                                                   const Target &t = get_target_from_environment());

    // @}

    /** Compile to object file and header pair, with the given
     * arguments. Also names the C function to match the first
     * argument.
     */
    //@{
    EXPORT void compile_to_file(const std::string &filename_prefix, std::vector<Argument> args,
                                const Target &target = get_target_from_environment());
    EXPORT void compile_to_file(const std::string &filename_prefix,
                                const Target &target = get_target_from_environment());
    EXPORT void compile_to_file(const std::string &filename_prefix, Argument a,
                                const Target &target = get_target_from_environment());
    EXPORT void compile_to_file(const std::string &filename_prefix, Argument a, Argument b,
                                const Target &target = get_target_from_environment());
    EXPORT void compile_to_file(const std::string &filename_prefix, Argument a, Argument b, Argument c,
                                const Target &target = get_target_from_environment());
    EXPORT void compile_to_file(const std::string &filename_prefix, Argument a, Argument b, Argument c, Argument d,
                                const Target &target = get_target_from_environment());
    EXPORT void compile_to_file(const std::string &filename_prefix, Argument a, Argument b, Argument c, Argument d, Argument e,
                                const Target &target = get_target_from_environment());
    // @}

    /** Eagerly jit compile the function to machine code. This
     * normally happens on the first call to realize. If you're
     * running your halide pipeline inside time-sensitive code and
     * wish to avoid including the time taken to compile a pipeline,
     * then you can call this ahead of time. Returns the raw function
     * pointer to the compiled pipeline. Default is to use the Target
     * returned from Halide::get_jit_target_from_environment()
     */
     EXPORT void *compile_jit(const Target &target = get_jit_target_from_environment());

    /** Set the error handler function that be called in the case of
     * runtime errors during halide pipelines. If you are compiling
     * statically, you can also just define your own function with
     * signature
     \code
     extern "C" void halide_error(void *user_context, const char *);
     \endcode
     * This will clobber Halide's version.
     */
    EXPORT void set_error_handler(void (*handler)(void *, const char *));

    /** Set a custom malloc and free for halide to use. Malloc should
     * return 32-byte aligned chunks of memory, and it should be safe
     * for Halide to read slightly out of bounds (up to 8 bytes before
     * the start or beyond the end). If compiling statically, routines
     * with appropriate signatures can be provided directly
    \code
     extern "C" void *halide_malloc(void *, size_t)
     extern "C" void halide_free(void *, void *)
     \endcode
     * These will clobber Halide's versions. See \file HalideRuntime.h
     * for declarations.
     */
    EXPORT void set_custom_allocator(void *(*malloc)(void *, size_t),
                                     void (*free)(void *, void *));

    /** Set a custom task handler to be called by the parallel for
     * loop. It is useful to set this if you want to do some
     * additional bookkeeping at the granularity of parallel
     * tasks. The default implementation does this:
     \code
     extern "C" int halide_do_task(void *user_context,
                                   int (*f)(void *, int, uint8_t *),
                                   int idx, uint8_t *state) {
         return f(user_context, idx, state);
     }
     \endcode
     * If you are statically compiling, you can also just define your
     * own version of the above function, and it will clobber Halide's
     * version.
     *
     * If you're trying to use a custom parallel runtime, you probably
     * don't want to call this. See instead \ref Func::set_custom_do_par_for .
    */
    EXPORT void set_custom_do_task(
        int (*custom_do_task)(void *, int (*)(void *, int, uint8_t *),
                              int, uint8_t *));

    /** Set a custom parallel for loop launcher. Useful if your app
     * already manages a thread pool. The default implementation is
     * equivalent to this:
     \code
     extern "C" int halide_do_par_for(void *user_context,
                                      int (*f)(void *, int, uint8_t *),
                                      int min, int extent, uint8_t *state) {
         int exit_status = 0;
         parallel for (int idx = min; idx < min+extent; idx++) {
             int job_status = halide_do_task(user_context, f, idx, state);
             if (job_status) exit_status = job_status;
         }
         return exit_status;
     }
     \endcode
     *
     * However, notwithstanding the above example code, if one task
     * fails, we may skip over other tasks, and if two tasks return
     * different error codes, we may select one arbitrarily to return.
     *
     * If you are statically compiling, you can also just define your
     * own version of the above function, and it will clobber Halide's
     * version.
     */
    EXPORT void set_custom_do_par_for(
        int (*custom_do_par_for)(void *, int (*)(void *, int, uint8_t *), int,
                                 int, uint8_t *));

    /** Set custom routines to call when tracing is enabled. Call this
     * on the output Func of your pipeline. This then sets custom
     * routines for the entire pipeline, not just calls to this
     * Func.
     *
     * If you are statically compiling, you can also just define your
     * own versions of the tracing functions (see HalideRuntime.h),
     * and they will clobber Halide's versions. */
    EXPORT void set_custom_trace(Internal::JITCompiledModule::TraceFn);

    /** Set the function called to print messages from the runtime.
     * If you are compiling statically, you can also just define your
     * own function with signature
     \code
     extern "C" void halide_print(void *user_context, const char *);
     \endcode
     * This will clobber Halide's version.
     */
    EXPORT void set_custom_print(void (*handler)(void *, const char *));

    /** Set the maximum number of bytes used by memoization caching.
     * If you are compiling statically, you should include HalideRuntime.h
     * and call halide_memoization_cache_set_size() instead.
     */
    EXPORT void memoization_cache_set_size(uint64_t size);

    /** When this function is compiled, include code that dumps its
     * values to a file after it is realized, for the purpose of
     * debugging.
     *
     * If filename ends in ".tif" or ".tiff" (case insensitive) the file
     * is in TIFF format and can be read by standard tools. Oherwise, the
     * file format is as follows:
     *
     * All data is in the byte-order of the target platform.  First, a
     * 20 byte-header containing four 32-bit ints, giving the extents
     * of the first four dimensions.  Dimensions beyond four are
     * folded into the fourth.  Then, a fifth 32-bit int giving the
     * data type of the function. The typecodes are given by: float =
     * 0, double = 1, uint8_t = 2, int8_t = 3, uint16_t = 4, int16_t =
     * 5, uint32_t = 6, int32_t = 7, uint64_t = 8, int64_t = 9. The
     * data follows the header, as a densely packed array of the given
     * size and the given type. If given the extension .tmp, this file
     * format can be natively read by the program ImageStack. */
    EXPORT void debug_to_file(const std::string &filename);

    /** The name of this function, either given during construction,
     * or automatically generated. */
    EXPORT const std::string &name() const;

    /** Get the pure arguments. */
    EXPORT std::vector<Var> args() const;

    /** The right-hand-side value of the pure definition of this
     * function. Causes an error if there's no pure definition, or if
     * the function is defined to return multiple values. */
    EXPORT Expr value() const;

    /** The values returned by this function. An error if the function
     * has not been been defined. Returns a Tuple with one element for
     * functions defined to return a single value. */
    EXPORT Tuple values() const;

    /** Does this function have at least a pure definition. */
    EXPORT bool defined() const;

    /** Get the left-hand-side of the update definition. An empty
     * vector if there's no update definition. If there are
     * multiple update definitions for this function, use the
     * argument to select which one you want. */
    EXPORT const std::vector<Expr> &update_args(int idx = 0) const;

    /** Get the right-hand-side of an update definition. An error if
     * there's no update definition. If there are multiple
     * update definitions for this function, use the argument to
     * select which one you want. */
    EXPORT Expr update_value(int idx = 0) const;

    /** Get the right-hand-side of an update definition for
     * functions that returns multiple values. An error if there's no
     * update definition. Returns a Tuple with one element for
     * functions that return a single value. */
    EXPORT Tuple update_values(int idx = 0) const;

    /** Get the reduction domain for an update definition, if there is
     * one. */
    EXPORT RDom reduction_domain(int idx = 0) const;

    /** Does this function have at least one update definition? */
    EXPORT bool has_update_definition() const;

    /** How many update definitions does this function have? */
    EXPORT int num_update_definitions() const;

    /** Is this function an external stage? That is, was it defined
     * using define_extern? */
    EXPORT bool is_extern() const;

    /** Add an extern definition for this Func. This lets you define a
     * Func that represents an external pipeline stage. You can, for
     * example, use it to wrap a call to an extern library such as
     * fftw. */
    // @{
    EXPORT void define_extern(const std::string &function_name,
                              const std::vector<ExternFuncArgument> &params,
                              Type t,
                              int dimensionality) {
        define_extern(function_name, params, Internal::vec<Type>(t), dimensionality);
    }

    EXPORT void define_extern(const std::string &function_name,
                              const std::vector<ExternFuncArgument> &params,
                              const std::vector<Type> &types,
                              int dimensionality);
    // @}

    /** Get the types of the outputs of this Func. */
    EXPORT const std::vector<Type> &output_types() const;

    /** Get the number of outputs of this Func. Corresponds to the
     * size of the Tuple this Func was defined to return. */
    EXPORT int outputs() const;

    /** Get the name of the extern function called for an extern
     * definition. */
    EXPORT const std::string &extern_function_name() const;

    /** The dimensionality (number of arguments) of this
     * function. Zero if the function is not yet defined. */
    EXPORT int dimensions() const;

    /** Construct either the left-hand-side of a definition, or a call
     * to a functions that happens to only contain vars as
     * arguments. If the function has already been defined, and fewer
     * arguments are given than the function has dimensions, then
     * enough implicit vars are added to the end of the argument list
     * to make up the difference (see \ref Var::implicit) */
    // @{
    EXPORT FuncRefVar operator()() const;
    EXPORT FuncRefVar operator()(Var x) const;
    EXPORT FuncRefVar operator()(Var x, Var y) const;
    EXPORT FuncRefVar operator()(Var x, Var y, Var z) const;
    EXPORT FuncRefVar operator()(Var x, Var y, Var z, Var w) const;
    EXPORT FuncRefVar operator()(Var x, Var y, Var z, Var w, Var u) const;
    EXPORT FuncRefVar operator()(Var x, Var y, Var z, Var w, Var u, Var v) const;
    EXPORT FuncRefVar operator()(std::vector<Var>) const;
    // @}

    /** Either calls to the function, or the left-hand-side of a
     * update definition (see \ref RDom). If the function has
     * already been defined, and fewer arguments are given than the
     * function has dimensions, then enough implicit vars are added to
     * the end of the argument list to make up the difference. (see
     * \ref Var::implicit)*/
    // @{
    EXPORT FuncRefExpr operator()(Expr x) const;
    EXPORT FuncRefExpr operator()(Expr x, Expr y) const;
    EXPORT FuncRefExpr operator()(Expr x, Expr y, Expr z) const;
    EXPORT FuncRefExpr operator()(Expr x, Expr y, Expr z, Expr w) const;
    EXPORT FuncRefExpr operator()(Expr x, Expr y, Expr z, Expr w, Expr u) const;
    EXPORT FuncRefExpr operator()(Expr x, Expr y, Expr z, Expr w, Expr u, Expr v) const;
    EXPORT FuncRefExpr operator()(std::vector<Expr>) const;
    // @}

    /** Split a dimension into inner and outer subdimensions with the
     * given names, where the inner dimension iterates from 0 to
     * factor-1. The inner and outer subdimensions can then be dealt
     * with using the other scheduling calls. It's ok to reuse the old
     * variable name as either the inner or outer variable. */
    EXPORT Func &split(VarOrRVar old, VarOrRVar outer, VarOrRVar inner, Expr factor);

    /** Join two dimensions into a single fused dimenion. The fused
     * dimension covers the product of the extents of the inner and
     * outer dimensions given. */
    EXPORT Func &fuse(VarOrRVar inner, VarOrRVar outer, VarOrRVar fused);

    /** Mark a dimension to be traversed serially. This is the default. */
    EXPORT Func &serial(VarOrRVar var);

    /** Mark a dimension to be traversed in parallel */
    EXPORT Func &parallel(VarOrRVar var);

    /** Split a dimension by the given task_size, and the parallelize the
     * outer dimension. This creates parallel tasks that have size
     * task_size. After this call, var refers to the outer dimension of
     * the split. The inner dimension has a new anonymous name. If you
     * wish to mutate it, or schedule with respect to it, do the split
     * manually. */
    EXPORT Func &parallel(VarOrRVar var, Expr task_size);

    /** Mark a dimension to be computed all-at-once as a single
     * vector. The dimension should have constant extent -
     * e.g. because it is the inner dimension following a split by a
     * constant factor. For most uses of vectorize you want the two
     * argument form. The variable to be vectorized should be the
     * innermost one. */
    EXPORT Func &vectorize(VarOrRVar var);

    /** Mark a dimension to be completely unrolled. The dimension
     * should have constant extent - e.g. because it is the inner
     * dimension following a split by a constant factor. For most uses
     * of unroll you want the two-argument form. */
    EXPORT Func &unroll(VarOrRVar var);

    /** Split a dimension by the given factor, then vectorize the
     * inner dimension. This is how you vectorize a loop of unknown
     * size. The variable to be vectorized should be the innermost
     * one. After this call, var refers to the outer dimension of the
     * split. */
    EXPORT Func &vectorize(VarOrRVar var, int factor);

    /** Split a dimension by the given factor, then unroll the inner
     * dimension. This is how you unroll a loop of unknown size by
     * some constant factor. After this call, var refers to the outer
     * dimension of the split. */
    EXPORT Func &unroll(VarOrRVar var, int factor);

    /** Statically declare that the range over which a function should
     * be evaluated is given by the second and third arguments. This
     * can let Halide perform some optimizations. E.g. if you know
     * there are going to be 4 color channels, you can completely
     * vectorize the color channel dimension without the overhead of
     * splitting it up. If bounds inference decides that it requires
     * more of this function than the bounds you have stated, a
     * runtime error will occur when you try to run your pipeline. */
    EXPORT Func &bound(Var var, Expr min, Expr extent);

    /** Split two dimensions at once by the given factors, and then
     * reorder the resulting dimensions to be xi, yi, xo, yo from
     * innermost outwards. This gives a tiled traversal. */
    EXPORT Func &tile(VarOrRVar x, VarOrRVar y,
                      VarOrRVar xo, VarOrRVar yo,
                      VarOrRVar xi, VarOrRVar yi,
                      Expr xfactor, Expr yfactor);

    /** A shorter form of tile, which reuses the old variable names as
     * the new outer dimensions */
    EXPORT Func &tile(VarOrRVar x, VarOrRVar y,
                      VarOrRVar xi, VarOrRVar yi,
                      Expr xfactor, Expr yfactor);

    /** Reorder variables to have the given nesting order, from
     * innermost out */
    EXPORT Func &reorder(const std::vector<VarOrRVar> &vars);

    /** Reorder two dimensions so that x is traversed inside y. Does
     * not affect the nesting order of other dimensions. E.g, if you
     * say foo(x, y, z, w) = bar; foo.reorder(w, x); then foo will be
     * traversed in the order (w, y, z, x), from innermost
     * outwards. */
    EXPORT Func &reorder(VarOrRVar x, VarOrRVar y);

    /** Reorder three dimensions to have the given nesting order, from
     * innermost out */
    EXPORT Func &reorder(VarOrRVar x, VarOrRVar y, VarOrRVar z);

    /** Reorder four dimensions to have the given nesting order, from
     * innermost out */
    EXPORT Func &reorder(VarOrRVar x, VarOrRVar y, VarOrRVar z,
                         VarOrRVar w);

    /** Reorder five dimensions to have the given nesting order, from
     * innermost out */
    EXPORT Func &reorder(VarOrRVar x, VarOrRVar y, VarOrRVar z,
                         VarOrRVar w, VarOrRVar t);

    /** Reorder six dimensions to have the given nesting order, from
     * innermost out */
    EXPORT Func &reorder(VarOrRVar x, VarOrRVar y, VarOrRVar z,
                         VarOrRVar w, VarOrRVar t1, VarOrRVar t2);

    /** Reorder seven dimensions to have the given nesting order, from
     * innermost out */
    EXPORT Func &reorder(VarOrRVar x, VarOrRVar y, VarOrRVar z,
                         VarOrRVar w, VarOrRVar t1, VarOrRVar t2,
                         VarOrRVar t3);

    /** Reorder eight dimensions to have the given nesting order, from
     * innermost out */
    EXPORT Func &reorder(VarOrRVar x, VarOrRVar y, VarOrRVar z,
                         VarOrRVar w, VarOrRVar t1, VarOrRVar t2,
                         VarOrRVar t3, VarOrRVar t4);

    /** Reorder nine dimensions to have the given nesting order, from
     * innermost out */
    EXPORT Func &reorder(VarOrRVar x, VarOrRVar y, VarOrRVar z,
                         VarOrRVar w, VarOrRVar t1, VarOrRVar t2,
                         VarOrRVar t3, VarOrRVar t4, VarOrRVar t5);

    /** Reorder ten dimensions to have the given nesting order, from
     * innermost out */
    EXPORT Func &reorder(VarOrRVar x, VarOrRVar y, VarOrRVar z,
                         VarOrRVar w, VarOrRVar t1, VarOrRVar t2,
                         VarOrRVar t3, VarOrRVar t4, VarOrRVar t5,
                         VarOrRVar t6);

    /** Rename a dimension. Equivalent to split with a inner size of one. */
    EXPORT Func &rename(VarOrRVar old_name, VarOrRVar new_name);

    /** Specify that race conditions are permitted for this Func,
     * which enables parallelizing over RVars even when Halide cannot
     * prove that it is safe to do so. Use this with great caution,
     * and only if you can prove to yourself that this is safe, as it
     * may result in a non-deterministic routine that returns
     * different values at different times or on different machines. */
    EXPORT Func &allow_race_conditions();


    /** Specialize a Func. This creates a special-case version of the
     * Func where the given condition is true. The most effective
     * conditions are those of the form param == value, and boolean
     * Params. Consider a simple example:
     \code
     f(x) = x + select(cond, 0, 1);
     f.compute_root();
     \endcode
     * This is equivalent to:
     \code
     for (int x = 0; x < width; x++) {
       f[x] = x + (cond ? 0 : 1);
     }
     \endcode
     * Adding the scheduling directive:
     \code
     f.specialize(cond)
     \endcode
     * makes it equivalent to:
     \code
     if (cond) {
       for (int x = 0; x < width; x++) {
         f[x] = x;
       }
     } else {
       for (int x = 0; x < width; x++) {
         f[x] = x + 1;
       }
     }
     \endcode
     * Note that the inner loops have been simplified. In the first
     * path Halide knows that cond is true, and in the second path
     * Halide knows that it is false.
     *
     * The specialized version gets its own schedule, which inherits
     * every directive made about the parent Func's schedule so far
     * except for its specializations. This method returns a handle to
     * the new schedule. If you wish to retrieve the specialized
     * sub-schedule again later, you can call this method with the
     * same condition. Consider the following example of scheduling
     * the specialized version:
     *
     \code
     f(x) = x;
     f.compute_root();
     f.specialize(width > 1).unroll(x, 2);
     \endcode
     * Assuming for simplicity that width is even, this is equivalent to:
     \code
     if (width > 1) {
       for (int x = 0; x < width/2; x++) {
         f[2*x] = 2*x;
         f[2*x + 1] = 2*x + 1;
       }
     } else {
       for (int x = 0; x < width/2; x++) {
         f[x] = x;
       }
     }
     \endcode
     * For this case, it may be better to schedule the un-specialized
     * case instead:
     \code
     f(x) = x;
     f.compute_root();
     f.specialize(width == 1); // Creates a copy of the schedule so far.
     f.unroll(x, 2); // Only applies to the unspecialized case.
     \endcode
     * This is equivalent to:
     \code
     if (width == 1) {
       f[0] = 0;
     } else {
       for (int x = 0; x < width/2; x++) {
         f[2*x] = 2*x;
         f[2*x + 1] = 2*x + 1;
       }
     }
     \endcode
     * This can be a good way to write a pipeline that splits,
     * vectorizes, or tiles, but can still handle small inputs.
     *
     * If a Func has several specializations, the first matching one
     * will be used, so the order in which you define specializations
     * is significant. For example:
     *
     \code
     f(x) = x + select(cond1, a, b) - select(cond2, c, d);
     f.specialize(cond1);
     f.specialize(cond2);
     \endcode
     * is equivalent to:
     \code
     if (cond1) {
       for (int x = 0; x < width; x++) {
         f[x] = x + a - (cond2 ? c : d);
       }
     } else if (cond2) {
       for (int x = 0; x < width; x++) {
         f[x] = x + b - c;
       }
     } else {
       for (int x = 0; x < width; x++) {
         f[x] = x + b - d;
       }
     }
     \endcode
     *
     * Specializations may in turn be specialized, which creates a
     * nested if statement in the generated code.
     *
     \code
     f(x) = x + select(cond1, a, b) - select(cond2, c, d);
     f.specialize(cond1).specialize(cond2);
     \endcode
     * This is equivalent to:
     \code
     if (cond1) {
       if (cond2) {
         for (int x = 0; x < width; x++) {
           f[x] = x + a - c;
         }
       } else {
         for (int x = 0; x < width; x++) {
           f[x] = x + a - d;
         }
       }
     } else {
       for (int x = 0; x < width; x++) {
         f[x] = x + b - (cond2 ? c : d);
       }
     }
     \endcode
     * To create a 4-way if statement that simplifies away all of the
     * ternary operators above, you could say:
     \code
     f.specialize(cond1).specialize(cond2);
     f.specialize(cond2);
     \endcode
     * or
     \code
     f.specialize(cond1 && cond2);
     f.specialize(cond1);
     f.specialize(cond2);
     \endcode
     *
     * Any prior Func which is compute_at some variable of this Func
     * gets separately included in all paths of the generated if
     * statement. The Var in the compute_at call to must exist in all
     * paths, but it may have been generated via a different path of
     * splits, fuses, and renames. This can be used somewhat
     * creatively. Consider the following code:
     \code
     g(x, y) = 8*x;
     f(x, y) = g(x, y) + 1;
     f.compute_root().specialize(cond);
     Var g_loop;
     f.specialize(cond).rename(y, g_loop);
     f.rename(x, g_loop);
     g.compute_at(f, g_loop);
     \endcode
     * When cond is true, this is equivalent to g.compute_at(f,y).
     * When it is false, this is equivalent to g.compute_at(f,x).
     */
    EXPORT Stage specialize(Expr condition);

    /** Tell Halide that the following dimensions correspond to GPU
     * thread indices. This is useful if you compute a producer
     * function within the block indices of a consumer function, and
     * want to control how that function's dimensions map to GPU
     * threads. If the selected target is not an appropriate GPU, this
     * just marks those dimensions as parallel. */
    // @{
    EXPORT Func &gpu_threads(VarOrRVar thread_x, GPUAPI gpu_api = GPU_Default);
    EXPORT Func &gpu_threads(VarOrRVar thread_x, VarOrRVar thread_y, GPUAPI gpu_api = GPU_Default);
    EXPORT Func &gpu_threads(VarOrRVar thread_x, VarOrRVar thread_y, VarOrRVar thread_z, GPUAPI gpu_api = GPU_Default);
    // @}

    /** Tell Halide to run this stage using a single gpu thread and
     * block. This is not an efficient use of your GPU, but it can be
     * useful to avoid copy-back for intermediate update stages that
     * touch a very small part of your Func. */
    EXPORT Func &gpu_single_thread(GPUAPI gpu_api = GPU_Default);

    /** \deprecated Old name for #gpu_threads. */
    // @{
    EXPORT Func &cuda_threads(VarOrRVar thread_x) {
        return gpu_threads(thread_x);
    }
    EXPORT Func &cuda_threads(VarOrRVar thread_x, VarOrRVar thread_y) {
        return gpu_threads(thread_x, thread_y);
    }
    EXPORT Func &cuda_threads(VarOrRVar thread_x, VarOrRVar thread_y, VarOrRVar thread_z) {
        return gpu_threads(thread_x, thread_y, thread_z);
    }
    // @}

    /** Tell Halide that the following dimensions correspond to GPU
     * block indices. This is useful for scheduling stages that will
     * run serially within each GPU block. If the selected target is
     * not ptx, this just marks those dimensions as parallel. */
    // @{
    EXPORT Func &gpu_blocks(VarOrRVar block_x, GPUAPI gpu_api = GPU_Default);
    EXPORT Func &gpu_blocks(VarOrRVar block_x, VarOrRVar block_y, GPUAPI gpu_api = GPU_Default);
    EXPORT Func &gpu_blocks(VarOrRVar block_x, VarOrRVar block_y, VarOrRVar block_z, GPUAPI gpu_api = GPU_Default);
    // @}

    /** \deprecated Old name for #gpu_blocks. */
    // @{
    EXPORT Func &cuda_blocks(VarOrRVar block_x) {
        return gpu_blocks(block_x);
    }
    EXPORT Func &cuda_blocks(VarOrRVar block_x, VarOrRVar block_y) {
        return gpu_blocks(block_x, block_y);
    }
    EXPORT Func &cuda_blocks(VarOrRVar block_x, VarOrRVar block_y, VarOrRVar block_z) {
        return gpu_blocks(block_x, block_y, block_z);
    }
    // @}

    /** Tell Halide that the following dimensions correspond to GPU
     * block indices and thread indices. If the selected target is not
     * ptx, these just mark the given dimensions as parallel. The
     * dimensions are consumed by this call, so do all other
     * unrolling, reordering, etc first. */
    // @{
    EXPORT Func &gpu(VarOrRVar block_x, VarOrRVar thread_x, GPUAPI gpu_api = GPU_Default);
    EXPORT Func &gpu(VarOrRVar block_x, VarOrRVar block_y,
                     VarOrRVar thread_x, VarOrRVar thread_y, GPUAPI gpu_api = GPU_Default);
    EXPORT Func &gpu(VarOrRVar block_x, VarOrRVar block_y, VarOrRVar block_z,
                     VarOrRVar thread_x, VarOrRVar thread_y, VarOrRVar thread_z, GPUAPI gpu_api = GPU_Default);
    // @}

    /** \deprecated Old name for #gpu. */
    // @{
    EXPORT Func &cuda(VarOrRVar block_x, VarOrRVar thread_x) {
        return gpu(block_x, thread_x);
    }
    EXPORT Func &cuda(VarOrRVar block_x, VarOrRVar block_y,
                      VarOrRVar thread_x, VarOrRVar thread_y) {
        return gpu(block_x, thread_x, block_y, thread_y);
    }
    EXPORT Func &cuda(VarOrRVar block_x, VarOrRVar block_y, VarOrRVar block_z,
                      VarOrRVar thread_x, VarOrRVar thread_y, VarOrRVar thread_z) {
        return gpu(block_x, thread_x, block_y, thread_y, block_z, thread_z);
    }
    // @}

    /** Short-hand for tiling a domain and mapping the tile indices
     * to GPU block indices and the coordinates within each tile to
     * GPU thread indices. Consumes the variables given, so do all
     * other scheduling first. */
    // @{
    EXPORT Func &gpu_tile(VarOrRVar x, int x_size, GPUAPI gpu_api = GPU_Default);
    EXPORT Func &gpu_tile(VarOrRVar x, VarOrRVar y, int x_size, int y_size, GPUAPI gpu_api = GPU_Default);
    EXPORT Func &gpu_tile(VarOrRVar x, VarOrRVar y, VarOrRVar z,
                          int x_size, int y_size, int z_size, GPUAPI gpu_api = GPU_Default);
    // @}

    /** \deprecated Old name for #gpu_tile. */
    // @{
    EXPORT Func &cuda_tile(VarOrRVar x, int x_size) {
        return gpu_tile(x, x_size);
    }
    EXPORT Func &cuda_tile(VarOrRVar x, VarOrRVar y, int x_size, int y_size) {
        return gpu_tile(x, y, x_size, y_size);
    }
    EXPORT Func &cuda_tile(VarOrRVar x, VarOrRVar y, VarOrRVar z,
                           int x_size, int y_size, int z_size) {
        return gpu_tile(x, y, z, x_size, y_size, z_size);
    }
    // @}

    /** Schedule for execution using GLSL. Conceptually, this is similar to
     * parallelization over 'x' and 'y' (since GLSL shaders compute individual
     * output pixels in parallel) and vectorization over 'c' (since GLSL
     * implicitly vectorizes the color channel). */
    EXPORT Func &glsl(Var x, Var y, Var c);

    /** Specify how the storage for the function is laid out. These
     * calls let you specify the nesting order of the dimensions. For
     * example, foo.reorder_storage(y, x) tells Halide to use
     * column-major storage for any realizations of foo, without
     * changing how you refer to foo in the code. You may want to do
     * this if you intend to vectorize across y. When representing
     * color images, foo.reorder_storage(c, x, y) specifies packed
     * storage (red, green, and blue values adjacent in memory), and
     * foo.reorder_storage(x, y, c) specifies planar storage (entire
     * red, green, and blue images one after the other in memory).
     *
     * If you leave out some dimensions, those remain in the same
     * positions in the nesting order while the specified variables
     * are reordered around them. */
    // @{
    EXPORT Func &reorder_storage(Var x, Var y);
    EXPORT Func &reorder_storage(Var x, Var y, Var z);
    EXPORT Func &reorder_storage(Var x, Var y, Var z, Var w);
    EXPORT Func &reorder_storage(Var x, Var y, Var z, Var w, Var t);
    // @}

    /** Compute this function as needed for each unique value of the
     * given var for the given calling function f.
     *
     * For example, consider the simple pipeline:
     \code
     Func f, g;
     Var x, y;
     g(x, y) = x*y;
     f(x, y) = g(x, y) + g(x, y+1) + g(x+1, y) + g(x+1, y+1);
     \endcode
     *
     * If we schedule f like so:
     *
     \code
     g.compute_at(f, x);
     \endcode
     *
     * Then the C code equivalent to this pipeline will look like this
     *
     \code

     int f[height][width];
     for (int y = 0; y < height; y++) {
         for (int x = 0; x < width; x++) {
             int g[2][2];
             g[0][0] = x*y;
             g[0][1] = (x+1)*y;
             g[1][0] = x*(y+1);
             g[1][1] = (x+1)*(y+1);
             f[y][x] = g[0][0] + g[1][0] + g[0][1] + g[1][1];
         }
     }

     \endcode
     *
     * The allocation and computation of g is within f's loop over x,
     * and enough of g is computed to satisfy all that f will need for
     * that iteration. This has excellent locality - values of g are
     * used as soon as they are computed, but it does redundant
     * work. Each value of g ends up getting computed four times. If
     * we instead schedule f like so:
     *
     \code
     g.compute_at(f, y);
     \endcode
     *
     * The equivalent C code is:
     *
     \code
     int f[height][width];
     for (int y = 0; y < height; y++) {
         int g[2][width+1];
         for (int x = 0; x < width; x++) {
             g[0][x] = x*y;
             g[1][x] = x*(y+1);
         }
         for (int x = 0; x < width; x++) {
             f[y][x] = g[0][x] + g[1][x] + g[0][x+1] + g[1][x+1];
         }
     }
     \endcode
     *
     * The allocation and computation of g is within f's loop over y,
     * and enough of g is computed to satisfy all that f will need for
     * that iteration. This does less redundant work (each point in g
     * ends up being evaluated twice), but the locality is not quite
     * as good, and we have to allocate more temporary memory to store
     * g.
     */
    EXPORT Func &compute_at(Func f, Var var);

    /** Schedule a function to be computed within the iteration over
     * some dimension of an update domain. Produces equivalent code
     * to the version of compute_at that takes a Var. */
    EXPORT Func &compute_at(Func f, RVar var);

    /** Compute all of this function once ahead of time. Reusing
     * the example in \ref Func::compute_at :
     *
     \code
     Func f, g;
     Var x, y;
     g(x, y) = x*y;
     f(x, y) = g(x, y) + g(x, y+1) + g(x+1, y) + g(x+1, y+1);

     g.compute_root();
     \endcode
     *
     * is equivalent to
     *
     \code
     int f[height][width];
     int g[height+1][width+1];
     for (int y = 0; y < height+1; y++) {
         for (int x = 0; x < width+1; x++) {
             g[y][x] = x*y;
         }
     }
     for (int y = 0; y < height; y++) {
         for (int x = 0; x < width; x++) {
             f[y][x] = g[y][x] + g[y+1][x] + g[y][x+1] + g[y+1][x+1];
         }
     }
     \endcode
     *
     * g is computed once ahead of time, and enough is computed to
     * satisfy all uses of it. This does no redundant work (each point
     * in g is evaluated once), but has poor locality (values of g are
     * probably not still in cache when they are used by f), and
     * allocates lots of temporary memory to store g.
     */
    EXPORT Func &compute_root();

    /** Use the halide_memoization_cache_... interface to store a
     *  computed version of this function across invocations of the
     *  Func.
     */
    EXPORT Func &memoize();


    /** Allocate storage for this function within f's loop over
     * var. Scheduling storage is optional, and can be used to
     * separate the loop level at which storage occurs from the loop
     * level at which computation occurs to trade off between locality
     * and redundant work. This can open the door for two types of
     * optimization.
     *
     * Consider again the pipeline from \ref Func::compute_at :
     \code
     Func f, g;
     Var x, y;
     g(x, y) = x*y;
     f(x, y) = g(x, y) + g(x+1, y) + g(x, y+1) + g(x+1, y+1);
     \endcode
     *
     * If we schedule it like so:
     *
     \code
     g.compute_at(f, x).store_at(f, y);
     \endcode
     *
     * Then the computation of g takes place within the loop over x,
     * but the storage takes place within the loop over y:
     *
     \code
     int f[height][width];
     for (int y = 0; y < height; y++) {
         int g[2][width+1];
         for (int x = 0; x < width; x++) {
             g[0][x] = x*y;
             g[0][x+1] = (x+1)*y;
             g[1][x] = x*(y+1);
             g[1][x+1] = (x+1)*(y+1);
             f[y][x] = g[0][x] + g[1][x] + g[0][x+1] + g[1][x+1];
         }
     }
     \endcode
     *
     * Provided the for loop over x is serial, halide then
     * automatically performs the following sliding window
     * optimization:
     *
     \code
     int f[height][width];
     for (int y = 0; y < height; y++) {
         int g[2][width+1];
         for (int x = 0; x < width; x++) {
             if (x == 0) {
                 g[0][x] = x*y;
                 g[1][x] = x*(y+1);
             }
             g[0][x+1] = (x+1)*y;
             g[1][x+1] = (x+1)*(y+1);
             f[y][x] = g[0][x] + g[1][x] + g[0][x+1] + g[1][x+1];
         }
     }
     \endcode
     *
     * Two of the assignments to g only need to be done when x is
     * zero. The rest of the time, those sites have already been
     * filled in by a previous iteration. This version has the
     * locality of compute_at(f, x), but allocates more memory and
     * does much less redundant work.
     *
     * Halide then further optimizes this pipeline like so:
     *
     \code
     int f[height][width];
     for (int y = 0; y < height; y++) {
         int g[2][2];
         for (int x = 0; x < width; x++) {
             if (x == 0) {
                 g[0][0] = x*y;
                 g[1][0] = x*(y+1);
             }
             g[0][(x+1)%2] = (x+1)*y;
             g[1][(x+1)%2] = (x+1)*(y+1);
             f[y][x] = g[0][x%2] + g[1][x%2] + g[0][(x+1)%2] + g[1][(x+1)%2];
         }
     }
     \endcode
     *
     * Halide has detected that it's possible to use a circular buffer
     * to represent g, and has reduced all accesses to g modulo 2 in
     * the x dimension. This optimization only triggers if the for
     * loop over x is serial, and if halide can statically determine
     * some power of two large enough to cover the range needed. For
     * powers of two, the modulo operator compiles to more efficient
     * bit-masking. This optimization reduces memory usage, and also
     * improves locality by reusing recently-accessed memory instead
     * of pulling new memory into cache.
     *
     */
    EXPORT Func &store_at(Func f, Var var);

    /** Equivalent to the version of store_at that takes a Var, but
     * schedules storage within the loop over a dimension of a
     * reduction domain */
    EXPORT Func &store_at(Func f, RVar var);

    /** Equivalent to \ref Func::store_at, but schedules storage
     * outside the outermost loop. */
    EXPORT Func &store_root();

    /** Aggressively inline all uses of this function. This is the
     * default schedule, so you're unlikely to need to call this. For
     * a Func with an update definition, that means it gets computed
     * as close to the innermost loop as possible.
     *
     * Consider once more the pipeline from \ref Func::compute_at :
     *
     \code
     Func f, g;
     Var x, y;
     g(x, y) = x*y;
     f(x, y) = g(x, y) + g(x+1, y) + g(x, y+1) + g(x+1, y+1);
     \endcode
     *
     * Leaving g as inline, this compiles to code equivalent to the following C:
     *
     \code
     int f[height][width];
     for (int y = 0; y < height; y++) {
         for (int x = 0; x < width; x++) {
             f[y][x] = x*y + x*(y+1) + (x+1)*y + (x+1)*(y+1);
         }
     }
     \endcode
     */
    EXPORT Func &compute_inline();

    /** Get a handle on an update step for the purposes of scheduling
     * it. */
    EXPORT Stage update(int idx = 0);

    /** Trace all loads from this Func by emitting calls to
     * halide_trace. If the Func is inlined, this has no
     * effect. */
    EXPORT Func &trace_loads();

    /** Trace all stores to the buffer backing this Func by emitting
     * calls to halide_trace. If the Func is inlined, this call
     * has no effect. */
    EXPORT Func &trace_stores();

    /** Trace all realizations of this Func by emitting calls to
     * halide_trace. */
    EXPORT Func &trace_realizations();

    /** Get a handle on the internal halide function that this Func
     * represents. Useful if you want to do introspection on Halide
     * functions */
    Internal::Function function() const {
        return func;
    }

    /** You can cast a Func to its pure stage for the purposes of
     * scheduling it. */
    operator Stage() const;

    /** Get a handle on the output buffer for this Func. Only relevant
     * if this is the output Func in a pipeline. Useful for making
     * static promises about strides, mins, and extents. */
    // @{
    EXPORT OutputImageParam output_buffer() const;
    EXPORT std::vector<OutputImageParam> output_buffers() const;
    // @}

    /** Casting a function to an expression is equivalent to calling
     * the function with zero arguments. Implicit variables will be
     * injected according to the function's dimensionality
     * (see \ref Var::implicit).
     *
     * This lets you write things like:
     *
     \code
     Func f, g;
     Var x;
     g(x) = ...
     f(_) = g * 2;
     \endcode
    */
    operator Expr() const {
        return (*this)(_);
    }

    /** Use a Func as an argument to an external stage. */
    operator ExternFuncArgument() const {
        return ExternFuncArgument(func);
    }

    /** Infer the arguments to the Func, sorted into a canonical order:
     * all buffers (sorted alphabetically by name), followed by all non-buffers
     * (sorted alphabetically by name).
     This lets you write things like:
     \code
     func.compile_to_assembly("/dev/stdout", func.infer_arguments());
     \endcode
     */
    EXPORT std::vector<Argument> infer_arguments() const;

};

 /** JIT-Compile and run enough code to evaluate a Halide
  * expression. This can be thought of as a scalar version of
  * \ref Func::realize */
template<typename T>
NO_INLINE T evaluate(Expr e) {
    user_assert(e.type() == type_of<T>())
        << "Can't evaluate expression "
        << e << " of type " << e.type()
        << " as a scalar of type " << type_of<T>() << "\n";
    Func f;
    f() = e;
    Image<T> im = f.realize();
    return im(0);
}

/** JIT-compile and run enough code to evaluate a Halide Tuple. */
// @{
template<typename A, typename B>
NO_INLINE void evaluate(Tuple t, A *a, B *b) {
    user_assert(t[0].type() == type_of<A>())
        << "Can't evaluate expression "
        << t[0] << " of type " << t[0].type()
        << " as a scalar of type " << type_of<A>() << "\n";
    user_assert(t[1].type() == type_of<B>())
        << "Can't evaluate expression "
        << t[1] << " of type " << t[1].type()
        << " as a scalar of type " << type_of<B>() << "\n";

    Func f;
    f() = t;
    Realization r = f.realize();
    *a = Image<A>(r[0])(0);
    *b = Image<B>(r[1])(0);
}

template<typename A, typename B, typename C>
NO_INLINE void evaluate(Tuple t, A *a, B *b, C *c) {
    user_assert(t[0].type() == type_of<A>())
        << "Can't evaluate expression "
        << t[0] << " of type " << t[0].type()
        << " as a scalar of type " << type_of<A>() << "\n";
    user_assert(t[1].type() == type_of<B>())
        << "Can't evaluate expression "
        << t[1] << " of type " << t[1].type()
        << " as a scalar of type " << type_of<B>() << "\n";
    user_assert(t[2].type() == type_of<C>())
        << "Can't evaluate expression "
        << t[2] << " of type " << t[2].type()
        << " as a scalar of type " << type_of<C>() << "\n";

    Func f;
    f() = t;
    Realization r = f.realize();
    *a = Image<A>(r[0])(0);
    *b = Image<B>(r[1])(0);
    *c = Image<C>(r[2])(0);
}

template<typename A, typename B, typename C, typename D>
NO_INLINE void evaluate(Tuple t, A *a, B *b, C *c, D *d) {
    user_assert(t[0].type() == type_of<A>())
        << "Can't evaluate expression "
        << t[0] << " of type " << t[0].type()
        << " as a scalar of type " << type_of<A>() << "\n";
    user_assert(t[1].type() == type_of<B>())
        << "Can't evaluate expression "
        << t[1] << " of type " << t[1].type()
        << " as a scalar of type " << type_of<B>() << "\n";
    user_assert(t[2].type() == type_of<C>())
        << "Can't evaluate expression "
        << t[2] << " of type " << t[2].type()
        << " as a scalar of type " << type_of<C>() << "\n";
    user_assert(t[3].type() == type_of<D>())
        << "Can't evaluate expression "
        << t[3] << " of type " << t[3].type()
        << " as a scalar of type " << type_of<D>() << "\n";

    Func f;
    f() = t;
    Realization r = f.realize();
    *a = Image<A>(r[0])(0);
    *b = Image<B>(r[1])(0);
    *c = Image<C>(r[2])(0);
    *d = Image<D>(r[3])(0);
}
 // @}


/** JIT-Compile and run enough code to evaluate a Halide
 * expression. This can be thought of as a scalar version of
 * \ref Func::realize. Can use GPU if jit target from environment
 * specifies one.
 */
template<typename T>
NO_INLINE T evaluate_may_gpu(Expr e) {
    user_assert(e.type() == type_of<T>())
        << "Can't evaluate expression "
        << e << " of type " << e.type()
        << " as a scalar of type " << type_of<T>() << "\n";
    bool has_gpu_feature = get_jit_target_from_environment().has_gpu_feature();
    Func f;
    f() = e;
    if (has_gpu_feature) {
        f.gpu_single_thread();
    }
    Image<T> im = f.realize();
    return im(0);
}

/** JIT-compile and run enough code to evaluate a Halide Tuple. Can
 *  use GPU if jit target from environment specifies one. */
// @{
template<typename A, typename B>
NO_INLINE void evaluate_may_gpu(Tuple t, A *a, B *b) {
    user_assert(t[0].type() == type_of<A>())
        << "Can't evaluate expression "
        << t[0] << " of type " << t[0].type()
        << " as a scalar of type " << type_of<A>() << "\n";
    user_assert(t[1].type() == type_of<B>())
        << "Can't evaluate expression "
        << t[1] << " of type " << t[1].type()
        << " as a scalar of type " << type_of<B>() << "\n";

    bool has_gpu_feature = get_jit_target_from_environment().has_gpu_feature();
    Func f;
    f() = t;
    if (has_gpu_feature) {
        f.gpu_single_thread();
    }
    Realization r = f.realize();
    *a = Image<A>(r[0])(0);
    *b = Image<B>(r[1])(0);
}

template<typename A, typename B, typename C>
NO_INLINE void evaluate_may_gpu(Tuple t, A *a, B *b, C *c) {
    user_assert(t[0].type() == type_of<A>())
        << "Can't evaluate expression "
        << t[0] << " of type " << t[0].type()
        << " as a scalar of type " << type_of<A>() << "\n";
    user_assert(t[1].type() == type_of<B>())
        << "Can't evaluate expression "
        << t[1] << " of type " << t[1].type()
        << " as a scalar of type " << type_of<B>() << "\n";
    user_assert(t[2].type() == type_of<C>())
        << "Can't evaluate expression "
        << t[2] << " of type " << t[2].type()
        << " as a scalar of type " << type_of<C>() << "\n";
    bool has_gpu_feature = get_jit_target_from_environment().has_gpu_feature();
    Func f;
    f() = t;
    if (has_gpu_feature) {
        f.gpu_single_thread();
    }
    Realization r = f.realize();
    *a = Image<A>(r[0])(0);
    *b = Image<B>(r[1])(0);
    *c = Image<C>(r[2])(0);
}

template<typename A, typename B, typename C, typename D>
NO_INLINE void evaluate_may_gpu(Tuple t, A *a, B *b, C *c, D *d) {
    user_assert(t[0].type() == type_of<A>())
        << "Can't evaluate expression "
        << t[0] << " of type " << t[0].type()
        << " as a scalar of type " << type_of<A>() << "\n";
    user_assert(t[1].type() == type_of<B>())
        << "Can't evaluate expression "
        << t[1] << " of type " << t[1].type()
        << " as a scalar of type " << type_of<B>() << "\n";
    user_assert(t[2].type() == type_of<C>())
        << "Can't evaluate expression "
        << t[2] << " of type " << t[2].type()
        << " as a scalar of type " << type_of<C>() << "\n";
    user_assert(t[3].type() == type_of<D>())
        << "Can't evaluate expression "
        << t[3] << " of type " << t[3].type()
        << " as a scalar of type " << type_of<D>() << "\n";

    bool has_gpu_feature = get_jit_target_from_environment().has_gpu_feature();
    Func f;
    f() = t;
    if (has_gpu_feature) {
        f.gpu_single_thread();
    }
    Realization r = f.realize();
    *a = Image<A>(r[0])(0);
    *b = Image<B>(r[1])(0);
    *c = Image<C>(r[2])(0);
    *d = Image<D>(r[3])(0);
}
// @}

}


#endif
#ifndef HALIDE_INLINE_REDUCTIONS_H
#define HALIDE_INLINE_REDUCTIONS_H


/** \file
 * Defines some inline reductions: sum, product, minimum, maximum.
 */
namespace Halide {

/** An inline reduction. This is suitable for convolution-type
 * operations - the reduction will be computed in the innermost loop
 * that it is used in. The argument may contain free or implicit
 * variables, and must refer to some reduction domain. The free
 * variables are still free in the return value, but the reduction
 * domain is captured - the result expression does not refer to a
 * reduction domain and can be used in a pure function definition.
 *
 * An example using \ref sum :
 *
 \code
 Func f, g;
 Var x;
 RDom r(0, 10);
 f(x) = x*x;
 g(x) = sum(f(x + r));
 \endcode
 *
 * Here g computes some blur of x, but g is still a pure function. The
 * sum is being computed by an anonymous reduction function that is
 * scheduled innermost within g.
 */
//@{
EXPORT Expr sum(Expr, const std::string &s = "sum");
EXPORT Expr product(Expr, const std::string &s = "product");
EXPORT Expr maximum(Expr, const std::string &s = "maximum");
EXPORT Expr minimum(Expr, const std::string &s = "minimum");
//@}

/** Variants of the inline reduction in which the RDom is stated
 * explicitly. The expression can refer to multiple RDoms, and only
 * the inner one is captured by the reduction. This allows you to
 * write expressions like:
 \code
 RDom r1(0, 10), r2(0, 10), r3(0, 10);
 Expr e = minimum(r1, product(r2, sum(r3, r1 + r2 + r3)));
 \endcode
*/
// @{
EXPORT Expr sum(RDom, Expr, const std::string &s = "sum");
EXPORT Expr product(RDom, Expr, const std::string &s = "product");
EXPORT Expr maximum(RDom, Expr, const std::string &s = "maximum");
EXPORT Expr minimum(RDom, Expr, const std::string &s = "minimum");
// @}


/** Returns an Expr or Tuple representing the coordinates of the point
 * in the RDom which minimizes or maximizes the expression. The
 * expression must refer to some RDom. Also returns the extreme value
 * of the expression as the last element of the tuple. */
// @{
EXPORT Tuple argmax(Expr, const std::string &s = "argmax");
EXPORT Tuple argmin(Expr, const std::string &s = "argmin");
EXPORT Tuple argmax(RDom, Expr, const std::string &s = "argmax");
EXPORT Tuple argmin(RDom, Expr, const std::string &s = "argmin");
// @}

}

#endif
#ifndef HALIDE_INTEGER_DIVISION_TABLE_H
#define HALIDE_INTEGER_DIVISION_TABLE_H

/** \file
 * Tables telling us how to do integer division via fixed-point
 * multiplication for various small constants.
 */
namespace Halide {
namespace Internal {
namespace IntegerDivision {

extern int64_t table_u8[256][4];
extern int64_t table_s8[256][4];
extern int64_t table_u16[256][4];
extern int64_t table_s16[256][4];
extern int64_t table_u32[256][4];
extern int64_t table_s32[256][4];

extern int64_t table_runtime_u8[256][4];
extern int64_t table_runtime_s8[256][4];
extern int64_t table_runtime_u16[256][4];
extern int64_t table_runtime_s16[256][4];
extern int64_t table_runtime_u32[256][4];
extern int64_t table_runtime_s32[256][4];

}
}
}

#endif
#ifndef HALIDE_IR_EQUALITY_H
#define HALIDE_IR_EQUALITY_H

/** \file
 * Methods to test Exprs and Stmts for equality of value
 */


namespace Halide {
namespace Internal {

/** A compare struct suitable for use in std::map and std::set that
 * computes a lexical ordering on IR nodes. */
struct IRDeepCompare {
    EXPORT bool operator()(const Expr &a, const Expr &b) const;
    EXPORT bool operator()(const Stmt &a, const Stmt &b) const;
};

/** Lossily track known equal exprs with a cache. On collision, the
 * old pair is evicted. Used below by ExprWithCompareCache. */
class IRCompareCache {
private:
    struct Entry {
        Expr a, b;
    };

    int bits;

    uint32_t hash(const Expr &a, const Expr &b) const {
        // Note this hash is symmetric in a and b, so that a
        // comparison in a and b hashes to the same bucket as
        // a comparison on b and a.
        uint64_t pa = (uint64_t)(a.ptr);
        uint64_t pb = (uint64_t)(b.ptr);
        pa ^= pb;
        pa ^= pa >> bits;
        pa ^= pa >> (bits*2);
        return pa & ((1 << bits) - 1);
    }

    std::vector<Entry> entries;

public:
    void insert(const Expr &a, const Expr &b) {
        uint32_t h = hash(a, b);
        entries[h].a = a;
        entries[h].b = b;
    }

    bool contains(const Expr &a, const Expr &b) const {
        uint32_t h = hash(a, b);
        const Entry &e = entries[h];
        return ((a.same_as(e.a) && b.same_as(e.b)) ||
                (a.same_as(e.b) && b.same_as(e.a)));
    }

    void clear() {
        for (size_t i = 0; i < entries.size(); i++) {
            entries[i].a = Expr();
            entries[i].b = Expr();
        }
    }

    IRCompareCache() {}
    IRCompareCache(int b) : bits(b), entries(1 << bits) {}

};

/** A wrapper about Exprs so that they can be deeply compared with a
 * cache for known-equal subexpressions. Useful for unsanitized Exprs
 * coming in from the front-end, which may be horrible graphs with
 * sub-expressions that are equal by value but not by identity. This
 * isn't a comparison object like IRDeepCompare above, because libc++
 * requires that comparison objects be stateless (and constructs a new
 * one for each comparison!), so they can't have a cache associated
 * with them. However, by sneakily making the cache a mutable member
 * of the objects being compared, we can dodge this issue.
 *
 * Clunky example usage:
 *
\code
Expr a, b, c, query;
std::set<ExprWithCompareCache> s;
IRCompareCache cache(8);
s.insert(ExprWithCompareCache(a, &cache));
s.insert(ExprWithCompareCache(b, &cache));
s.insert(ExprWithCompareCache(c, &cache));
if (m.contains(ExprWithCompareCache(query, &cache))) {...}
\endcode
 *
 */
struct ExprWithCompareCache {
    Expr expr;
    mutable IRCompareCache *cache;

    ExprWithCompareCache() : cache(NULL) {}
    ExprWithCompareCache(const Expr &e, IRCompareCache *c) : expr(e), cache(c) {}

    /** The comparison uses (and updates) the cache */
    EXPORT bool operator<(const ExprWithCompareCache &other) const;
};

/** Compare IR nodes for equality of value. Traverses entire IR
 * tree. For equality of reference, use Expr::same_as */
// @{
EXPORT bool equal(Expr a, Expr b);
EXPORT bool equal(Stmt a, Stmt b);
// @}

EXPORT void ir_equality_test();

}
}

#endif
#ifndef HALIDE_IR_MATCH_H
#define HALIDE_IR_MATCH_H

/** \file
 * Defines a method to match a fragment of IR against a pattern containing wildcards
 */

#include <vector>


namespace Halide {
namespace Internal {

/** Does the first expression have the same structure as the second?
 * Variables in the first expression with the name * are interpreted
 * as wildcards, and their matching equivalent in the second
 * expression is placed in the vector give as the third argument.
 *
 * For example:
 \code
 Expr x = new Variable(Int(32), "*");
 match(x + x, 3 + (2*k), result)
 \endcode
 * should return true, and set result[0] to 3 and
 * result[1] to 2*k.
 */

bool expr_match(Expr pattern, Expr expr, std::vector<Expr> &result);

/** Does the first expression have the same structure as the second?
 * Variables are matched consistently. The first time a variable is
 * matched, it assumes the value of the matching part of the second
 * expression. Subsequent matches must be equal to the first match.
 *
 * For example:
 \code
 Var x("x"), y("y");
 match(x*(x + 1), a*(a + b), result)
 \endcode
 * should return true, and set result["x"] = a, and result["y"] = b.
 */

bool expr_match(Expr pattern, Expr expr, std::map<std::string, Expr> &result);

void expr_match_test();

}
}

#endif
#ifndef HALIDE_IR_MUTATOR_H
#define HALIDE_IR_MUTATOR_H

/** \file
 * Defines a base class for passes over the IR that modify it
 */


namespace Halide {
namespace Internal {

/** A base class for passes over the IR which modify it
 * (e.g. replacing a variable with a value (Substitute.h), or
 * constant-folding).
 *
 * Your mutate should override the visit methods you care about. Return
 * the new expression by assigning to expr or stmt. The default ones
 * recursively mutate their children. To mutate sub-expressions and
 * sub-statements you should the mutate method, which will dispatch to
 * the appropriate visit method and then return the value of expr or
 * stmt after the call to visit.
 */
class IRMutator : public IRVisitor {
public:

    /** This is the main interface for using a mutator. Also call
     * these in your subclass to mutate sub-expressions and
     * sub-statements.
     */
    virtual Expr mutate(Expr expr);
    virtual Stmt mutate(Stmt stmt);

protected:

    /** visit methods that take Exprs assign to this to return their
     * new value */
    Expr expr;

    /** visit methods that take Stmts assign to this to return their
     * new value */
    Stmt stmt;

    virtual void visit(const IntImm *);
    virtual void visit(const FloatImm *);
    virtual void visit(const StringImm *);
    virtual void visit(const Cast *);
    virtual void visit(const Variable *);
    virtual void visit(const Add *);
    virtual void visit(const Sub *);
    virtual void visit(const Mul *);
    virtual void visit(const Div *);
    virtual void visit(const Mod *);
    virtual void visit(const Min *);
    virtual void visit(const Max *);
    virtual void visit(const EQ *);
    virtual void visit(const NE *);
    virtual void visit(const LT *);
    virtual void visit(const LE *);
    virtual void visit(const GT *);
    virtual void visit(const GE *);
    virtual void visit(const And *);
    virtual void visit(const Or *);
    virtual void visit(const Not *);
    virtual void visit(const Select *);
    virtual void visit(const Load *);
    virtual void visit(const Ramp *);
    virtual void visit(const Broadcast *);
    virtual void visit(const Call *);
    virtual void visit(const Let *);
    virtual void visit(const LetStmt *);
    virtual void visit(const AssertStmt *);
    virtual void visit(const Pipeline *);
    virtual void visit(const For *);
    virtual void visit(const Store *);
    virtual void visit(const Provide *);
    virtual void visit(const Allocate *);
    virtual void visit(const Free *);
    virtual void visit(const Realize *);
    virtual void visit(const Block *);
    virtual void visit(const IfThenElse *);
    virtual void visit(const Evaluate *);
};

}
}

#endif
#ifndef FIND_CALLS_H
#define FIND_CALLS_H

/** \file
 *
 * Defines analyses to extract the functions called a function.
 */

#include <string>
#include <map>

namespace Halide {
namespace Internal {

/** Construct a map from name to Function definition object for all Halide
 *  functions called directly in the definition of the Function f, including
 *  in update definitions, update index expressions, and RDom extents. This map
 *  _does not_ include the Function f, unless it is called recursively by
 *  itself.
 */
std::map<std::string, Function> find_direct_calls(Function f);

/** Construct a map from name to Function definition object for all Halide
 *  functions called directly in the definition of the Function f, or
 *  indirectly in those functions' definitions, recursively. This map always
 *  _includes_ the Function f.
 */
std::map<std::string, Function> find_transitive_calls(Function f);

}
}

#endif
#ifndef HALIDE_LAMBDA_H
#define HALIDE_LAMBDA_H


/** \file
 * Convenience functions for creating small anonymous Halide
 * functions. See test/lambda.cpp for example usage. */

namespace Halide {

/** Create a zero-dimensional halide function that returns the given
 * expression. The function may have more dimensions if the expression
 * contains implicit arguments. */
inline Func lambda(Expr e) {
    Func f("lambda" + Internal::unique_name('_'));
    f(_) = e;
    return f;
}

/** Create a 1-D halide function in the first argument that returns
 * the second argument. The function may have more dimensions if the
 * expression contains implicit arguments and the list of Var
 * arguments contains a placeholder ("_"). */
inline Func lambda(Var x, Expr e) {
    Func f("lambda" + Internal::unique_name('_'));
    f(x) = e;
    return f;
}

/** Create a 2-D halide function in the first two arguments that
 * returns the last argument. The function may have more dimensions if
 * the expression contains implicit arguments and the list of Var
 * arguments contains a placeholder ("_"). */
inline Func lambda(Var x, Var y, Expr e) {
    Func f("lambda" + Internal::unique_name('_'));
    f(x, y) = e;
    return f;
}

/** Create a 3-D halide function in the first three arguments that
 * returns the last argument.  The function may have more dimensions
 * if the expression contains implicit arguments and the list of Var
 * arguments contains a placeholder ("_"). */
inline Func lambda(Var x, Var y, Var z, Expr e) {
    Func f("lambda" + Internal::unique_name('_'));
    f(x, y, z) = e;
    return f;
}

/** Create a 4-D halide function in the first four arguments that
 * returns the last argument. The function may have more dimensions if
 * the expression contains implicit arguments and the list of Var
 * arguments contains a placeholder ("_"). */
inline Func lambda(Var x, Var y, Var z, Var w, Expr e) {
    Func f("lambda" + Internal::unique_name('_'));
    f(x, y, z, w) = e;
    return f;
}

/** Create a 5-D halide function in the first five arguments that
 * returns the last argument. The function may have more dimensions if
 * the expression contains implicit arguments and the list of Var
 * arguments contains a placeholder ("_"). */
inline Func lambda(Var x, Var y, Var z, Var w, Var v, Expr e) {
    Func f("lambda" + Internal::unique_name('_'));
    f(x, y, z, w, v) = e;
    return f;
}

}

#endif //HALIDE_LAMBDA_H
#ifndef HALIDE_INTERNAL_LOWER_H
#define HALIDE_INTERNAL_LOWER_H

/** \file
 *
 * Defines the function that generates a statement that computes a
 * Halide function using its schedule.
 */


namespace Halide {
namespace Internal {

/** Given a halide function with a schedule, create a statement that
 * evaluates it. Automatically pulls in all the functions f depends
 * on. Some stages of lowering may be target-specific. */
Stmt lower(Function f, const Target &t);

void lower_test();

}
}

#endif
/** \file
 * This file only exists to contain the front-page of the documentation
 */

/** \mainpage Halide
 *
 * Halide is a programming language designed to make it easier to
 * write high-performance image processing code on modern
 * machines. Its front end is embedded in C++. Compiler
 * targets include x86/SSE, ARM v7/NEON, CUDA, Native Client, and
 * OpenCL.
 *
 * You build a Halide program by writing C++ code using objects of
 * type \ref Halide::Var, \ref Halide::Expr, and \ref Halide::Func,
 * and then calling \ref Halide::Func::compile_to_file to generate an
 * object file and header (good for deploying large routines), or
 * calling \ref Halide::Func::realize to JIT-compile and run the
 * pipeline immediately (good for testing small routines).
 *
 * To learn Halide, we recommend you start with the <a href=examples.html>tutorials</a>.
 *
 * You can also look in the test folder for many small examples that
 * use Halide's various features, and in the apps folder for some
 * larger examples that statically compile halide pipelines. In
 * particular check out local_laplacian, bilateral_grid, and
 * interpolate.
 *
 * Below are links to the documentation for the important classes in Halide.
 *
 * For defining, scheduling, and evaluating basic pipelines:
 *
 * Halide::Func, Halide::Stage, Halide::Var
 *
 * Our image data type:
 *
 * Halide::Image
 *
 * For passing around and reusing halide expressions:
 *
 * Halide::Expr
 *
 * For representing scalar and image parameters to pipelines:
 *
 * Halide::Param, Halide::ImageParam
 *
 * For writing functions that reduce or scatter over some domain:
 *
 * Halide::RDom
 *
 * For writing and evaluating functions that return multiple values:
 *
 * Halide::Tuple, Halide::Realization
 *
 */

/**
 * \example tutorial/lesson_01_basics.cpp
 * \example tutorial/lesson_02_input_image.cpp
 * \example tutorial/lesson_03_debugging_1.cpp
 * \example tutorial/lesson_04_debugging_2.cpp
 * \example tutorial/lesson_05_scheduling_1.cpp
 * \example tutorial/lesson_06_realizing_over_shifted_domains.cpp
 * \example tutorial/lesson_07_multi_stage_pipelines.cpp
 * \example tutorial/lesson_08_scheduling_2.cpp
 * \example tutorial/lesson_09_update_definitions.cpp
 * \example tutorial/lesson_10_aot_compilation_generate.cpp
 * \example tutorial/lesson_10_aot_compilation_run.cpp
 * \example tutorial/lesson_11_cross_compilation.cpp
 * \example tutorial/lesson_12_using_the_gpu.cpp
 */
#ifndef HALIDE_REMOVE_TRIVIAL_FOR_LOOPS_H
#define HALIDE_REMOVE_TRIVIAL_FOR_LOOPS_H

/** \file
 * Defines the lowering pass removes for loops of size 1
 */


namespace Halide {
namespace Internal {

/** Convert for loops of size 1 into LetStmt nodes, which allows for
 * further simplification. Done during a late stage of lowering. */
Stmt remove_trivial_for_loops(Stmt s);

}
}

#endif
#ifndef HALIDE_SIMPLIFY_H
#define HALIDE_SIMPLIFY_H

/** \file
 * Methods for simplifying halide statements and expressions
 */

#include <cmath>

namespace Halide {
namespace Internal {

/** Perform a a wide range of simplifications to expressions and
 * statements, including constant folding, substituting in trivial
 * values, arithmetic rearranging, etc. Simplifies across let
 * statements, so must not be called on stmts with dangling or
 * repeated variable names.
 */
// @{
EXPORT Stmt simplify(Stmt, bool simplify_lets = true,
                     const Scope<Interval> &bounds = Scope<Interval>::empty_scope(),
                     const Scope<ModulusRemainder> &alignment = Scope<ModulusRemainder>::empty_scope());
EXPORT Expr simplify(Expr, bool simplify_lets = true,
                     const Scope<Interval> &bounds = Scope<Interval>::empty_scope(),
                     const Scope<ModulusRemainder> &alignment = Scope<ModulusRemainder>::empty_scope());
// @}

/** Simplify expressions found in a statement, but don't simplify
 * across different statements. This is safe to perform at an earlier
 * stage in lowering than full simplification of a stmt. */
EXPORT Stmt simplify_exprs(Stmt);

/** Implementations of division and mod that are specific to Halide.
 * Use these implementations; do not use native C division or mod to
 * simplify Halide expressions. Halide division and modulo satisify
 * the Euclidean definition of division for integers a and b:
 *
 /code
 (a/b)*b + a%b = a
 0 <= a%b < |b|
 /endcode
 *
 */
// @{
template<typename T>
inline T mod_imp(T a, T b) {
    Type t = type_of<T>();
    if (t.is_int()) {
        T r = a % b;
        r = r + (r < 0 ? (T)std::abs((int)b) : 0);
        return r;
    } else {
        return a % b;
    }
}

template<typename T>
inline T div_imp(T a, T b) {
    Type t = type_of<T>();
    if (t.is_int()) {
        int q = a / b;
        int r = a - q * b;
        int bs = b >> (t.bits - 1);
        int rs = r >> (t.bits - 1);
        return q - (rs & bs) + (rs & ~bs);
    } else {
        return a / b;
    }
}
// @}

// Special cases for float, double.
template<> inline float mod_imp<float>(float a, float b) {
    float f = a - b * (floorf(a / b));
    // The remainder has the same sign as b.
    return f;
}
template<> inline double mod_imp<double>(double a, double b) {
    double f = a - b * (std::floor(a / b));
    return f;
}

template<> inline float div_imp<float>(float a, float b) {
    return a/b;
}
template<> inline double div_imp<double>(double a, double b) {
    return a/b;
}


EXPORT void simplify_test();

}
}

#endif
#ifndef HALIDE_SLIDING_WINDOW_H
#define HALIDE_SLIDING_WINDOW_H

/** \file
 *
 * Defines the sliding_window lowering optimization pass, which avoids
 * computing provably-already-computed values.
 */

#include <map>

namespace Halide {
namespace Internal {

/** Perform sliding window optimizations on a halide
 * statement. I.e. don't bother computing points in a function that
 * have provably already been computed by a previous iteration.
 */
Stmt sliding_window(Stmt s, const std::map<std::string, Function> &env);

}
}

#endif
#ifndef HALIDE_STMT_COMPILER_H
#define HALIDE_STMT_COMPILER_H

/** \file
 * Defines a compiler that produces native code from halide statements
 */


#include <string>
#include <vector>

namespace Halide {

namespace Internal {


/** A handle to a generic statement compiler. Can take Halide
 * statements and turn them into assembly, bitcode, machine code, or a
 * jit-compiled module. */
class CodeGen;
class StmtCompiler {
    IntrusivePtr<CodeGen> contents;
public:

    /** Build a code generator for the given target. */
    StmtCompiler(Target target);

    /** Compile a statement to an llvm module of the given name with
     * the given toplevel arguments, and the given buffers embedded
     * inside it. The module is stored internally until one of the
     * later functions is called: */
    void compile(Stmt stmt, std::string name,
                 const std::vector<Argument> &args,
                 const std::vector<Buffer> &images_to_embed);

    /** Write the module to an llvm bitcode file */
    void compile_to_bitcode(const std::string &filename);

    /** Compile and write the module to either a binary object file,
     * or as assembly */
    void compile_to_native(const std::string &filename, bool assembly = false);

    /** Return a function pointer with type given by the vector of
     * Arguments passed to compile. Also returns a wrapped version of
     * the function, which is a single-argument function that takes an
     * array of void * (i.e. a void **). Each entry in this array
     * either points to a buffer_t, or to a scalar of the type
     * specified by the argument list.
     *
     * Also returns various other useful functions within the module,
     * such as a hook for setting the function to call when an assert
     * fails.
     */
    JITCompiledModule compile_to_function_pointers();
};

}
}



#endif
#ifndef HALIDE_STORAGE_FLATTENING_H
#define HALIDE_STORAGE_FLATTENING_H

/** \file
 * Defines the lowering pass that flattens multi-dimensional storage
 * into single-dimensional array access
 */

#include <map>


namespace Halide {
namespace Internal {

/** Take a statement with multi-dimensional Realize, Provide, and Call
 * nodes, and turn it into a statement with single-dimensional
 * Allocate, Store, and Load nodes respectively. */
Stmt storage_flattening(Stmt s, const std::string &output,
                        const std::map<std::string, Function> &env);

}
}

#endif
#ifndef HALIDE_STORAGE_FOLDING_H
#define HALIDE_STORAGE_FOLDING_H

/** \file
 * Defines the lowering optimization pass that reduces large buffers
 * down to smaller circular buffers when possible
 */


namespace Halide {
namespace Internal {

/** Fold storage of functions if possible. This means reducing one of
 * the dimensions module something for the purpose of storage, if we
 * can prove that this is safe to do. E.g consider:
 *
 \code
 f(x) = ...
 g(x) = f(x-1) + f(x)
 f.store_root().compute_at(g, x);
 \endcode
 *
 * We can store f as a circular buffer of size two, instead of
 * allocating space for all of it.
 */
Stmt storage_folding(Stmt s);

}
}

#endif
#ifndef HALIDE_SUBSTITUTE_H
#define HALIDE_SUBSTITUTE_H

/** \file
 *
 * Defines methods for substituting out variables in expressions and
 * statements. */

#include <map>


namespace Halide {
namespace Internal {

/** Substitute variables with the given name with the replacement
 * expression within expr. This is a dangerous thing to do if variable
 * names have not been uniquified. While it won't traverse inside let
 * statements with the same name as the first argument, moving a piece
 * of syntax around can change its meaning, because it can cross lets
 * that redefine variable names that it includes references to. */
EXPORT Expr substitute(std::string name, Expr replacement, Expr expr);

/** Substitute variables with the given name with the replacement
 * expression within stmt. */
EXPORT Stmt substitute(std::string name, Expr replacement, Stmt stmt);

/** Substitute variables with names in the map. */
// @{
EXPORT Expr substitute(const std::map<std::string, Expr> &replacements, Expr expr);
EXPORT Stmt substitute(const std::map<std::string, Expr> &replacements, Stmt stmt);
// @}

/** Substitute expressions for other expressions. */
// @{
EXPORT Expr substitute(Expr find, Expr replacement, Expr expr);
EXPORT Stmt substitute(Expr find, Expr replacement, Stmt stmt);
// @}

}
}

#endif
#ifndef HALIDE_PROFILING_H
#define HALIDE_PROFILING_H

/** \file
 * Defines the lowering pass that injects print statements when profiling is turned on
 */


namespace Halide {
namespace Internal {

/** Take a statement representing a halide pipeline, and (depending on
 * the environment variable HL_PROFILE), insert high-resolution timing
 * into the generated code; summaries of execution times and counts will
 * be logged at the end. Should be done before storage flattening, but
 * after all bounds inference. Use util/HalideProf to analyze the output.
 *
 * NOTE: this makes no effort to provide accurate or useful information
 * when parallelization is scheduled; more work would need to be done
 * to safely record data from multiple threads.
 *
 * NOTE: this makes no effort to account for overhead from the profiling
 * instructions inserted; profile-enabled runtimes will be slower,
 * and inner loops will be more profoundly affected.
 */
Stmt inject_profiling(Stmt, std::string);

/** Gets the current profiling level (by reading HL_PROFILE) */
int profiling_level();

}
}

#endif
#ifndef HALIDE_TRACING_H
#define HALIDE_TRACING_H

/** \file
 * Defines the lowering pass that injects print statements when tracing is turned on
 */

#include <map>

namespace Halide {
namespace Internal {

/** Take a statement representing a halide pipeline, inject calls to
 * tracing functions at interesting points, such as
 * allocations. Should be done before storage flattening, but after
 * all bounds inference. */
Stmt inject_tracing(Stmt, const std::map<std::string, Function> &env, Function output);

}
}

#endif
#ifndef HALIDE_UNROLL_LOOPS_H
#define HALIDE_UNROLL_LOOPS_H

/** \file
 * Defines the lowering pass that unrolls loops marked as such
 */


namespace Halide {
namespace Internal {

/** Take a statement with for loops marked for unrolling, and convert
 * each into several copies of the innermost statement. I.e. unroll
 * the loop. */
Stmt unroll_loops(Stmt);

}
}

#endif
#ifndef HALIDE_VECTORIZE_LOOPS_H
#define HALIDE_VECTORIZE_LOOPS_H

/** \file
 * Defines the lowering pass that vectorizes loops marked as such
 */


namespace Halide {
namespace Internal {

/** Take a statement with for loops marked for vectorization, and turn
 * them into single statements that operate on vectors. The loops in
 * question must have constant extent.
 */
Stmt vectorize_loops(Stmt);

}
}

#endif
#ifndef HALIDE_DEBUG_TO_FILE_H
#define HALIDE_DEBUG_TO_FILE_H

/** \file
 * Defines the lowering pass that injects code at the end of
 * every realization to dump functions to a file for debugging.  */

#include <map>

namespace Halide {
namespace Internal {

/** Takes a statement with Realize nodes still unlowered. If the
 * corresponding functions have a debug_file set, then inject code
 * that will dump the contents of those functions to a file after the
 * realization. */
Stmt debug_to_file(Stmt s, std::string output, const std::map<std::string, Function> &env);

}
}

#endif
#ifndef HALIDE_EARLY_FREE_H
#define HALIDE_EARLY_FREE_H

/** \file
 * Defines the lowering pass that injects markers just after
 * the last use of each buffer so that they can potentially be freed
 * earlier.
 */


namespace Halide {
namespace Internal {

/** Take a statement with allocations and inject markers (of the form
 * of calls to "mark buffer dead") after the last use of each
 * allocation. Targets may use this to free buffers earlier than the
 * close of their Allocate node. */
Stmt inject_early_frees(Stmt s);

}
}

#endif
#ifndef HALIDE_UNIQUIFY_VARIABLE_NAMES
#define HALIDE_UNIQUIFY_VARIABLE_NAMES

/** \file
 * Defines the lowering pass that renames all variables to have unique names.
 */


namespace Halide {
namespace Internal {

/** Modify a statement so that every internally-defined variable name
 * is unique. This lets later passes assume syntactic equivalence is
 * semantic equivalence. */
Stmt uniquify_variable_names(Stmt s);

}
}


#endif
#ifndef HALIDE_INTERNAL_CSE_H
#define HALIDE_INTERNAL_CSE_H

/** \file
 * Defines a pass for introducing let expressions to wrap common sub-expressions. */


namespace Halide {
namespace Internal {

/** Replace each common sub-expression in the argument with a
 * variable, and wrap the resulting expr in a let statement giving a
 * value to that variable.
 *
 * This is important to do within Halide (instead of punting to llvm),
 * because exprs that come in from the front-end are small when
 * considered as a graph, but combinatorially large when considered as
 * a tree. For an example of a such a case, see
 * test/code_explosion.cpp */
EXPORT Expr common_subexpression_elimination(Expr);

/** Do common-subexpression-elimination on each expression in a
 * statement. Does not introduce let statements. */
EXPORT Stmt common_subexpression_elimination(Stmt);

EXPORT void cse_test();

}
}

#endif
#ifndef HALIDE_LERP_H
#define HALIDE_LERP_H


/** \file
 * Defines methods for converting a lerp intrinsic into Halide IR.
 */

namespace Halide {
namespace Internal {

/** Build Halide IR that computes a lerp. Use by codegen targets that
 * don't have a native lerp. */
Expr EXPORT lower_lerp(Expr zero_val, Expr one_val, Expr weight);

}
}

#endif
#ifndef HALIDE_SKIP_STAGES
#define HALIDE_SKIP_STAGES


/** \file
 * Defines a pass that dynamically avoids realizing unnecessary stages.
 */

namespace Halide {
namespace Internal {

/** Avoid computing certain stages if we can infer a runtime condition
 * to check that tells us they won't be used. Does this by aanalyzing
 * all reads of each buffer allocated, and inferring some condition
 * that tells us if the reads occur. If the condition is non-trivial,
 * inject ifs that guard the production. */
Stmt skip_stages(Stmt s, const std::vector<std::string> &order);

}
}

#endif
#ifndef HALIDE_SPECIALIZE_CLAMPED_RAMPS_H
#define HALIDE_SPECIALIZE_CLAMPED_RAMPS_H

/** \file
 * Defines a lowering pass that simplifies code using clamped ramps.
 */


namespace Halide {
namespace Internal {

/** Take a statement with multi-dimensional Realize, Provide, and Call
 * nodes, and turn it into a statement with single-dimensional
 * Allocate, Store, and Load nodes respectively. */
Stmt specialize_clamped_ramps(Stmt s);

}
}

#endif
#ifndef HALIDE_REMOVE_UNDEF
#define HALIDE_REMOVE_UNDEF


/** \file
 * Defines a lowering pass that elides stores that depend on unitialized values.
 */

namespace Halide {
namespace Internal {

/** Removes stores that depend on undef values, and statements that
 * only contain such stores. */
Stmt remove_undef(Stmt s);

}
}

#endif
#ifndef HALIDE_FAST_INTEGER_DIVIDE_H
#define HALIDE_FAST_INTEGER_DIVIDE_H


namespace Halide {

/** Built-in images used for fast_integer_divide below. Use of
 * fast_integer_divide will automatically embed the appropriate tables
 * in your object file. They are declared here in case you want to do
 * something non-default with them. */
namespace IntegerDivideTable {
EXPORT Image<uint8_t> integer_divide_table_u8();
EXPORT Image<uint8_t> integer_divide_table_s8();
EXPORT Image<uint16_t> integer_divide_table_u16();
EXPORT Image<uint16_t> integer_divide_table_s16();
EXPORT Image<uint32_t> integer_divide_table_u32();
EXPORT Image<uint32_t> integer_divide_table_s32();
}


/** Integer division by small values can be done exactly as multiplies
 * and shifts. This function does integer division for numerators of
 * various integer types (8, 16, 32 bit signed and unsigned)
 * numerators and uint8 denominators. The type of the result is the
 * type of the numerator. The unsigned version is faster than the
 * signed version, so cast the numerator to an unsigned int if you
 * know it's positive.
 *
 * If your divisor is compile-time constant, Halide performs a
 * slightly better optimization automatically, so there's no need to
 * use this function (but it won't hurt).
 *
 * This function vectorizes well on arm, and well on x86 for 16 and 8
 * bit vectors. For 32-bit vectors on x86 you're better off using
 * native integer division.
 *
 * Also, this routine treats division by zero as division by
 * 256. I.e. it interprets the uint8 divisor as a number from 1 to 256
 * inclusive.
 */
EXPORT Expr fast_integer_divide(Expr numerator, Expr denominator);

}

#endif
#ifndef HALIDE_ALLOCATION_BOUNDS_INFERENCE_H
#define HALIDE_ALLOCATION_BOUNDS_INFERENCE_H

/** \file
 * Defines the lowering pass that determines how large internal allocations should be.
 */

#include <map>
#include <string>


namespace Halide {
namespace Internal {

/** Take a partially statement with Realize nodes in terms of
 * variables, and define values for those variables. */
Stmt allocation_bounds_inference(Stmt s,
                                 const std::map<std::string, Function> &env,
                                 const std::map<std::pair<std::string, int>, Interval> &func_bounds);
}
}

#endif
#ifndef HALIDE_INLINE_H
#define HALIDE_INLINE_H



/** \file
 * Methods for replacing calls to functions with their definitions.
 */

namespace Halide {
namespace Internal {

/** Inline a single named function, which must be pure. */
// @{
Stmt inline_function(Stmt, Function);
Expr inline_function(Expr, Function);
// @}

}
}


#endif
#ifndef HALIDE_QUALIFY_H
#define HALIDE_QUALIFY_H


/** \file
 *
 * Defines methods for prefixing names in an expression with a prefix string.
 */

namespace Halide {
namespace Internal {

/** Prefix all variable names in the given expression with the prefix string. */
Expr qualify(const std::string &prefix, Expr value);

}
}


#endif
#ifndef HALIDE_UNIFY_DUPLICATE_LETS_H
#define HALIDE_UNIFY_DUPLICATE_LETS_H

/** \file
 * Defines the lowering pass that coalesces redundant let statements
 */


namespace Halide {
namespace Internal {

/** Find let statements that all define the same value, and make later
 * ones just reuse the symbol names of the earlier ones. */
Stmt unify_duplicate_lets(Stmt s);

}
}

#endif
#ifndef HALIDE_EXPR_USES_VAR_H
#define HALIDE_EXPR_USES_VAR_H

/** \file
 * Defines a method to determine if an expression depends on some variables.
 */


namespace Halide {
namespace Internal {

/** Test if an expression references the given variable. */
bool expr_uses_var(Expr e, const std::string &v);

template<typename T>
class ExprUsesVars : public IRVisitor {
    using IRVisitor::visit;

    const Scope<T> &scope;

    void visit(const Variable *v) {
        if (scope.contains(v->name)) {
            result = true;
        }
    }
public:
    ExprUsesVars(const Scope<T> &s) : scope(s), result(false) {}
    bool result;
};

/** Test if an expression references any of the variables in a scope. */
template<typename T>
inline bool expr_uses_vars(Expr e, const Scope<T> &s) {
    ExprUsesVars<T> uses(s);
    e.accept(&uses);
    return uses.result;
}

}
}

#endif
#ifndef HALIDE_RANDOM_H
#define HALIDE_RANDOM_H

#include <vector>
#include <string>

/** \file
 *
 * Defines deterministic random functions, and methods to redirect
 * front-end calls to random_float and random_int to use them. */


namespace Halide {
namespace Internal {

/** Return a random floating-point number between zero and one that
 * varies deterministically based on the input expressions. */
Expr random_float(const std::vector<Expr> &);

/** Return a random unsigned integer between zero and 2^32-1 that
 * varies deterministically based on the input expressions (which must
 * be integers). */
Expr random_int(const std::vector<Expr> &);

/** Convert calls to random() to IR generated by random_float and
 * random_int. Tags all calls with the variables in free_vars, and the
 * integer given as the last argument. */
Expr lower_random(Expr e, const std::vector<std::string> &free_vars, int tag);

}
}

#endif
#ifndef HALIDE_CODEGEN_OPENGL_DEV_H
#define HALIDE_CODEGEN_OPENGL_DEV_H

/** \file
 * Defines the code-generator for producing GLSL kernel code
 */


#include <sstream>
#include <map>
#include <string>

namespace Halide {
namespace Internal {

class CodeGen_GLSL;

class CodeGen_OpenGL_Dev : public CodeGen_GPU_Dev {
public:
    CodeGen_OpenGL_Dev(const Target &target);
    ~CodeGen_OpenGL_Dev();

    // CodeGen_GPU_Dev interface
    void add_kernel(Stmt stmt, const std::string &name,
                    const std::vector<GPU_Argument> &args);
    void init_module();
    std::vector<char> compile_to_src();
    std::string get_current_kernel_name();
    void dump();

private:
    CodeGen_GLSL *glc;
    std::ostringstream src_stream;
    std::string cur_kernel_name;
    Target target;
};


/** Compile one statement into GLSL. */
class CodeGen_GLSL : public CodeGen_C {
public:
    CodeGen_GLSL(std::ostream &s);
    void compile(Stmt stmt,
                 std::string name,
                 const std::vector<GPU_Argument> &args,
                 const Target &target);

    EXPORT static void test();

protected:
    using CodeGen_C::visit;
    std::string print_type(Type type);
    std::string print_name(const std::string &);

    void visit(const FloatImm *);

    void visit(const Cast *);
    void visit(const For *);
    void visit(const Select *);

    void visit(const Max *);
    void visit(const Min *);
    void visit(const Div *);
    void visit(const Mod *);

    void visit(const Load *);
    void visit(const Store *);

    void visit(const Call *);
    void visit(const AssertStmt *);
    void visit(const Broadcast *);

    void visit(const Evaluate *);

private:
    std::string get_vector_suffix(Expr e);

    std::map<std::string, std::string> builtin;
};

}}

#endif
#ifndef HALIDE_INJECT_OPENGL_INTRINSICS_H
#define HALIDE_INJECT_OPENGL_INTRINSICS_H


/** \file
 * Defines the lowering pass that injects texture loads and texture
 * stores for opengl.
 */


namespace Halide {
namespace Internal {

/** Take a statement with for kernel for loops and turn loads and
 * stores inside the loops into OpenGL texture load and store
 * intrinsics. Should only be run when the OpenGL target is active. */
Stmt inject_opengl_intrinsics(Stmt s);

}
}

#endif
#ifndef HALIDE_SYNCTHREADS_H
#define HALIDE_SYNCTHREADS_H

/** \file
 * Defines the lowering pass that fuses and normalizes loops over gpu
 * threads to target OpenCL and CUDA.
 */


namespace Halide {
namespace Internal {

/** Rewrite all GPU loops to have a min of zero. */
Stmt zero_gpu_loop_mins(Stmt s);

/** Converts Halide's GPGPU IR to the OpenCL/CUDA model. Within every
 * loop over gpu block indices, fuse the inner loops over thread
 * indices into a single loop (with predication to turn off
 * threads). Also injects synchronization points as needed, and hoists
 * allocations at the block level out into a single shared memory
 * array. */
Stmt fuse_gpu_thread_loops(Stmt s);

}
}

#endif
#ifndef HALIDE_HOST_GPU_BUFFER_COPIES_H
#define HALIDE_HOST_GPU_BUFFER_COPIES_H

/** \file
 * Defines the lowering passes that deal with host and device buffer flow.
 */


namespace Halide {
namespace Internal {

/** Inject calls to halide_dev_malloc, halide_copy_to_dev, and
 * halide_copy_to_host as needed. */
Stmt inject_host_dev_buffer_copies(Stmt s);

/** Inject calls to halide_dev_free as needed. */
Stmt inject_dev_frees(Stmt s);

}
}

#endif
#ifndef HALIDE_PARALLEL_RVAR_H
#define HALIDE_PARALLEL_RVAR_H

#include <string>

/** \file
 *
 * Method for checking if it's safe to parallelize an update
 * definition across a reduction variable.
 */

namespace Halide {
namespace Internal {

/** Returns whether or not Halide can prove that it is safe to
 * parallelize an update definition across a specific variable. If
 * this returns true, it's definitely safe. If this returns false, it
 * may still be safe, but Halide couldn't prove it.
 */
bool can_parallelize_rvar(const std::string &rvar,
                          const std::string &func,
                          const UpdateDefinition &r);

}
}

#endif
#ifndef HALIDE_BOUNDARY_CONDITIONS_H
#define HALIDE_BOUNDARY_CONDITIONS_H

/** \file
 * Support for imposing boundary conditions on Halide::Funcs.
 */

#include <utility>
#include <vector>


namespace Halide {

/** namespace to hold functions for imposing boundary conditions on
 *  Halide Funcs.
 *
 *  All functions in this namespace transform a source Func to a
 *  result Func where the result produces the values of the source
 *  within a given region and a different set of values outside the
 *  given region. A region is an N dimensional box specified by
 *  mins and extents.
 *
 *  Three areas are defined:
 *      The image is the entire set of values in the region.
 *      The edge is the set of pixels in the image but adjacent
 *          to coordinates that are not
 *      The interior is the image minus the edge (and is undefined
 *          if the extent of any region is 1 or less).
 *
 *  If the source Func has more dimensions than are specified, the extra ones
 *  are unmodified.
 *
 *  Numerous options for specifing the outside area are provided,
 *  including replacement with an expression, repeating the edge
 *  samples, mirroring over the edge, and repeating or mirroring the
 *  entire image.
 *
 *  TODO: Add support for passing Image<T> and ImageParam, and
 *  possibly other types directly to this functions.
 */
namespace BoundaryConditions {

namespace Internal {

#if __cplusplus > 199711L // C++11 arbitrary number of args support

inline NO_INLINE void collect_bounds(std::vector<std::pair<Expr, Expr> > &collected_bounds,
                                     Expr min, Expr extent) {
    collected_bounds.push_back(std::make_pair(min, extent));
}

template <typename ...Bounds>
inline NO_INLINE void collect_bounds(std::vector<std::pair<Expr, Expr> > &collected_bounds,
                                     Expr min, Expr extent, Bounds... bounds) {
    collected_bounds.push_back(std::make_pair(min, extent));
    collect_bounds(collected_bounds, bounds...);
}

#endif // C++11 support.

inline const Func &func_like_to_func(const Func &func) {
    return func;
}

template <typename T>
inline NO_INLINE Func func_like_to_func(T func_like) {
    std::vector<Var> args;
    for (int i = 0; i < func_like.dimensions(); i++) {
        args.push_back(Var::implicit(i));
    }
    Func func;
    func(args) = func_like(args);
    return func;
}

}

/** Impose a boundary condition such that a given expression is returned
 *  everywhere outside the boundary. Generally the expression will be a
 *  constant, though the code currently allows accessing the arguments
 *  of source.
 *
 *  An ImageParam, Image<T>, or similar can be passed instead of a Func. If this
 *  is done and no bounds are given, the boundaries will be taken from the
 *  min and extent methods of the passed object.
 *
 *  (This is similar to setting GL_TEXTURE_WRAP_* to GL_CLAMP_TO_BORDER
 *   and putting value in the border of the texture.)
 */
// @{
EXPORT Func constant_exterior(const Func &source, Expr value,
                              const std::vector<std::pair<Expr, Expr> > &bounds);

template <typename T>
inline NO_INLINE Func constant_exterior(T func_like, Expr value) {
    std::vector<std::pair<Expr, Expr> > object_bounds;
    for (int i = 0; i < func_like.dimensions(); i++) {
        object_bounds.push_back(std::make_pair(Expr(func_like.min(i)), Expr(func_like.extent(i))));
    }

    return constant_exterior(Internal::func_like_to_func(func_like), value, object_bounds);
}

#if __cplusplus > 199711L // C++11 arbitrary number of args support
template <typename T, typename ...Bounds>
inline NO_INLINE Func constant_exterior(T func_like, Expr value,
                                        Bounds... bounds) {
    std::vector<std::pair<Expr, Expr> > collected_bounds;
    Internal::collect_bounds(collected_bounds, bounds...);
    return constant_exterior(Internal::func_like_to_func(func_like), value, collected_bounds);
}
#else
template <typename T>
inline NO_INLINE Func constant_exterior(T func_like, Expr value,
                                        Expr min0, Expr extent0) {
    std::vector<std::pair<Expr, Expr> > bounds;
    bounds.push_back(std::make_pair(min0, extent0));
    return constant_exterior(Internal::func_like_to_func(func_like), value, bounds);
}

template <typename T>
inline NO_INLINE Func constant_exterior(T func_like, Expr value,
                                        Expr min0, Expr extent0,
                                        Expr min1, Expr extent1) {
    std::vector<std::pair<Expr, Expr> > bounds;
    bounds.push_back(std::make_pair(min0, extent0));
    bounds.push_back(std::make_pair(min1, extent1));
    return constant_exterior(Internal::func_like_to_func(func_like), value, bounds);
}

template <typename T>
inline NO_INLINE Func constant_exterior(T func_like, Expr value,
                                        Expr min0, Expr extent0,
                                        Expr min1, Expr extent1,
                                        Expr min2, Expr extent2) {
    std::vector<std::pair<Expr, Expr> > bounds;
    bounds.push_back(std::make_pair(min0, extent0));
    bounds.push_back(std::make_pair(min1, extent1));
    bounds.push_back(std::make_pair(min2, extent2));
    return constant_exterior(Internal::func_like_to_func(func_like), value, bounds);
}

template <typename T>
inline NO_INLINE Func constant_exterior(T func_like, Expr value,
                                        Expr min0, Expr extent0,
                                        Expr min1, Expr extent1,
                                        Expr min2, Expr extent2,
                                        Expr min3, Expr extent3) {
    std::vector<std::pair<Expr, Expr> > bounds;
    bounds.push_back(std::make_pair(min0, extent0));
    bounds.push_back(std::make_pair(min1, extent1));
    bounds.push_back(std::make_pair(min2, extent2));
    bounds.push_back(std::make_pair(min3, extent3));
    return constant_exterior(Internal::func_like_to_func(func_like), value, bounds);
}

template <typename T>
inline NO_INLINE Func constant_exterior(T func_like, Expr value,
                                        Expr min0, Expr extent0,
                                        Expr min1, Expr extent1,
                                        Expr min2, Expr extent2,
                                        Expr min3, Expr extent3,
                                        Expr min4, Expr extent4) {
    std::vector<std::pair<Expr, Expr> > bounds;
    bounds.push_back(std::make_pair(min0, extent0));
    bounds.push_back(std::make_pair(min1, extent1));
    bounds.push_back(std::make_pair(min2, extent2));
    bounds.push_back(std::make_pair(min3, extent3));
    bounds.push_back(std::make_pair(min4, extent4));
    return constant_exterior(Internal::func_like_to_func(func_like), value, bounds);
}

template <typename T>
inline NO_INLINE Func constant_exterior(T func_like, Expr value,
                                        Expr min0, Expr extent0,
                                        Expr min1, Expr extent1,
                                        Expr min2, Expr extent2,
                                        Expr min3, Expr extent3,
                                        Expr min4, Expr extent4,
                                        Expr min5, Expr extent5) {
    std::vector<std::pair<Expr, Expr> > bounds;
    bounds.push_back(std::make_pair(min0, extent0));
    bounds.push_back(std::make_pair(min1, extent1));
    bounds.push_back(std::make_pair(min2, extent2));
    bounds.push_back(std::make_pair(min3, extent3));
    bounds.push_back(std::make_pair(min4, extent4));
    bounds.push_back(std::make_pair(min5, extent5));
    return constant_exterior(Internal::func_like_to_func(func_like), value, bounds);
}
#endif
// @}

/** Impose a boundary condition such that the nearest edge sample is returned
 *  everywhere outside the given region.
 *
 *  An ImageParam, Image<T>, or similar can be passed instead of a Func. If this
 *  is done and no bounds are given, the boundaries will be taken from the
 *  min and extent methods of the passed object.
 *
 *  (This is similar to setting GL_TEXTURE_WRAP_* to GL_CLAMP_TO_EDGE.)
 */
// @{
EXPORT Func repeat_edge(const Func &source,
                        const std::vector<std::pair<Expr, Expr> > &bounds);

template <typename T>
inline NO_INLINE Func repeat_edge(T func_like) {
    std::vector<std::pair<Expr, Expr> > object_bounds;
    for (int i = 0; i < func_like.dimensions(); i++) {
        object_bounds.push_back(std::make_pair(Expr(func_like.min(i)), Expr(func_like.extent(i))));
    }

    return repeat_edge(Internal::func_like_to_func(func_like), object_bounds);
}


#if __cplusplus > 199711L // C++11 arbitrary number of args support
template <typename T, typename ...Bounds>
inline NO_INLINE Func repeat_edge(T func_like, Bounds... bounds) {
    std::vector<std::pair<Expr, Expr> > collected_bounds;
    Internal::collect_bounds(collected_bounds, bounds...);
    return repeat_edge(Internal::func_like_to_func(func_like), collected_bounds);
}
#else
template <typename T>
inline NO_INLINE Func repeat_edge(T func_like,
                                  Expr min0, Expr extent0) {
    std::vector<std::pair<Expr, Expr> > bounds;
    bounds.push_back(std::make_pair(min0, extent0));
    return repeat_edge(Internal::func_like_to_func(func_like), bounds);
}

template <typename T>
inline NO_INLINE Func repeat_edge(T func_like,
                                  Expr min0, Expr extent0,
                                  Expr min1, Expr extent1) {
    std::vector<std::pair<Expr, Expr> > bounds;
    bounds.push_back(std::make_pair(min0, extent0));
    bounds.push_back(std::make_pair(min1, extent1));
    return repeat_edge(Internal::func_like_to_func(func_like), bounds);
}

template <typename T>
inline NO_INLINE Func repeat_edge(T func_like,
                                  Expr min0, Expr extent0,
                                  Expr min1, Expr extent1,
                                  Expr min2, Expr extent2) {
    std::vector<std::pair<Expr, Expr> > bounds;
    bounds.push_back(std::make_pair(min0, extent0));
    bounds.push_back(std::make_pair(min1, extent1));
    bounds.push_back(std::make_pair(min2, extent2));
    return repeat_edge(Internal::func_like_to_func(func_like), bounds);
}

template <typename T>
inline NO_INLINE Func repeat_edge(T func_like,
                                  Expr min0, Expr extent0,
                                  Expr min1, Expr extent1,
                                  Expr min2, Expr extent2,
                                  Expr min3, Expr extent3) {
    std::vector<std::pair<Expr, Expr> > bounds;
    bounds.push_back(std::make_pair(min0, extent0));
    bounds.push_back(std::make_pair(min1, extent1));
    bounds.push_back(std::make_pair(min2, extent2));
    bounds.push_back(std::make_pair(min3, extent3));
    return repeat_edge(Internal::func_like_to_func(func_like), bounds);
}

template <typename T>
inline NO_INLINE Func repeat_edge(T func_like,
                                  Expr min0, Expr extent0,
                                  Expr min1, Expr extent1,
                                  Expr min2, Expr extent2,
                                  Expr min3, Expr extent3,
                                  Expr min4, Expr extent4) {
    std::vector<std::pair<Expr, Expr> > bounds;
    bounds.push_back(std::make_pair(min0, extent0));
    bounds.push_back(std::make_pair(min1, extent1));
    bounds.push_back(std::make_pair(min2, extent2));
    bounds.push_back(std::make_pair(min3, extent3));
    bounds.push_back(std::make_pair(min4, extent4));
    return repeat_edge(Internal::func_like_to_func(func_like), bounds);
}

template <typename T>
inline NO_INLINE Func repeat_edge(T func_like,
                                  Expr min0, Expr extent0,
                                  Expr min1, Expr extent1,
                                  Expr min2, Expr extent2,
                                  Expr min3, Expr extent3,
                                  Expr min4, Expr extent4,
                                  Expr min5, Expr extent5) {
    std::vector<std::pair<Expr, Expr> > bounds;
    bounds.push_back(std::make_pair(min0, extent0));
    bounds.push_back(std::make_pair(min1, extent1));
    bounds.push_back(std::make_pair(min2, extent2));
    bounds.push_back(std::make_pair(min3, extent3));
    bounds.push_back(std::make_pair(min4, extent4));
    bounds.push_back(std::make_pair(min5, extent5));
    return repeat_edge(Internal::func_like_to_func(func_like), bounds);
}
#endif
// @}

/** Impose a boundary condition such that the entire coordinate space is
 *  tiled with copies of the image abutted against each other.
 *
 *  An ImageParam, Image<T>, or similar can be passed instead of a Func. If this
 *  is done and no bounds are given, the boundaries will be taken from the
 *  min and extent methods of the passed object.
 *
 *  (This is similar to setting GL_TEXTURE_WRAP_* to GL_REPEAT.)
 */
// @{
EXPORT Func repeat_image(const Func &source,
                         const std::vector<std::pair<Expr, Expr> > &bounds);

template <typename T>
inline NO_INLINE Func repeat_image(T func_like) {
    std::vector<std::pair<Expr, Expr> > object_bounds;
    for (int i = 0; i < func_like.dimensions(); i++) {
        object_bounds.push_back(std::make_pair(Expr(func_like.min(i)), Expr(func_like.extent(i))));
    }

    return repeat_image(Internal::func_like_to_func(func_like), object_bounds);
}

#if __cplusplus > 199711L // C++11 arbitrary number of args support
template <typename T, typename ...Bounds>
inline NO_INLINE Func repeat_image(T func_like, Bounds... bounds) {
    std::vector<std::pair<Expr, Expr> > collected_bounds;
    Internal::collect_bounds(collected_bounds, bounds...);
    return repeat_image(Internal::func_like_to_func(func_like), collected_bounds);
}
#else
template <typename T>
inline NO_INLINE Func repeat_image(T func_like,
                                   Expr min0, Expr extent0) {
    std::vector<std::pair<Expr, Expr> > bounds;
    bounds.push_back(std::make_pair(min0, extent0));
    return repeat_image(Internal::func_like_to_func(func_like), bounds);
}

template <typename T>
inline NO_INLINE Func repeat_image(T func_like,
                                   Expr min0, Expr extent0,
                                   Expr min1, Expr extent1) {
    std::vector<std::pair<Expr, Expr> > bounds;
    bounds.push_back(std::make_pair(min0, extent0));
    bounds.push_back(std::make_pair(min1, extent1));
    return repeat_image(Internal::func_like_to_func(func_like), bounds);
}

template <typename T>
inline NO_INLINE Func repeat_image(T func_like,
                                   Expr min0, Expr extent0,
                                   Expr min1, Expr extent1,
                                   Expr min2, Expr extent2) {
    std::vector<std::pair<Expr, Expr> > bounds;
    bounds.push_back(std::make_pair(min0, extent0));
    bounds.push_back(std::make_pair(min1, extent1));
    bounds.push_back(std::make_pair(min2, extent2));
    return repeat_image(Internal::func_like_to_func(func_like), bounds);
}

template <typename T>
inline NO_INLINE Func repeat_image(T func_like,
                                   Expr min0, Expr extent0,
                                   Expr min1, Expr extent1,
                                   Expr min2, Expr extent2,
                                   Expr min3, Expr extent3) {
    std::vector<std::pair<Expr, Expr> > bounds;
    bounds.push_back(std::make_pair(min0, extent0));
    bounds.push_back(std::make_pair(min1, extent1));
    bounds.push_back(std::make_pair(min2, extent2));
    bounds.push_back(std::make_pair(min3, extent3));
    return repeat_image(Internal::func_like_to_func(func_like), bounds);
}

template <typename T>
inline NO_INLINE Func repeat_image(T func_like,
                                   Expr min0, Expr extent0,
                                   Expr min1, Expr extent1,
                                   Expr min2, Expr extent2,
                                   Expr min3, Expr extent3,
                                   Expr min4, Expr extent4) {
    std::vector<std::pair<Expr, Expr> > bounds;
    bounds.push_back(std::make_pair(min0, extent0));
    bounds.push_back(std::make_pair(min1, extent1));
    bounds.push_back(std::make_pair(min2, extent2));
    bounds.push_back(std::make_pair(min3, extent3));
    bounds.push_back(std::make_pair(min4, extent4));
    return repeat_image(Internal::func_like_to_func(func_like), bounds);
}

template <typename T>
inline NO_INLINE Func repeat_image(T func_like,
                                   Expr min0, Expr extent0,
                                   Expr min1, Expr extent1,
                                   Expr min2, Expr extent2,
                                   Expr min3, Expr extent3,
                                   Expr min4, Expr extent4,
                                   Expr min5, Expr extent5) {
    std::vector<std::pair<Expr, Expr> > bounds;
    bounds.push_back(std::make_pair(min0, extent0));
    bounds.push_back(std::make_pair(min1, extent1));
    bounds.push_back(std::make_pair(min2, extent2));
    bounds.push_back(std::make_pair(min3, extent3));
    bounds.push_back(std::make_pair(min4, extent4));
    bounds.push_back(std::make_pair(min5, extent5));
    return repeat_image(Internal::func_like_to_func(func_like), bounds);
}
#endif

/** Impose a boundary condition such that the entire coordinate space is
 *  tiled with copies of the image abutted against each other, but mirror
 *  them such that adjacent edges are the same.
 *
 *  An ImageParam, Image<T>, or similar can be passed instead of a Func. If this
 *  is done and no bounds are given, the boundaries will be taken from the
 *  min and extent methods of the passed object.
 *
 *  (This is similar to setting GL_TEXTURE_WRAP_* to GL_MIRRORED_REPEAT.)
 */
// @{
EXPORT Func mirror_image(const Func &source,
                         const std::vector<std::pair<Expr, Expr> > &bounds);

template <typename T>
inline NO_INLINE Func mirror_image(T func_like) {
    std::vector<std::pair<Expr, Expr> > object_bounds;
    for (int i = 0; i < func_like.dimensions(); i++) {
        object_bounds.push_back(std::make_pair(Expr(func_like.min(i)), Expr(func_like.extent(i))));
    }

    return mirror_image(Internal::func_like_to_func(func_like), object_bounds);
}

#if __cplusplus > 199711L // C++11 arbitrary number of args support
template <typename T, typename ...Bounds>
inline NO_INLINE Func mirror_image(T func_like, Bounds... bounds) {
    std::vector<std::pair<Expr, Expr> > collected_bounds;
    Internal::collect_bounds(collected_bounds, bounds...);
    return mirror_image(Internal::func_like_to_func(func_like), collected_bounds);
}
#else
template <typename T>
inline NO_INLINE Func mirror_image(T func_like,
                                   Expr min0, Expr extent0) {
    std::vector<std::pair<Expr, Expr> > bounds;
    bounds.push_back(std::make_pair(min0, extent0));
    return mirror_image(Internal::func_like_to_func(func_like), bounds);
}

template <typename T>
inline NO_INLINE Func mirror_image(T func_like,
                                   Expr min0, Expr extent0,
                                   Expr min1, Expr extent1) {
    std::vector<std::pair<Expr, Expr> > bounds;
    bounds.push_back(std::make_pair(min0, extent0));
    bounds.push_back(std::make_pair(min1, extent1));
    return mirror_image(Internal::func_like_to_func(func_like), bounds);
}

template <typename T>
inline NO_INLINE Func mirror_image(T func_like,
                                   Expr min0, Expr extent0,
                                   Expr min1, Expr extent1,
                                   Expr min2, Expr extent2) {
    std::vector<std::pair<Expr, Expr> > bounds;
    bounds.push_back(std::make_pair(min0, extent0));
    bounds.push_back(std::make_pair(min1, extent1));
    bounds.push_back(std::make_pair(min2, extent2));
    return mirror_image(Internal::func_like_to_func(func_like), bounds);
}

template <typename T>
inline NO_INLINE Func mirror_image(T func_like,
                                   Expr min0, Expr extent0,
                                   Expr min1, Expr extent1,
                                   Expr min2, Expr extent2,
                                   Expr min3, Expr extent3) {
    std::vector<std::pair<Expr, Expr> > bounds;
    bounds.push_back(std::make_pair(min0, extent0));
    bounds.push_back(std::make_pair(min1, extent1));
    bounds.push_back(std::make_pair(min2, extent2));
    bounds.push_back(std::make_pair(min3, extent3));
    return mirror_image(Internal::func_like_to_func(func_like), bounds);
}

template <typename T>
inline NO_INLINE Func mirror_image(T func_like,
                                   Expr min0, Expr extent0,
                                   Expr min1, Expr extent1,
                                   Expr min2, Expr extent2,
                                   Expr min3, Expr extent3,
                                   Expr min4, Expr extent4) {
    std::vector<std::pair<Expr, Expr> > bounds;
    bounds.push_back(std::make_pair(min0, extent0));
    bounds.push_back(std::make_pair(min1, extent1));
    bounds.push_back(std::make_pair(min2, extent2));
    bounds.push_back(std::make_pair(min3, extent3));
    bounds.push_back(std::make_pair(min4, extent4));
    return mirror_image(Internal::func_like_to_func(func_like), bounds);
}

template <typename T>
inline NO_INLINE Func mirror_image(T func_like,
                                   Expr min0, Expr extent0,
                                   Expr min1, Expr extent1,
                                   Expr min2, Expr extent2,
                                   Expr min3, Expr extent3,
                                   Expr min4, Expr extent4,
                                   Expr min5, Expr extent5) {
    std::vector<std::pair<Expr, Expr> > bounds;
    bounds.push_back(std::make_pair(min0, extent0));
    bounds.push_back(std::make_pair(min1, extent1));
    bounds.push_back(std::make_pair(min2, extent2));
    bounds.push_back(std::make_pair(min3, extent3));
    bounds.push_back(std::make_pair(min4, extent4));
    bounds.push_back(std::make_pair(min5, extent5));
    return mirror_image(Internal::func_like_to_func(func_like), bounds);
}
#endif
// @}

/** Impose a boundary condition such that the entire coordinate space is
 *  tiled with copies of the image abutted against each other, but mirror
 *  them such that adjacent edges are the same and then overlap the edges.
 *
 *  This produces an error if any extent is 1 or less. (TODO: check this.)
 *
 *  An ImageParam, Image<T>, or similar can be passed instead of a Func. If this
 *  is done and no bounds are given, the boundaries will be taken from the
 *  min and extent methods of the passed object.
 *
 *  (I do not believe there is a direct GL_TEXTURE_WRAP_* equivalent for this.)
 */
// @{
EXPORT Func mirror_interior(const Func &source,
                            const std::vector<std::pair<Expr, Expr> > &bounds);

template <typename T>
inline NO_INLINE Func mirror_interior(T func_like) {
    std::vector<std::pair<Expr, Expr> > object_bounds;
    for (int i = 0; i < func_like.dimensions(); i++) {
        object_bounds.push_back(std::make_pair(Expr(func_like.min(i)), Expr(func_like.extent(i))));
    }

    return mirror_interior(Internal::func_like_to_func(func_like), object_bounds);
}

#if __cplusplus > 199711L // C++11 arbitrary number of args support
template <typename T, typename ...Bounds>
inline NO_INLINE Func mirror_interior(T func_like, Bounds... bounds) {
    std::vector<std::pair<Expr, Expr> > collected_bounds;
    Internal::collect_bounds(collected_bounds, bounds...);
    return mirror_interior(Internal::func_like_to_func(func_like), collected_bounds);
}
#else
template <typename T>
inline NO_INLINE Func mirror_interior(T func_like,
                                      Expr min0, Expr extent0) {
    std::vector<std::pair<Expr, Expr> > bounds;
    bounds.push_back(std::make_pair(min0, extent0));
    return mirror_interior(Internal::func_like_to_func(func_like), bounds);
}

template <typename T>
inline NO_INLINE Func mirror_interior(T func_like,
                                      Expr min0, Expr extent0,
                                      Expr min1, Expr extent1) {
    std::vector<std::pair<Expr, Expr> > bounds;
    bounds.push_back(std::make_pair(min0, extent0));
    bounds.push_back(std::make_pair(min1, extent1));
    return mirror_interior(Internal::func_like_to_func(func_like), bounds);
}

template <typename T>
inline NO_INLINE Func mirror_interior(T func_like,
                                      Expr min0, Expr extent0,
                                      Expr min1, Expr extent1,
                                      Expr min2, Expr extent2) {
    std::vector<std::pair<Expr, Expr> > bounds;
    bounds.push_back(std::make_pair(min0, extent0));
    bounds.push_back(std::make_pair(min1, extent1));
    bounds.push_back(std::make_pair(min2, extent2));
    return mirror_interior(Internal::func_like_to_func(func_like), bounds);
}

template <typename T>
inline NO_INLINE Func mirror_interior(T func_like,
                                      Expr min0, Expr extent0,
                                      Expr min1, Expr extent1,
                                      Expr min2, Expr extent2,
                                      Expr min3, Expr extent3) {
    std::vector<std::pair<Expr, Expr> > bounds;
    bounds.push_back(std::make_pair(min0, extent0));
    bounds.push_back(std::make_pair(min1, extent1));
    bounds.push_back(std::make_pair(min2, extent2));
    bounds.push_back(std::make_pair(min3, extent3));
    return mirror_interior(Internal::func_like_to_func(func_like), bounds);
}

template <typename T>
inline NO_INLINE Func mirror_interior(T func_like,
                                      Expr min0, Expr extent0,
                                      Expr min1, Expr extent1,
                                      Expr min2, Expr extent2,
                                      Expr min3, Expr extent3,
                                      Expr min4, Expr extent4) {
    std::vector<std::pair<Expr, Expr> > bounds;
    bounds.push_back(std::make_pair(min0, extent0));
    bounds.push_back(std::make_pair(min1, extent1));
    bounds.push_back(std::make_pair(min2, extent2));
    bounds.push_back(std::make_pair(min3, extent3));
    bounds.push_back(std::make_pair(min4, extent4));
    return mirror_interior(Internal::func_like_to_func(func_like), bounds);
}

template <typename T>
inline NO_INLINE Func mirror_interior(T func_like,
                                      Expr min0, Expr extent0,
                                      Expr min1, Expr extent1,
                                      Expr min2, Expr extent2,
                                      Expr min3, Expr extent3,
                                      Expr min4, Expr extent4,
                                      Expr min5, Expr extent5) {
    std::vector<std::pair<Expr, Expr> > bounds;
    bounds.push_back(std::make_pair(min0, extent0));
    bounds.push_back(std::make_pair(min1, extent1));
    bounds.push_back(std::make_pair(min2, extent2));
    bounds.push_back(std::make_pair(min3, extent3));
    bounds.push_back(std::make_pair(min4, extent4));
    bounds.push_back(std::make_pair(min5, extent5));
    return mirror_interior(Internal::func_like_to_func(func_like), bounds);
}
#endif
// @}

}

}

#endif
#ifndef HALIDE_INTERNAL_CACHING_H
#define HALIDE_INTERNAL_CACHING_H

/** \file
 *
 * Defines the interface to the pass that injects support for
 * compute_cached roots.
 */

#include <map>

namespace Halide {
namespace Internal {

/** Transform pipeline calls for Funcs scheduled with memoize to do a
 *  lookup call to the runtime cache implementation, and if there is a
 *  miss, compute the results and call the runtime to store it back to
 *  the cache.
 *  Should leave non-memoized Funcs unchanged.
 */
Stmt inject_memoization(Stmt s, const std::map<std::string, Function> &env,
                        const std::string &name);

}
}

#endif
#ifndef HALIDE_HUMAN_READABLE_STMT
#define HALIDE_HUMAN_READABLE_STMT

/** \file
* Defines methods for simplifying a stmt into a human-readable form.
*/


namespace Halide {
namespace Internal {

/**
 * Returns a Stmt simplified using a concrete size of the output, and
 * other optional values for parameters.
 */
// @{
EXPORT Stmt human_readable_stmt(Function f, Stmt s, Buffer buf);
EXPORT Stmt human_readable_stmt(Function f, Stmt s, Buffer buf,
                                std::map<std::string, Expr> additional_replacements);
// @}

}}

#endif
#ifndef HALIDE_STMT_TO_HTML
#define HALIDE_STMT_TO_HTML

/** \file
 * Defines a function to dump an HTML-formatted stmt to a file.
 */


namespace Halide {
namespace Internal {

/**
 * Dump an HTML-formatted print of a Stmt to filename.
 */
EXPORT void print_to_html(std::string filename, Stmt s);

}}

#endif
// This file gets included at the end of Halide.h


// Clean up macros used inside Halide headers
#undef user_assert
#undef user_error
#undef user_warning
#undef internal_error
#undef internal_assert
#undef halide_runtime_error
