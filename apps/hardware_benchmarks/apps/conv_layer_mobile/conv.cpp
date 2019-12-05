#include <iostream>
#include <math.h>
#include <float.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

extern "C" {
int64_t halide_current_time_ns(void *ctx);
void halide_profiler_pipeline_end(void *, void *);
}

#ifdef _WIN32
__declspec(dllimport) float __cdecl roundf(float);
__declspec(dllimport) double __cdecl round(double);
#else
inline float asinh_f32(float x) {return asinhf(x);}
inline float acosh_f32(float x) {return acoshf(x);}
inline float atanh_f32(float x) {return atanhf(x);}
inline double asinh_f64(double x) {return asinh(x);}
inline double acosh_f64(double x) {return acosh(x);}
inline double atanh_f64(double x) {return atanh(x);}
#endif
inline float sqrt_f32(float x) {return sqrtf(x);}
inline float sin_f32(float x) {return sinf(x);}
inline float asin_f32(float x) {return asinf(x);}
inline float cos_f32(float x) {return cosf(x);}
inline float acos_f32(float x) {return acosf(x);}
inline float tan_f32(float x) {return tanf(x);}
inline float atan_f32(float x) {return atanf(x);}
inline float sinh_f32(float x) {return sinhf(x);}
inline float cosh_f32(float x) {return coshf(x);}
inline float tanh_f32(float x) {return tanhf(x);}
inline float hypot_f32(float x, float y) {return hypotf(x, y);}
inline float exp_f32(float x) {return expf(x);}
inline float log_f32(float x) {return logf(x);}
inline float pow_f32(float x, float y) {return powf(x, y);}
inline float floor_f32(float x) {return floorf(x);}
inline float ceil_f32(float x) {return ceilf(x);}
inline float round_f32(float x) {return roundf(x);}

inline double sqrt_f64(double x) {return sqrt(x);}
inline double sin_f64(double x) {return sin(x);}
inline double asin_f64(double x) {return asin(x);}
inline double cos_f64(double x) {return cos(x);}
inline double acos_f64(double x) {return acos(x);}
inline double tan_f64(double x) {return tan(x);}
inline double atan_f64(double x) {return atan(x);}
inline double sinh_f64(double x) {return sinh(x);}
inline double cosh_f64(double x) {return cosh(x);}
inline double tanh_f64(double x) {return tanh(x);}
inline double hypot_f64(double x, double y) {return hypot(x, y);}
inline double exp_f64(double x) {return exp(x);}
inline double log_f64(double x) {return log(x);}
inline double pow_f64(double x, double y) {return pow(x, y);}
inline double floor_f64(double x) {return floor(x);}
inline double ceil_f64(double x) {return ceil(x);}
inline double round_f64(double x) {return round(x);}

inline float nan_f32() {return NAN;}
inline float neg_inf_f32() {return -INFINITY;}
inline float inf_f32() {return INFINITY;}
inline bool is_nan_f32(float x) {return x != x;}
inline bool is_nan_f64(double x) {return x != x;}

template<typename A, typename B>
inline A reinterpret(const B &b) {
    #if __cplusplus >= 201103L
    static_assert(sizeof(A) == sizeof(B), "type size mismatch");
    #endif
    A a;
    memcpy(&a, &b, sizeof(a));
    return a;
}
inline float float_from_bits(uint32_t bits) {
    return reinterpret<float, uint32_t>(bits);
}

template<typename T>
inline uint8_t halide_count_leading_zeros(T v) {
    int bits = sizeof(v) * 8;
    while (v) {
        v >>= 1;
        bits--;
    }
    return bits;
}

template<typename T>
inline T halide_cpp_max(const T &a, const T &b) {return (a > b) ? a : b;}

template<typename T>
inline T halide_cpp_min(const T &a, const T &b) {return (a < b) ? a : b;}

template<typename A, typename B>
const B &return_second(const A &a, const B &b) {
    (void) a;
    return b;
}

template<typename A, typename B>
inline auto quiet_div(const A &a, const B &b) -> decltype(a / b) {
    return b == 0 ? static_cast<decltype(a / b)>(0) : (a / b);
}

template<typename A, typename B>
inline auto quiet_mod(const A &a, const B &b) -> decltype(a % b) {
    return b == 0 ? static_cast<decltype(a % b)>(0) : (a % b);
}

namespace {
class HalideFreeHelper {
    typedef void (*FreeFunction)(void *user_context, void *p);
    void * user_context;
    void *p;
    FreeFunction free_function;
public:
    HalideFreeHelper(void *user_context, void *p, FreeFunction free_function)
        : user_context(user_context), p(p), free_function(free_function) {}
    ~HalideFreeHelper() { free(); }
    void free() {
        if (p) {
            // TODO: do all free_functions guarantee to ignore a nullptr?
            free_function(user_context, p);
            p = nullptr;
        }
    }
};
} // namespace

#ifndef HALIDE_HALIDERUNTIME_H
#define HALIDE_HALIDERUNTIME_H

#ifndef COMPILING_HALIDE_RUNTIME
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#else
#include "runtime_internal.h"
#endif

#ifdef __cplusplus
// Forward declare type to allow naming typed handles.
// See Type.h for documentation.
template<typename T> struct halide_handle_traits;
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Note that you should not use "inline" along with HALIDE_ALWAYS_INLINE;
// it is not necessary, and may produce warnings for some build configurations.
#ifdef _MSC_VER
#define HALIDE_ALWAYS_INLINE __forceinline
#define HALIDE_NEVER_INLINE __declspec(noinline)
#else
#define HALIDE_ALWAYS_INLINE __attribute__((always_inline)) inline
#define HALIDE_NEVER_INLINE __attribute__((noinline))
#endif

/** \file
 *
 * This file declares the routines used by Halide internally in its
 * runtime. On platforms that support weak linking, these can be
 * replaced with user-defined versions by defining an extern "C"
 * function with the same name and signature.
 *
 * When doing Just In Time (JIT) compilation methods on the Func being
 * compiled must be called instead. The corresponding methods are
 * documented below.
 *
 * All of these functions take a "void *user_context" parameter as their
 * first argument; if the Halide kernel that calls back to any of these
 * functions has been compiled with the UserContext feature set on its Target,
 * then the value of that pointer passed from the code that calls the
 * Halide kernel is piped through to the function.
 *
 * Some of these are also useful to call when using the default
 * implementation. E.g. halide_shutdown_thread_pool.
 *
 * Note that even on platforms with weak linking, some linker setups
 * may not respect the override you provide. E.g. if the override is
 * in a shared library and the halide object files are linked directly
 * into the output, the builtin versions of the runtime functions will
 * be called. See your linker documentation for more details. On
 * Linux, LD_DYNAMIC_WEAK=1 may help.
 *
 */

// Forward-declare to suppress warnings if compiling as C.
struct halide_buffer_t;
struct buffer_t;

/** Print a message to stderr. Main use is to support tracing
 * functionality, print, and print_when calls. Also called by the default
 * halide_error.  This function can be replaced in JITed code by using
 * halide_custom_print and providing an implementation of halide_print
 * in AOT code. See Func::set_custom_print.
 */
// @{
extern void halide_print(void *user_context, const char *);
extern void halide_default_print(void *user_context, const char *);
typedef void (*halide_print_t)(void *, const char *);
extern halide_print_t halide_set_custom_print(halide_print_t print);
// @}

/** Halide calls this function on runtime errors (for example bounds
 * checking failures). This function can be replaced in JITed code by
 * using Func::set_error_handler, or in AOT code by calling
 * halide_set_error_handler. In AOT code on platforms that support
 * weak linking (i.e. not Windows), you can also override it by simply
 * defining your own halide_error.
 */
// @{
extern void halide_error(void *user_context, const char *);
extern void halide_default_error(void *user_context, const char *);
typedef void (*halide_error_handler_t)(void *, const char *);
extern halide_error_handler_t halide_set_error_handler(halide_error_handler_t handler);
// @}

/** Cross-platform mutex. Must be initialized with zero and implementation
 * must treat zero as an unlocked mutex with no waiters, etc.
 */
struct halide_mutex {
    uintptr_t _private[1];
};

/** Cross platform condition variable. Must be initialized to 0. */
struct halide_cond {
    uintptr_t _private[1];
};

/** A basic set of mutex and condition variable functions, which call
 * platform specific code for mutual exclusion. Equivalent to posix
 * calls. */
//@{
extern void halide_mutex_lock(struct halide_mutex *mutex);
extern void halide_mutex_unlock(struct halide_mutex *mutex);
extern void halide_cond_signal(struct halide_cond *cond);
extern void halide_cond_broadcast(struct halide_cond *cond);
extern void halide_cond_signal(struct halide_cond *cond);
extern void halide_cond_wait(struct halide_cond *cond, struct halide_mutex *mutex);
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
typedef int (*halide_task_t)(void *user_context, int task_number, uint8_t *closure);
extern int halide_do_par_for(void *user_context,
                             halide_task_t task,
                             int min, int size, uint8_t *closure);
extern void halide_shutdown_thread_pool();
//@}

/** Set a custom method for performing a parallel for loop. Returns
 * the old do_par_for handler. */
typedef int (*halide_do_par_for_t)(void *, halide_task_t, int, int, uint8_t*);
extern halide_do_par_for_t halide_set_custom_do_par_for(halide_do_par_for_t do_par_for);

/** An opaque struct representing a semaphore. Used by the task system for async tasks. */
struct halide_semaphore_t {
    uint64_t _private[2];
};

/** A struct representing a semaphore and a number of items that must
 * be acquired from it. Used in halide_parallel_task_t below. */
struct halide_semaphore_acquire_t {
    struct halide_semaphore_t *semaphore;
    int count;
};
extern int halide_semaphore_init(struct halide_semaphore_t *, int n);
extern int halide_semaphore_release(struct halide_semaphore_t *, int n);
extern bool halide_semaphore_try_acquire(struct halide_semaphore_t *, int n);
typedef int (*halide_semaphore_init_t)(struct halide_semaphore_t *, int);
typedef int (*halide_semaphore_release_t)(struct halide_semaphore_t *, int);
typedef bool (*halide_semaphore_try_acquire_t)(struct halide_semaphore_t *, int);


/** A task representing a serial for loop evaluated over some range.
 * Note that task_parent is a pass through argument that should be
 * passed to any dependent taks that are invokved using halide_do_parallel_tasks
 * underneath this call. */
typedef int (*halide_loop_task_t)(void *user_context, int min, int extent,
                                  uint8_t *closure, void *task_parent);

/** A parallel task to be passed to halide_do_parallel_tasks. This
 * task may recursively call halide_do_parallel_tasks, and there may
 * be complex dependencies between seemingly unrelated tasks expressed
 * using semaphores. If you are using a custom task system, care must
 * be taken to avoid potential deadlock. This can be done by carefully
 * respecting the static metadata at the end of the task struct.*/
struct halide_parallel_task_t {
    // The function to call. It takes a user context, a min and
    // extent, a closure, and a task system pass through argument.
    halide_loop_task_t fn;

    // The closure to pass it
    uint8_t *closure;

    // The name of the function to be called. For debugging purposes only.
    const char *name;

    // An array of semaphores that must be acquired before the
    // function is called. Must be reacquired for every call made.
    struct halide_semaphore_acquire_t *semaphores;
    int num_semaphores;

    // The entire range the function should be called over. This range
    // may be sliced up and the function called multiple times.
    int min, extent;

    // A parallel task provides several pieces of metadata to prevent
    // unbounded resource usage or deadlock.

    // The first is the minimum number of execution contexts (call
    // stacks or threads) necessary for the function to run to
    // completion. This may be greater than one when there is nested
    // parallelism with internal producer-consumer relationships
    // (calling the function recursively spawns and blocks on parallel
    // sub-tasks that communicate with each other via semaphores). If
    // a parallel runtime calls the function when fewer than this many
    // threads are idle, it may need to create more threads to
    // complete the task, or else risk deadlock due to committing all
    // threads to tasks that cannot complete without more.
    //
    // FIXME: Note that extern stages are assumed to only require a
    // single thread to complete. If the extern stage is itself a
    // Halide pipeline, this may be an underestimate.
    int min_threads;

    // The calls to the function should be in serial order from min to min+extent-1, with only
    // one executing at a time. If false, any order is fine, and
    // concurrency is fine.
    bool serial;
};

/** Enqueue some number of the tasks described above and wait for them
 * to complete. While waiting, the calling threads assists with either
 * the tasks enqueued, or other non-blocking tasks in the task
 * system. Note that task_parent should be NULL for top-level calls
 * and the pass through argument if this call is being made from
 * another task. */
extern int halide_do_parallel_tasks(void *user_context, int num_tasks,
                                    struct halide_parallel_task_t *tasks,
                                    void *task_parent);

/** If you use the default do_par_for, you can still set a custom
 * handler to perform each individual task. Returns the old handler. */
//@{
typedef int (*halide_do_task_t)(void *, halide_task_t, int, uint8_t *);
extern halide_do_task_t halide_set_custom_do_task(halide_do_task_t do_task);
extern int halide_do_task(void *user_context, halide_task_t f, int idx,
                          uint8_t *closure);
//@}

/** The version of do_task called for loop tasks. By default calls the
 * loop task with the same arguments. */
// @{
  typedef int (*halide_do_loop_task_t)(void *, halide_loop_task_t, int, int, uint8_t *, void *);
extern halide_do_loop_task_t halide_set_custom_do_loop_task(halide_do_loop_task_t do_task);
extern int halide_do_loop_task(void *user_context, halide_loop_task_t f, int min, int extent,
                               uint8_t *closure, void *task_parent);
//@}

/** Provide an entire custom tasking runtime via function
 * pointers. Note that do_task and semaphore_try_acquire are only ever
 * called by halide_default_do_par_for and
 * halide_default_do_parallel_tasks, so it's only necessary to provide
 * those if you are mixing in the default implementations of
 * do_par_for and do_parallel_tasks. */
// @{
typedef int (*halide_do_parallel_tasks_t)(void *, int, struct halide_parallel_task_t *,
                                          void *task_parent);
extern void halide_set_custom_parallel_runtime(
    halide_do_par_for_t,
    halide_do_task_t,
    halide_do_loop_task_t,
    halide_do_parallel_tasks_t,
    halide_semaphore_init_t,
    halide_semaphore_try_acquire_t,
    halide_semaphore_release_t
    );
// @}

/** The default versions of the parallel runtime functions. */
// @{
extern int halide_default_do_par_for(void *user_context,
                                     halide_task_t task,
                                     int min, int size, uint8_t *closure);
extern int halide_default_do_parallel_tasks(void *user_context,
                                            int num_tasks,
                                            struct halide_parallel_task_t *tasks,
                                            void *task_parent);
extern int halide_default_do_task(void *user_context, halide_task_t f, int idx,
                                  uint8_t *closure);
extern int halide_default_do_loop_task(void *user_context, halide_loop_task_t f,
                                       int min, int extent,
                                       uint8_t *closure, void *task_parent);
extern int halide_default_semaphore_init(struct halide_semaphore_t *, int n);
extern int halide_default_semaphore_release(struct halide_semaphore_t *, int n);
extern bool halide_default_semaphore_try_acquire(struct halide_semaphore_t *, int n);
// @}

struct halide_thread;

/** Spawn a thread. Returns a handle to the thread for the purposes of
 * joining it. The thread must be joined in order to clean up any
 * resources associated with it. */
extern struct halide_thread *halide_spawn_thread(void (*f)(void *), void *closure);

/** Join a thread. */
extern void halide_join_thread(struct halide_thread *);

/** Set the number of threads used by Halide's thread pool. Returns
 * the old number.
 *
 * n < 0  : error condition
 * n == 0 : use a reasonable system default (typically, number of cpus online).
 * n == 1 : use exactly one thread; this will always enforce serial execution
 * n > 1  : use a pool of exactly n threads.
 *
 * Note that the default iOS and OSX behavior will treat n > 1 like n == 0;
 * that is, any positive value other than 1 will use a system-determined number
 * of threads.
 *
 * (Note that this is only guaranteed when using the default implementations
 * of halide_do_par_for(); custom implementations may completely ignore values
 * passed to halide_set_num_threads().)
 */
extern int halide_set_num_threads(int n);

/** Halide calls these functions to allocate and free memory. To
 * replace in AOT code, use the halide_set_custom_malloc and
 * halide_set_custom_free, or (on platforms that support weak
 * linking), simply define these functions yourself. In JIT-compiled
 * code use Func::set_custom_allocator.
 *
 * If you override them, and find yourself wanting to call the default
 * implementation from within your override, use
 * halide_default_malloc/free.
 *
 * Note that halide_malloc must return a pointer aligned to the
 * maximum meaningful alignment for the platform for the purpose of
 * vector loads and stores. The default implementation uses 32-byte
 * alignment, which is safe for arm and x86. Additionally, it must be
 * safe to read at least 8 bytes before the start and beyond the
 * end.
 */
//@{
extern void *halide_malloc(void *user_context, size_t x);
extern void halide_free(void *user_context, void *ptr);
extern void *halide_default_malloc(void *user_context, size_t x);
extern void halide_default_free(void *user_context, void *ptr);
typedef void *(*halide_malloc_t)(void *, size_t);
typedef void (*halide_free_t)(void *, void *);
extern halide_malloc_t halide_set_custom_malloc(halide_malloc_t user_malloc);
extern halide_free_t halide_set_custom_free(halide_free_t user_free);
//@}

/** Halide calls these functions to interact with the underlying
 * system runtime functions. To replace in AOT code on platforms that
 * support weak linking, define these functions yourself, or use
 * the halide_set_custom_load_library() and halide_set_custom_get_library_symbol()
 * functions. In JIT-compiled code, use JITSharedRuntime::set_default_handlers().
 *
 * halide_load_library and halide_get_library_symbol are equivalent to
 * dlopen and dlsym. halide_get_symbol(sym) is equivalent to
 * dlsym(RTLD_DEFAULT, sym).
 */
//@{
extern void *halide_get_symbol(const char *name);
extern void *halide_load_library(const char *name);
extern void *halide_get_library_symbol(void *lib, const char *name);
extern void *halide_default_get_symbol(const char *name);
extern void *halide_default_load_library(const char *name);
extern void *halide_default_get_library_symbol(void *lib, const char *name);
typedef void *(*halide_get_symbol_t)(const char *name);
typedef void *(*halide_load_library_t)(const char *name);
typedef void *(*halide_get_library_symbol_t)(void *lib, const char *name);
extern halide_get_symbol_t halide_set_custom_get_symbol(halide_get_symbol_t user_get_symbol);
extern halide_load_library_t halide_set_custom_load_library(halide_load_library_t user_load_library);
extern halide_get_library_symbol_t halide_set_custom_get_library_symbol(halide_get_library_symbol_t user_get_library_symbol);
//@}

/** Called when debug_to_file is used inside %Halide code.  See
 * Func::debug_to_file for how this is called
 *
 * Cannot be replaced in JITted code at present.
 */
extern int32_t halide_debug_to_file(void *user_context, const char *filename,
                                    int32_t type_code,
                                    struct halide_buffer_t *buf);

/** Types in the halide type system. They can be ints, unsigned ints,
 * or floats (of various bit-widths), or a handle (which is always 64-bits).
 * Note that the int/uint/float values do not imply a specific bit width
 * (the bit width is expected to be encoded in a separate value).
 */
typedef enum halide_type_code_t
#if __cplusplus >= 201103L
: uint8_t
#endif
{
    halide_type_int = 0,   //!< signed integers
    halide_type_uint = 1,  //!< unsigned integers
    halide_type_float = 2, //!< floating point numbers
    halide_type_handle = 3 //!< opaque pointer type (void *)
} halide_type_code_t;

// Note that while __attribute__ can go before or after the declaration,
// __declspec apparently is only allowed before.
#ifndef HALIDE_ATTRIBUTE_ALIGN
    #ifdef _MSC_VER
        #define HALIDE_ATTRIBUTE_ALIGN(x) __declspec(align(x))
    #else
        #define HALIDE_ATTRIBUTE_ALIGN(x) __attribute__((aligned(x)))
    #endif
#endif

/** A runtime tag for a type in the halide type system. Can be ints,
 * unsigned ints, or floats of various bit-widths (the 'bits'
 * field). Can also be vectors of the same (by setting the 'lanes'
 * field to something larger than one). This struct should be
 * exactly 32-bits in size. */
struct halide_type_t {
    /** The basic type code: signed integer, unsigned integer, or floating point. */
#if __cplusplus >= 201103L
    HALIDE_ATTRIBUTE_ALIGN(1) halide_type_code_t code; // halide_type_code_t
#else
    HALIDE_ATTRIBUTE_ALIGN(1) uint8_t code; // halide_type_code_t
#endif

    /** The number of bits of precision of a single scalar value of this type. */
    HALIDE_ATTRIBUTE_ALIGN(1) uint8_t bits;

    /** How many elements in a vector. This is 1 for scalar types. */
    HALIDE_ATTRIBUTE_ALIGN(2) uint16_t lanes;

#ifdef __cplusplus
    /** Construct a runtime representation of a Halide type from:
     * code: The fundamental type from an enum.
     * bits: The bit size of one element.
     * lanes: The number of vector elements in the type. */
    HALIDE_ALWAYS_INLINE halide_type_t(halide_type_code_t code, uint8_t bits, uint16_t lanes = 1)
        : code(code), bits(bits), lanes(lanes) {
    }

    /** Default constructor is required e.g. to declare halide_trace_event
     * instances. */
    HALIDE_ALWAYS_INLINE halide_type_t() : code((halide_type_code_t)0), bits(0), lanes(0) {}

    /** Compare two types for equality. */
    HALIDE_ALWAYS_INLINE bool operator==(const halide_type_t &other) const {
        return as_u32() == other.as_u32();
    }

    HALIDE_ALWAYS_INLINE bool operator!=(const halide_type_t &other) const {
        return !(*this == other);
    }

    HALIDE_ALWAYS_INLINE bool operator<(const halide_type_t &other) const {
        return as_u32() < other.as_u32();
    }

    /** Size in bytes for a single element, even if width is not 1, of this type. */
    HALIDE_ALWAYS_INLINE int bytes() const { return (bits + 7) / 8; }

private:
    HALIDE_ALWAYS_INLINE uint32_t as_u32() const {
        uint32_t u;
        memcpy(&u, this, sizeof(u));
        return u;
    }
#endif
};

enum halide_trace_event_code_t {halide_trace_load = 0,
                                halide_trace_store = 1,
                                halide_trace_begin_realization = 2,
                                halide_trace_end_realization = 3,
                                halide_trace_produce = 4,
                                halide_trace_end_produce = 5,
                                halide_trace_consume = 6,
                                halide_trace_end_consume = 7,
                                halide_trace_begin_pipeline = 8,
                                halide_trace_end_pipeline = 9,
                                halide_trace_tag = 10 };

struct halide_trace_event_t {
    /** The name of the Func or Pipeline that this event refers to */
    const char *func;

    /** If the event type is a load or a store, this points to the
     * value being loaded or stored. Use the type field to safely cast
     * this to a concrete pointer type and retrieve it. For other
     * events this is null. */
    void *value;

    /** For loads and stores, an array which contains the location
     * being accessed. For vector loads or stores it is an array of
     * vectors of coordinates (the vector dimension is innermost).
     *
     * For realization or production-related events, this will contain
     * the mins and extents of the region being accessed, in the order
     * min0, extent0, min1, extent1, ...
     *
     * For pipeline-related events, this will be null.
     */
    int32_t *coordinates;

    /** For halide_trace_tag, this points to a read-only null-terminated string
     * of arbitrary text. For all other events, this will be null.
     */
    const char *trace_tag;

    /** If the event type is a load or a store, this is the type of
     * the data. Otherwise, the value is meaningless. */
    struct halide_type_t type;

    /** The type of event */
    enum halide_trace_event_code_t event;

    /* The ID of the parent event (see below for an explanation of
     * event ancestry). */
    int32_t parent_id;

    /** If this was a load or store of a Tuple-valued Func, this is
     * which tuple element was accessed. */
    int32_t value_index;

    /** The length of the coordinates array */
    int32_t dimensions;

#ifdef __cplusplus
    // If we don't explicitly mark the default ctor as inline,
    // certain build configurations can fail (notably iOS)
    HALIDE_ALWAYS_INLINE halide_trace_event_t() {}
#endif
};

/** Called when Funcs are marked as trace_load, trace_store, or
 * trace_realization. See Func::set_custom_trace. The default
 * implementation either prints events via halide_print, or if
 * HL_TRACE_FILE is defined, dumps the trace to that file in a
 * sequence of trace packets. The header for a trace packet is defined
 * below. If the trace is going to be large, you may want to make the
 * file a named pipe, and then read from that pipe into gzip.
 *
 * halide_trace returns a unique ID which will be passed to future
 * events that "belong" to the earlier event as the parent id. The
 * ownership hierarchy looks like:
 *
 * begin_pipeline
 * +--trace_tag (if any)
 * +--trace_tag (if any)
 * ...
 * +--begin_realization
 * |  +--produce
 * |  |  +--load/store
 * |  |  +--end_produce
 * |  +--consume
 * |  |  +--load
 * |  |  +--end_consume
 * |  +--end_realization
 * +--end_pipeline
 *
 * Threading means that ownership cannot be inferred from the ordering
 * of events. There can be many active realizations of a given
 * function, or many active productions for a single
 * realization. Within a single production, the ordering of events is
 * meaningful.
 *
 * Note that all trace_tag events (if any) will occur just after the begin_pipeline
 * event, but before any begin_realization events. All trace_tags for a given Func
 * will be emitted in the order added.
 */
// @}
extern int32_t halide_trace(void *user_context, const struct halide_trace_event_t *event);
extern int32_t halide_default_trace(void *user_context, const struct halide_trace_event_t *event);
typedef int32_t (*halide_trace_t)(void *user_context, const struct halide_trace_event_t *);
extern halide_trace_t halide_set_custom_trace(halide_trace_t trace);
// @}

/** The header of a packet in a binary trace. All fields are 32-bit. */
struct halide_trace_packet_t {
    /** The total size of this packet in bytes. Always a multiple of
     * four. Equivalently, the number of bytes until the next
     * packet. */
    uint32_t size;

    /** The id of this packet (for the purpose of parent_id). */
    int32_t id;

    /** The remaining fields are equivalent to those in halide_trace_event_t */
    // @{
    struct halide_type_t type;
    enum halide_trace_event_code_t event;
    int32_t parent_id;
    int32_t value_index;
    int32_t dimensions;
    // @}

    #ifdef __cplusplus
    // If we don't explicitly mark the default ctor as inline,
    // certain build configurations can fail (notably iOS)
    HALIDE_ALWAYS_INLINE halide_trace_packet_t() {}

    /** Get the coordinates array, assuming this packet is laid out in
     * memory as it was written. The coordinates array comes
     * immediately after the packet header. */
    HALIDE_ALWAYS_INLINE const int *coordinates() const {
        return (const int *)(this + 1);
    }

    HALIDE_ALWAYS_INLINE int *coordinates() {
        return (int *)(this + 1);
    }

    /** Get the value, assuming this packet is laid out in memory as
     * it was written. The packet comes immediately after the coordinates
     * array. */
    HALIDE_ALWAYS_INLINE const void *value() const {
        return (const void *)(coordinates() + dimensions);
    }

    HALIDE_ALWAYS_INLINE void *value() {
        return (void *)(coordinates() + dimensions);
    }

    /** Get the func name, assuming this packet is laid out in memory
     * as it was written. It comes after the value. */
    HALIDE_ALWAYS_INLINE const char *func() const {
        return (const char *)value() + type.lanes * type.bytes();
    }

    HALIDE_ALWAYS_INLINE char *func() {
        return (char *)value() + type.lanes * type.bytes();
    }

    /** Get the trace_tag (if any), assuming this packet is laid out in memory
     * as it was written. It comes after the func name. If there is no trace_tag,
     * this will return a pointer to an empty string. */
    HALIDE_ALWAYS_INLINE const char *trace_tag() const {
        const char *f = func();
        // strlen may not be available here
        while (*f++) {
            // nothing
        }
        return f;
    }

    HALIDE_ALWAYS_INLINE char *trace_tag() {
        char *f = func();
        // strlen may not be available here
        while (*f++) {
            // nothing
        }
        return f;
    }
    #endif
};



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

/** All Halide GPU or device backend implementations provide an
 * interface to be used with halide_device_malloc, etc. This is
 * accessed via the functions below.
 */

/** An opaque struct containing per-GPU API implementations of the
 * device functions. */
struct halide_device_interface_impl_t;

/** Each GPU API provides a halide_device_interface_t struct pointing
 * to the code that manages device allocations. You can access these
 * functions directly from the struct member function pointers, or by
 * calling the functions declared below. Note that the global
 * functions are not available when using Halide as a JIT compiler.
 * If you are using raw halide_buffer_t in that context you must use
 * the function pointers in the device_interface struct.
 *
 * The function pointers below are currently the same for every GPU
 * API; only the impl field varies. These top-level functions do the
 * bookkeeping that is common across all GPU APIs, and then dispatch
 * to more API-specific functions via another set of function pointers
 * hidden inside the impl field.
 */
struct halide_device_interface_t {
    int (*device_malloc)(void *user_context, struct halide_buffer_t *buf,
                         const struct halide_device_interface_t *device_interface);
    int (*device_free)(void *user_context, struct halide_buffer_t *buf);
    int (*device_sync)(void *user_context, struct halide_buffer_t *buf);
    void (*device_release)(void *user_context,
                          const struct halide_device_interface_t *device_interface);
    int (*copy_to_host)(void *user_context, struct halide_buffer_t *buf);
    int (*copy_to_device)(void *user_context, struct halide_buffer_t *buf,
                          const struct halide_device_interface_t *device_interface);
    int (*device_and_host_malloc)(void *user_context, struct halide_buffer_t *buf,
                                  const struct halide_device_interface_t *device_interface);
    int (*device_and_host_free)(void *user_context, struct halide_buffer_t *buf);
    int (*buffer_copy)(void *user_context, struct halide_buffer_t *src,
                       const struct halide_device_interface_t *dst_device_interface, struct halide_buffer_t *dst);
    int (*device_crop)(void *user_context, const struct halide_buffer_t *src,
                       struct halide_buffer_t *dst);
    int (*device_slice)(void *user_context, const struct halide_buffer_t *src,
                        int slice_dim, int slice_pos, struct halide_buffer_t *dst);
    int (*device_release_crop)(void *user_context, struct halide_buffer_t *buf);
    int (*wrap_native)(void *user_context, struct halide_buffer_t *buf, uint64_t handle,
                       const struct halide_device_interface_t *device_interface);
    int (*detach_native)(void *user_context, struct halide_buffer_t *buf);
    int (*compute_capability)(void *user_context, int *major, int *minor);
    const struct halide_device_interface_impl_t *impl;
};

/** Release all data associated with the given device interface, in
 * particular all resources (memory, texture, context handles)
 * allocated by Halide. Must be called explicitly when using AOT
 * compilation. This is *not* thread-safe with respect to actively
 * running Halide code. Ensure all pipelines are finished before
 * calling this. */
extern void halide_device_release(void *user_context,
                                  const struct halide_device_interface_t *device_interface);

/** Copy image data from device memory to host memory. This must be called
 * explicitly to copy back the results of a GPU-based filter. */
extern int halide_copy_to_host(void *user_context, struct halide_buffer_t *buf);

/** Copy image data from host memory to device memory. This should not
 * be called directly; Halide handles copying to the device
 * automatically.  If interface is NULL and the buf has a non-zero dev
 * field, the device associated with the dev handle will be
 * used. Otherwise if the dev field is 0 and interface is NULL, an
 * error is returned. */
extern int halide_copy_to_device(void *user_context, struct halide_buffer_t *buf,
                                 const struct halide_device_interface_t *device_interface);

/** Copy data from one buffer to another. The buffers may have
 * different shapes and sizes, but the destination buffer's shape must
 * be contained within the source buffer's shape. That is, for each
 * dimension, the min on the destination buffer must be greater than
 * or equal to the min on the source buffer, and min+extent on the
 * destination buffer must be less that or equal to min+extent on the
 * source buffer. The source data is pulled from either device or
 * host memory on the source, depending on the dirty flags. host is
 * preferred if both are valid. The dst_device_interface parameter
 * controls the destination memory space. NULL means host memory. */
extern int halide_buffer_copy(void *user_context, struct halide_buffer_t *src,
                              const struct halide_device_interface_t *dst_device_interface,
                              struct halide_buffer_t *dst);

/** Give the destination buffer a device allocation which is an alias
 * for the same coordinate range in the source buffer. Modifies the
 * device, device_interface, and the device_dirty flag only. Only
 * supported by some device APIs (others will return
 * halide_error_code_device_crop_unsupported). Call
 * halide_device_release_crop instead of halide_device_free to clean
 * up resources associated with the cropped view. Do not free the
 * device allocation on the source buffer while the destination buffer
 * still lives. Note that the two buffers do not share dirty flags, so
 * care must be taken to update them together as needed. Note that src
 * and dst are required to have the same number of dimensions.
 *
 * Note also that (in theory) device interfaces which support cropping may
 * still not support cropping a crop (instead, create a new crop of the parent
 * buffer); in practice, no known implementation has this limitation, although
 * it is possible that some future implementations may require it. */
extern int halide_device_crop(void *user_context,
                              const struct halide_buffer_t *src,
                              struct halide_buffer_t *dst);

/** Give the destination buffer a device allocation which is an alias
 * for a similar coordinate range in the source buffer, but with one dimension
 * sliced away in the dst. Modifies the device, device_interface, and the
 * device_dirty flag only. Only supported by some device APIs (others will return
 * halide_error_code_device_crop_unsupported). Call
 * halide_device_release_crop instead of halide_device_free to clean
 * up resources associated with the sliced view. Do not free the
 * device allocation on the source buffer while the destination buffer
 * still lives. Note that the two buffers do not share dirty flags, so
 * care must be taken to update them together as needed. Note that the dst buffer
 * must have exactly one fewer dimension than the src buffer, and that slice_dim
 * and slice_pos must be valid within src. */
extern int halide_device_slice(void *user_context,
                               const struct halide_buffer_t *src,
                               int slice_dim, int slice_pos,
                               struct halide_buffer_t *dst);

/** Release any resources associated with a cropped/sliced view of another
 * buffer. */
extern int halide_device_release_crop(void *user_context,
                                      struct halide_buffer_t *buf);

/** Wait for current GPU operations to complete. Calling this explicitly
 * should rarely be necessary, except maybe for profiling. */
extern int halide_device_sync(void *user_context, struct halide_buffer_t *buf);

/** Allocate device memory to back a halide_buffer_t. */
extern int halide_device_malloc(void *user_context, struct halide_buffer_t *buf,
                                const struct halide_device_interface_t *device_interface);

/** Free device memory. */
extern int halide_device_free(void *user_context, struct halide_buffer_t *buf);

/** Wrap or detach a native device handle, setting the device field
 * and device_interface field as appropriate for the given GPU
 * API. The meaning of the opaque handle is specific to the device
 * interface, so if you know the device interface in use, call the
 * more specific functions in the runtime headers for your specific
 * device API instead (e.g. HalideRuntimeCuda.h). */
// @{
extern int halide_device_wrap_native(void *user_context,
                                     struct halide_buffer_t *buf,
                                     uint64_t handle,
                                     const struct halide_device_interface_t *device_interface);
extern int halide_device_detach_native(void *user_context, struct halide_buffer_t *buf);
// @}

/** Versions of the above functions that accept legacy buffer_t structs. */
// @{
extern int halide_copy_to_host_legacy(void *user_context, struct buffer_t *buf);
extern int halide_copy_to_device_legacy(void *user_context, struct buffer_t *buf,
                                 const struct halide_device_interface_t *device_interface);
extern int halide_device_sync_legacy(void *user_context, struct buffer_t *buf);
extern int halide_device_malloc_legacy(void *user_context, struct buffer_t *buf,
                                const struct halide_device_interface_t *device_interface);
extern int halide_device_free_legacy(void *user_context, struct buffer_t *buf);
// @}

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
 *  data. The last argument is a list if halide_buffer_t pointers which
 *  represents the outputs of the memoized Func. If the Func does not
 *  return a Tuple, there will only be one halide_buffer_t in the list. The
 *  tuple_count parameters determines the length of the list.
 *
 * The return values are:
 * -1: Signals an error.
 *  0: Success and cache hit.
 *  1: Success and cache miss.
 */
extern int halide_memoization_cache_lookup(void *user_context, const uint8_t *cache_key, int32_t size,
                                           struct halide_buffer_t *realized_bounds,
                                           int32_t tuple_count, struct halide_buffer_t **tuple_buffers);

/** Given a cache key for a memoized result, currently constructed
 *  from the Func name and top-level Func name plus the arguments of
 *  the computation, store the result in the cache for futre access by
 *  halide_memoization_cache_lookup. (The internals of the cache key
 *  should be considered opaque by this function.) Data is copied out
 *  from the inputs and inputs are unmodified. The last argument is a
 *  list if halide_buffer_t pointers which represents the outputs of the
 *  memoized Func. If the Func does not return a Tuple, there will
 *  only be one halide_buffer_t in the list. The tuple_count parameters
 *  determines the length of the list.
 *
 * If there is a memory allocation failure, the store does not store
 * the data into the cache.
 */
extern int halide_memoization_cache_store(void *user_context, const uint8_t *cache_key, int32_t size,
                                          struct halide_buffer_t *realized_bounds,
                                          int32_t tuple_count,
                                          struct halide_buffer_t **tuple_buffers);

/** If halide_memoization_cache_lookup succeeds,
 * halide_memoization_cache_release must be called to signal the
 * storage is no longer being used by the caller. It will be passed
 * the host pointer of one the buffers returned by
 * halide_memoization_cache_lookup. That is
 * halide_memoization_cache_release will be called multiple times for
 * the case where halide_memoization_cache_lookup is handling multiple
 * buffers.  (This corresponds to memoizing a Tuple in Halide.) Note
 * that the host pointer must be sufficient to get to all information
 * the relase operation needs. The default Halide cache impleemntation
 * accomplishes this by storing extra data before the start of the user
 * modifiable host storage.
 *
 * This call is like free and does not have a failure return.
  */
extern void halide_memoization_cache_release(void *user_context, void *host);

/** Free all memory and resources associated with the memoization cache.
 * Must be called at a time when no other threads are accessing the cache.
 */
extern void halide_memoization_cache_cleanup();

/** Create a unique file with a name of the form prefixXXXXXsuffix in an arbitrary
 * (but writable) directory; this is typically $TMP or /tmp, but the specific
 * location is not guaranteed. (Note that the exact form of the file name
 * may vary; in particular, the suffix may be ignored on non-Posix systems.)
 * The file is created (but not opened), thus this can be called from
 * different threads (or processes, e.g. when building with parallel make)
 * without risking collision. Note that the caller is always responsible
 * for deleting this file. Returns nonzero value if an error occurs.
 */
extern int halide_create_temp_file(void *user_context,
  const char *prefix, const char *suffix,
  char *path_buf, size_t path_buf_size);

/** Annotate that a given range of memory has been initialized;
 * only used when Target::MSAN is enabled.
 *
 * The default implementation uses the LLVM-provided AnnotateMemoryIsInitialized() function.
 */
extern void halide_msan_annotate_memory_is_initialized(void *user_context, const void *ptr, uint64_t len);

/** Mark the data pointed to by the buffer_t as initialized (but *not* the buffer_t itself),
 * using halide_msan_annotate_memory_is_initialized() for marking.
 *
 * The default implementation takes pains to only mark the active memory ranges
 * (skipping padding), and sorting into ranges to always mark the smallest number of
 * ranges, in monotonically increasing memory order.
 *
 * Most client code should never need to replace the default implementation.
 */
extern void halide_msan_annotate_buffer_is_initialized(void *user_context, struct halide_buffer_t *buffer);
extern void halide_msan_annotate_buffer_is_initialized_as_destructor(void *user_context, void *buffer);

/** The error codes that may be returned by a Halide pipeline. */
enum halide_error_code_t {
    /** There was no error. This is the value returned by Halide on success. */
    halide_error_code_success = 0,

    /** An uncategorized error occurred. Refer to the string passed to halide_error. */
    halide_error_code_generic_error = -1,

    /** A Func was given an explicit bound via Func::bound, but this
     * was not large enough to encompass the region that is used of
     * the Func by the rest of the pipeline. */
    halide_error_code_explicit_bounds_too_small = -2,

    /** The elem_size field of a halide_buffer_t does not match the size in
     * bytes of the type of that ImageParam. Probable type mismatch. */
    halide_error_code_bad_type = -3,

    /** A pipeline would access memory outside of the halide_buffer_t passed
     * in. */
    halide_error_code_access_out_of_bounds = -4,

    /** A halide_buffer_t was given that spans more than 2GB of memory. */
    halide_error_code_buffer_allocation_too_large = -5,

    /** A halide_buffer_t was given with extents that multiply to a number
     * greater than 2^31-1 */
    halide_error_code_buffer_extents_too_large = -6,

    /** Applying explicit constraints on the size of an input or
     * output buffer shrank the size of that buffer below what will be
     * accessed by the pipeline. */
    halide_error_code_constraints_make_required_region_smaller = -7,

    /** A constraint on a size or stride of an input or output buffer
     * was not met by the halide_buffer_t passed in. */
    halide_error_code_constraint_violated = -8,

    /** A scalar parameter passed in was smaller than its minimum
     * declared value. */
    halide_error_code_param_too_small = -9,

    /** A scalar parameter passed in was greater than its minimum
     * declared value. */
    halide_error_code_param_too_large = -10,

    /** A call to halide_malloc returned NULL. */
    halide_error_code_out_of_memory = -11,

    /** A halide_buffer_t pointer passed in was NULL. */
    halide_error_code_buffer_argument_is_null = -12,

    /** debug_to_file failed to open or write to the specified
     * file. */
    halide_error_code_debug_to_file_failed = -13,

    /** The Halide runtime encountered an error while trying to copy
     * from device to host. Turn on -debug in your target string to
     * see more details. */
    halide_error_code_copy_to_host_failed = -14,

    /** The Halide runtime encountered an error while trying to copy
     * from host to device. Turn on -debug in your target string to
     * see more details. */
    halide_error_code_copy_to_device_failed = -15,

    /** The Halide runtime encountered an error while trying to
     * allocate memory on device. Turn on -debug in your target string
     * to see more details. */
    halide_error_code_device_malloc_failed = -16,

    /** The Halide runtime encountered an error while trying to
     * synchronize with a device. Turn on -debug in your target string
     * to see more details. */
    halide_error_code_device_sync_failed = -17,

    /** The Halide runtime encountered an error while trying to free a
     * device allocation. Turn on -debug in your target string to see
     * more details. */
    halide_error_code_device_free_failed = -18,

    /** Buffer has a non-zero device but no device interface, which
     * violates a Halide invariant. */
    halide_error_code_no_device_interface = -19,

    /** An error occurred when attempting to initialize the Matlab
     * runtime. */
    halide_error_code_matlab_init_failed = -20,

    /** The type of an mxArray did not match the expected type. */
    halide_error_code_matlab_bad_param_type = -21,

    /** There is a bug in the Halide compiler. */
    halide_error_code_internal_error = -22,

    /** The Halide runtime encountered an error while trying to launch
     * a GPU kernel. Turn on -debug in your target string to see more
     * details. */
    halide_error_code_device_run_failed = -23,

    /** The Halide runtime encountered a host pointer that violated
     * the alignment set for it by way of a call to
     * set_host_alignment */
    halide_error_code_unaligned_host_ptr = -24,

    /** A fold_storage directive was used on a dimension that is not
     * accessed in a monotonically increasing or decreasing fashion. */
    halide_error_code_bad_fold = -25,

    /** A fold_storage directive was used with a fold factor that was
     * too small to store all the values of a producer needed by the
     * consumer. */
    halide_error_code_fold_factor_too_small = -26,

    /** User-specified require() expression was not satisfied. */
    halide_error_code_requirement_failed = -27,

    /** At least one of the buffer's extents are negative. */
    halide_error_code_buffer_extents_negative = -28,

    /** A compiled pipeline was passed the old deprecated buffer_t
     * struct, and it could not be upgraded to a halide_buffer_t. */
    halide_error_code_failed_to_upgrade_buffer_t = -29,

    /** A compiled pipeline was passed the old deprecated buffer_t
     * struct in bounds inference mode, but the returned information
     * can't be expressed in the old buffer_t. */
    halide_error_code_failed_to_downgrade_buffer_t = -30,

    /** A specialize_fail() schedule branch was selected at runtime. */
    halide_error_code_specialize_fail = -31,

    /** The Halide runtime encountered an error while trying to wrap a
     * native device handle.  Turn on -debug in your target string to
     * see more details. */
    halide_error_code_device_wrap_native_failed = -32,

    /** The Halide runtime encountered an error while trying to detach
     * a native device handle.  Turn on -debug in your target string
     * to see more details. */
    halide_error_code_device_detach_native_failed = -33,

    /** The host field on an input or output was null, the device
     * field was not zero, and the pipeline tries to use the buffer on
     * the host. You may be passing a GPU-only buffer to a pipeline
     * which is scheduled to use it on the CPU. */
    halide_error_code_host_is_null = -34,

    /** A folded buffer was passed to an extern stage, but the region
     * touched wraps around the fold boundary. */
    halide_error_code_bad_extern_fold = -35,

    /** Buffer has a non-null device_interface but device is 0, which
     * violates a Halide invariant. */
    halide_error_code_device_interface_no_device= -36,

    /** Buffer has both host and device dirty bits set, which violates
     * a Halide invariant. */
    halide_error_code_host_and_device_dirty = -37,

    /** The halide_buffer_t * passed to a halide runtime routine is
     * nullptr and this is not allowed. */
    halide_error_code_buffer_is_null = -38,

    /** The Halide runtime encountered an error while trying to copy
     * from one buffer to another. Turn on -debug in your target
     * string to see more details. */
    halide_error_code_device_buffer_copy_failed = -39,

    /** Attempted to make cropped/sliced alias of a buffer with a device
     * field, but the device_interface does not support cropping. */
    halide_error_code_device_crop_unsupported = -40,

    /** Cropping/slicing a buffer failed for some other reason. Turn on -debug
     * in your target string. */
    halide_error_code_device_crop_failed = -41,

    /** An operation on a buffer required an allocation on a
     * particular device interface, but a device allocation already
     * existed on a different device interface. Free the old one
     * first. */
    halide_error_code_incompatible_device_interface = -42,

    /** The dimensions field of a halide_buffer_t does not match the dimensions of that ImageParam. */
    halide_error_code_bad_dimensions = -43,

    /** An expression that would perform an integer division or modulo
     * by zero was evaluated. */
    halide_error_code_integer_division_by_zero = -44,

};

/** Halide calls the functions below on various error conditions. The
 * default implementations construct an error message, call
 * halide_error, then return the matching error code above. On
 * platforms that support weak linking, you can override these to
 * catch the errors individually. */

/** A call into an extern stage for the purposes of bounds inference
 * failed. Returns the error code given by the extern stage. */
extern int halide_error_bounds_inference_call_failed(void *user_context, const char *extern_stage_name, int result);

/** A call to an extern stage failed. Returned the error code given by
 * the extern stage. */
extern int halide_error_extern_stage_failed(void *user_context, const char *extern_stage_name, int result);

/** Various other error conditions. See the enum above for a
 * description of each. */
// @{
extern int halide_error_explicit_bounds_too_small(void *user_context, const char *func_name, const char *var_name,
                                                      int min_bound, int max_bound, int min_required, int max_required);
extern int halide_error_bad_type(void *user_context, const char *func_name,
                                 uint8_t code_given, uint8_t correct_code,
                                 uint8_t bits_given, uint8_t correct_bits,
                                 uint16_t lanes_given, uint16_t correct_lanes);
extern int halide_error_bad_dimensions(void *user_context, const char *func_name,
                                       int32_t dimensions_given, int32_t correct_dimensions);
extern int halide_error_access_out_of_bounds(void *user_context, const char *func_name,
                                             int dimension, int min_touched, int max_touched,
                                             int min_valid, int max_valid);
extern int halide_error_buffer_allocation_too_large(void *user_context, const char *buffer_name,
                                                    uint64_t allocation_size, uint64_t max_size);
extern int halide_error_buffer_extents_negative(void *user_context, const char *buffer_name, int dimension, int extent);
extern int halide_error_buffer_extents_too_large(void *user_context, const char *buffer_name,
                                                 int64_t actual_size, int64_t max_size);
extern int halide_error_constraints_make_required_region_smaller(void *user_context, const char *buffer_name,
                                                                 int dimension,
                                                                 int constrained_min, int constrained_extent,
                                                                 int required_min, int required_extent);
extern int halide_error_constraint_violated(void *user_context, const char *var, int val,
                                            const char *constrained_var, int constrained_val);
extern int halide_error_param_too_small_i64(void *user_context, const char *param_name,
                                            int64_t val, int64_t min_val);
extern int halide_error_param_too_small_u64(void *user_context, const char *param_name,
                                            uint64_t val, uint64_t min_val);
extern int halide_error_param_too_small_f64(void *user_context, const char *param_name,
                                            double val, double min_val);
extern int halide_error_param_too_large_i64(void *user_context, const char *param_name,
                                            int64_t val, int64_t max_val);
extern int halide_error_param_too_large_u64(void *user_context, const char *param_name,
                                            uint64_t val, uint64_t max_val);
extern int halide_error_param_too_large_f64(void *user_context, const char *param_name,
                                            double val, double max_val);
extern int halide_error_out_of_memory(void *user_context);
extern int halide_error_buffer_argument_is_null(void *user_context, const char *buffer_name);
extern int halide_error_debug_to_file_failed(void *user_context, const char *func,
                                             const char *filename, int error_code);
extern int halide_error_unaligned_host_ptr(void *user_context, const char *func_name, int alignment);
extern int halide_error_host_is_null(void *user_context, const char *func_name);
extern int halide_error_failed_to_upgrade_buffer_t(void *user_context,
                                                   const char *input_name,
                                                   const char *reason);
extern int halide_error_failed_to_downgrade_buffer_t(void *user_context,
                                                     const char *input_name,
                                                     const char *reason);
extern int halide_error_bad_fold(void *user_context, const char *func_name, const char *var_name,
                                 const char *loop_name);
extern int halide_error_bad_extern_fold(void *user_context, const char *func_name,
                                        int dim, int min, int extent, int valid_min, int fold_factor);

extern int halide_error_fold_factor_too_small(void *user_context, const char *func_name, const char *var_name,
                                              int fold_factor, const char *loop_name, int required_extent);
extern int halide_error_requirement_failed(void *user_context, const char *condition, const char *message);
extern int halide_error_specialize_fail(void *user_context, const char *message);
extern int halide_error_no_device_interface(void *user_context);
extern int halide_error_device_interface_no_device(void *user_context);
extern int halide_error_host_and_device_dirty(void *user_context);
extern int halide_error_buffer_is_null(void *user_context, const char *routine);
extern int halide_error_integer_division_by_zero(void *user_context);
// @}

/** Optional features a compilation Target can have.
 */
typedef enum halide_target_feature_t {
    halide_target_feature_jit = 0,  ///< Generate code that will run immediately inside the calling process.
    halide_target_feature_debug = 1,  ///< Turn on debug info and output for runtime code.
    halide_target_feature_no_asserts = 2,  ///< Disable all runtime checks, for slightly tighter code.
    halide_target_feature_no_bounds_query = 3, ///< Disable the bounds querying functionality.

    halide_target_feature_sse41 = 4,  ///< Use SSE 4.1 and earlier instructions. Only relevant on x86.
    halide_target_feature_avx = 5,  ///< Use AVX 1 instructions. Only relevant on x86.
    halide_target_feature_avx2 = 6,  ///< Use AVX 2 instructions. Only relevant on x86.
    halide_target_feature_fma = 7,  ///< Enable x86 FMA instruction
    halide_target_feature_fma4 = 8,  ///< Enable x86 (AMD) FMA4 instruction set
    halide_target_feature_f16c = 9,  ///< Enable x86 16-bit float support

    halide_target_feature_armv7s = 10,  ///< Generate code for ARMv7s. Only relevant for 32-bit ARM.
    halide_target_feature_no_neon = 11,  ///< Avoid using NEON instructions. Only relevant for 32-bit ARM.

    halide_target_feature_vsx = 12,  ///< Use VSX instructions. Only relevant on POWERPC.
    halide_target_feature_power_arch_2_07 = 13,  ///< Use POWER ISA 2.07 new instructions. Only relevant on POWERPC.

    halide_target_feature_cuda = 14,  ///< Enable the CUDA runtime. Defaults to compute capability 2.0 (Fermi)
    halide_target_feature_cuda_capability30 = 15,  ///< Enable CUDA compute capability 3.0 (Kepler)
    halide_target_feature_cuda_capability32 = 16,  ///< Enable CUDA compute capability 3.2 (Tegra K1)
    halide_target_feature_cuda_capability35 = 17,  ///< Enable CUDA compute capability 3.5 (Kepler)
    halide_target_feature_cuda_capability50 = 18,  ///< Enable CUDA compute capability 5.0 (Maxwell)

    halide_target_feature_opencl = 19,  ///< Enable the OpenCL runtime.
    halide_target_feature_cl_doubles = 20,  ///< Enable double support on OpenCL targets

    halide_target_feature_opengl = 21,  ///< Enable the OpenGL runtime.
    halide_target_feature_openglcompute = 22, ///< Enable OpenGL Compute runtime.

    halide_target_feature_unused_23 = 23, ///< Unused. (Formerly: Enable the RenderScript runtime.)

    halide_target_feature_user_context = 24,  ///< Generated code takes a user_context pointer as first argument

    halide_target_feature_matlab = 25,  ///< Generate a mexFunction compatible with Matlab mex libraries. See tools/mex_halide.m.

    halide_target_feature_profile = 26, ///< Launch a sampling profiler alongside the Halide pipeline that monitors and reports the runtime used by each Func
    halide_target_feature_no_runtime = 27, ///< Do not include a copy of the Halide runtime in any generated object file or assembly

    halide_target_feature_metal = 28, ///< Enable the (Apple) Metal runtime.
    halide_target_feature_mingw = 29, ///< For Windows compile to MinGW toolset rather then Visual Studio

    halide_target_feature_c_plus_plus_mangling = 30, ///< Generate C++ mangled names for result function, et al

    halide_target_feature_large_buffers = 31, ///< Enable 64-bit buffer indexing to support buffers > 2GB. Ignored if bits != 64.

    halide_target_feature_hvx_64 = 32, ///< Enable HVX 64 byte mode.
    halide_target_feature_hvx_128 = 33, ///< Enable HVX 128 byte mode.
    halide_target_feature_hvx_v62 = 34, ///< Enable Hexagon v62 architecture.
    halide_target_feature_fuzz_float_stores = 35, ///< On every floating point store, set the last bit of the mantissa to zero. Pipelines for which the output is very different with this feature enabled may also produce very different output on different processors.
    halide_target_feature_soft_float_abi = 36, ///< Enable soft float ABI. This only enables the soft float ABI calling convention, which does not necessarily use soft floats.
    halide_target_feature_msan = 37, ///< Enable hooks for MSAN support.
    halide_target_feature_avx512 = 38, ///< Enable the base AVX512 subset supported by all AVX512 architectures. The specific feature sets are AVX-512F and AVX512-CD. See https://en.wikipedia.org/wiki/AVX-512 for a description of each AVX subset.
    halide_target_feature_avx512_knl = 39, ///< Enable the AVX512 features supported by Knight's Landing chips, such as the Xeon Phi x200. This includes the base AVX512 set, and also AVX512-CD and AVX512-ER.
    halide_target_feature_avx512_skylake = 40, ///< Enable the AVX512 features supported by Skylake Xeon server processors. This adds AVX512-VL, AVX512-BW, and AVX512-DQ to the base set. The main difference from the base AVX512 set is better support for small integer ops. Note that this does not include the Knight's Landing features. Note also that these features are not available on Skylake desktop and mobile processors.
    halide_target_feature_avx512_cannonlake = 41, ///< Enable the AVX512 features expected to be supported by future Cannonlake processors. This includes all of the Skylake features, plus AVX512-IFMA and AVX512-VBMI.
    halide_target_feature_hvx_use_shared_object = 42, ///< Deprecated
    halide_target_feature_trace_loads = 43, ///< Trace all loads done by the pipeline. Equivalent to calling Func::trace_loads on every non-inlined Func.
    halide_target_feature_trace_stores = 44, ///< Trace all stores done by the pipeline. Equivalent to calling Func::trace_stores on every non-inlined Func.
    halide_target_feature_trace_realizations = 45, ///< Trace all realizations done by the pipeline. Equivalent to calling Func::trace_realizations on every non-inlined Func.
    halide_target_feature_cuda_capability61 = 46,  ///< Enable CUDA compute capability 6.1 (Pascal)
    halide_target_feature_hvx_v65 = 47, ///< Enable Hexagon v65 architecture.
    halide_target_feature_hvx_v66 = 48, ///< Enable Hexagon v66 architecture.
    halide_target_feature_cl_half = 49,  ///< Enable half support on OpenCL targets
    halide_target_feature_strict_float = 50, ///< Turn off all non-IEEE floating-point optimization. Currently applies only to LLVM targets.
    halide_target_feature_legacy_buffer_wrappers = 51,  ///< Emit legacy wrapper code for buffer_t (vs halide_buffer_t) when AOT-compiled.
    halide_target_feature_tsan = 52, ///< Enable hooks for TSAN support.
    halide_target_feature_asan = 53, ///< Enable hooks for ASAN support.
    halide_target_feature_d3d12compute = 54, ///< Enable Direct3D 12 Compute runtime.
    halide_target_feature_check_unsafe_promises = 55, ///< Insert assertions for promises.
    halide_target_feature_hexagon_dma = 56, ///< Enable Hexagon DMA buffers.
    halide_target_feature_embed_bitcode = 57,  ///< Emulate clang -fembed-bitcode flag.
    halide_target_feature_coreir = 58, ///< Enable output to CoreIR.
    halide_target_feature_coreir_valid = 59, ///< Enable output signal valid for CoreIR.
    halide_target_feature_hls = 60, ///< Enable output to HLS.
    halide_target_feature_end = 61 ///< A sentinel. Every target is considered to have this feature, and setting this feature does nothing.
} halide_target_feature_t;

/** This function is called internally by Halide in some situations to determine
 * if the current execution environment can support the given set of
 * halide_target_feature_t flags. The implementation must do the following:
 *
 * -- If there are flags set in features that the function knows *cannot* be supported, return 0.
 * -- Otherwise, return 1.
 * -- Note that any flags set in features that the function doesn't know how to test should be ignored;
 * this implies that a return value of 1 means "not known to be bad" rather than "known to be good".
 *
 * In other words: a return value of 0 means "It is not safe to use code compiled with these features",
 * while a return value of 1 means "It is not obviously unsafe to use code compiled with these features".
 *
 * The default implementation simply calls halide_default_can_use_target_features.
 *
 * Note that `features` points to an array of `count` uint64_t; this array must contain enough
 * bits to represent all the currently known features. Any excess bits must be set to zero.
 */
// @{
extern int halide_can_use_target_features(int count, const uint64_t *features);
typedef int (*halide_can_use_target_features_t)(int count, const uint64_t *features);
extern halide_can_use_target_features_t halide_set_custom_can_use_target_features(halide_can_use_target_features_t);
// @}

/**
 * This is the default implementation of halide_can_use_target_features; it is provided
 * for convenience of user code that may wish to extend halide_can_use_target_features
 * but continue providing existing support, e.g.
 *
 *     int halide_can_use_target_features(int count, const uint64_t *features) {
 *          if (features[halide_target_somefeature >> 6] & (1LL << (halide_target_somefeature & 63))) {
 *              if (!can_use_somefeature()) {
 *                  return 0;
 *              }
 *          }
 *          return halide_default_can_use_target_features(count, features);
 *     }
 */
extern int halide_default_can_use_target_features(int count, const uint64_t *features);


typedef struct halide_dimension_t {
    int32_t min, extent, stride;

    // Per-dimension flags. None are defined yet (This is reserved for future use).
    uint32_t flags;

#ifdef __cplusplus
    HALIDE_ALWAYS_INLINE halide_dimension_t() : min(0), extent(0), stride(0), flags(0) {}
    HALIDE_ALWAYS_INLINE halide_dimension_t(int32_t m, int32_t e, int32_t s, uint32_t f = 0) :
        min(m), extent(e), stride(s), flags(f) {}

    HALIDE_ALWAYS_INLINE bool operator==(const halide_dimension_t &other) const {
        return (min == other.min) &&
            (extent == other.extent) &&
            (stride == other.stride) &&
            (flags == other.flags);
    }

    HALIDE_ALWAYS_INLINE bool operator!=(const halide_dimension_t &other) const {
        return !(*this == other);
    }
#endif
} halide_dimension_t;

#ifdef __cplusplus
} // extern "C"
#endif

typedef enum {halide_buffer_flag_host_dirty = 1,
              halide_buffer_flag_device_dirty = 2} halide_buffer_flags;

/**
 * The raw representation of an image passed around by generated
 * Halide code. It includes some stuff to track whether the image is
 * not actually in main memory, but instead on a device (like a
 * GPU). For a more convenient C++ wrapper, use Halide::Buffer<T>. */
typedef struct halide_buffer_t {
    /** A device-handle for e.g. GPU memory used to back this buffer. */
    uint64_t device;

    /** The interface used to interpret the above handle. */
    const struct halide_device_interface_t *device_interface;

    /** A pointer to the start of the data in main memory. In terms of
     * the Halide coordinate system, this is the address of the min
     * coordinates (defined below). */
    uint8_t* host;

    /** flags with various meanings. */
    uint64_t flags;

    /** The type of each buffer element. */
    struct halide_type_t type;

    /** The dimensionality of the buffer. */
    int32_t dimensions;

    /** The shape of the buffer. Halide does not own this array - you
     * must manage the memory for it yourself. */
    halide_dimension_t *dim;

    /** Pads the buffer up to a multiple of 8 bytes */
    void *padding;

#ifdef __cplusplus
    /** Convenience methods for accessing the flags */
    // @{
    HALIDE_ALWAYS_INLINE bool get_flag(halide_buffer_flags flag) const {
        return (flags & flag) != 0;
    }

    HALIDE_ALWAYS_INLINE void set_flag(halide_buffer_flags flag, bool value) {
        if (value) {
            flags |= flag;
        } else {
            flags &= ~flag;
        }
    }

    HALIDE_ALWAYS_INLINE bool host_dirty() const {
        return get_flag(halide_buffer_flag_host_dirty);
    }

    HALIDE_ALWAYS_INLINE bool device_dirty() const {
        return get_flag(halide_buffer_flag_device_dirty);
    }

    HALIDE_ALWAYS_INLINE void set_host_dirty(bool v = true) {
        set_flag(halide_buffer_flag_host_dirty, v);
    }

    HALIDE_ALWAYS_INLINE void set_device_dirty(bool v = true) {
        set_flag(halide_buffer_flag_device_dirty, v);
    }
    // @}

    /** The total number of elements this buffer represents. Equal to
     * the product of the extents */
    HALIDE_ALWAYS_INLINE size_t number_of_elements() const {
        size_t s = 1;
        for (int i = 0; i < dimensions; i++) {
            s *= dim[i].extent;
        }
        return s;
    }

    /** A pointer to the element with the lowest address. If all
     * strides are positive, equal to the host pointer. */
    HALIDE_ALWAYS_INLINE uint8_t *begin() const {
        ptrdiff_t index = 0;
        for (int i = 0; i < dimensions; i++) {
            if (dim[i].stride < 0) {
                index += dim[i].stride * (dim[i].extent - 1);
            }
        }
        return host + index * type.bytes();
    }

    /** A pointer to one beyond the element with the highest address. */
    HALIDE_ALWAYS_INLINE uint8_t *end() const {
        ptrdiff_t index = 0;
        for (int i = 0; i < dimensions; i++) {
            if (dim[i].stride > 0) {
                index += dim[i].stride * (dim[i].extent - 1);
            }
        }
        index += 1;
        return host + index * type.bytes();
    }

    /** The total number of bytes spanned by the data in memory. */
    HALIDE_ALWAYS_INLINE size_t size_in_bytes() const {
        return (size_t)(end() - begin());
    }

    /** A pointer to the element at the given location. */
    HALIDE_ALWAYS_INLINE uint8_t *address_of(const int *pos) const {
        ptrdiff_t index = 0;
        for (int i = 0; i < dimensions; i++) {
            index += dim[i].stride * (pos[i] - dim[i].min);
        }
        return host + index * type.bytes();
    }

    /** Attempt to call device_sync for the buffer. If the buffer
     * has no device_interface (or no device_sync), this is a quiet no-op.
     * Calling this explicitly should rarely be necessary, except for profiling. */
    HALIDE_ALWAYS_INLINE int device_sync(void *ctx = NULL) {
        if (device_interface && device_interface->device_sync) {
            return device_interface->device_sync(ctx, this);
        }
        return 0;
    }

    /** Check if an input buffer passed extern stage is a querying
     * bounds. Compared to doing the host pointer check directly,
     * this both adds clarity to code and will facilitate moving to
     * another representation for bounds query arguments. */
    HALIDE_ALWAYS_INLINE bool is_bounds_query() const {
        return host == NULL && device == 0;
    }

#endif
} halide_buffer_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HALIDE_ATTRIBUTE_DEPRECATED
#ifdef HALIDE_ALLOW_DEPRECATED
#define HALIDE_ATTRIBUTE_DEPRECATED(x)
#else
#ifdef _MSC_VER
#define HALIDE_ATTRIBUTE_DEPRECATED(x) __declspec(deprecated(x))
#else
#define HALIDE_ATTRIBUTE_DEPRECATED(x) __attribute__((deprecated(x)))
#endif
#endif
#endif

/** The old buffer_t, included for compatibility with old code. Don't
 * use it. */
#ifndef BUFFER_T_DEFINED
#define BUFFER_T_DEFINED
typedef struct buffer_t {
    uint64_t dev;
    uint8_t* host;
    int32_t extent[4];
    int32_t stride[4];
    int32_t min[4];
    int32_t elem_size;
    HALIDE_ATTRIBUTE_ALIGN(1) bool host_dirty;
    HALIDE_ATTRIBUTE_ALIGN(1) bool dev_dirty;
    HALIDE_ATTRIBUTE_ALIGN(1) uint8_t _padding[10 - sizeof(void *)];
} buffer_t;
#endif // BUFFER_T_DEFINED

/** Copies host pointer, mins, extents, strides, and device state from
 * an old-style buffer_t into a new-style halide_buffer_t. If bounds_query_only is nonzero,
 * the copy is only done if the old_buf has null host and dev (ie, a bounds query is being
 * performed); otherwise new_buf is left untouched. (This is used for input buffers to avoid
 * benign data races.) The dimensions and type fields of the new buffer_t should already be
 * set. Returns an error code if the upgrade could not be performed. */
extern int halide_upgrade_buffer_t(void *user_context, const char *name,
                                   const buffer_t *old_buf, halide_buffer_t *new_buf,
                                   int bounds_query_only);

/** Copies the host pointer, mins, extents, strides, and device state
 * from a halide_buffer_t to a buffer_t. Also sets elem_size. Useful
 * for backporting the results of bounds inference. */
extern int halide_downgrade_buffer_t(void *user_context, const char *name,
                                     const halide_buffer_t *new_buf, buffer_t *old_buf);

/** Copies the dirty flags and device allocation state from a new
 * buffer_t back to a legacy buffer_t. */
extern int halide_downgrade_buffer_t_device_fields(void *user_context, const char *name,
                                                   const halide_buffer_t *new_buf, buffer_t *old_buf);

/** halide_scalar_value_t is a simple union able to represent all the well-known
 * scalar values in a filter argument. Note that it isn't tagged with a type;
 * you must ensure you know the proper type before accessing. Most user
 * code will never need to create instances of this struct; its primary use
 * is to hold def/min/max values in a halide_filter_argument_t. (Note that
 * this is conceptually just a union; it's wrapped in a struct to ensure
 * that it doesn't get anonymized by LLVM.)
 */
struct halide_scalar_value_t {
    union {
        bool b;
        int8_t i8;
        int16_t i16;
        int32_t i32;
        int64_t i64;
        uint8_t u8;
        uint16_t u16;
        uint32_t u32;
        uint64_t u64;
        float f32;
        double f64;
        void *handle;
    } u;
    #ifdef __cplusplus
    HALIDE_ALWAYS_INLINE halide_scalar_value_t() {u.u64 = 0;}
    #endif
};

enum halide_argument_kind_t {
    halide_argument_kind_input_scalar = 0,
    halide_argument_kind_input_buffer = 1,
    halide_argument_kind_output_buffer = 2
};

/*
    These structs must be robust across different compilers and settings; when
    modifying them, strive for the following rules:

    1) All fields are explicitly sized. I.e. must use int32_t and not "int"
    2) All fields must land on an alignment boundary that is the same as their size
    3) Explicit padding is added to make that so
    4) The sizeof the struct is padded out to a multiple of the largest natural size thing in the struct
    5) don't forget that 32 and 64 bit pointers are different sizes
*/

/**
 * halide_filter_argument_t is essentially a plain-C-struct equivalent to
 * Halide::Argument; most user code will never need to create one.
 */
struct halide_filter_argument_t {
    const char *name;       // name of the argument; will never be null or empty.
    int32_t kind;           // actually halide_argument_kind_t
    int32_t dimensions;     // always zero for scalar arguments
    struct halide_type_t type;
    // These pointers should always be null for buffer arguments,
    // and *may* be null for scalar arguments. (A null value means
    // there is no def/min/max specified for this argument.)
    const struct halide_scalar_value_t *def;
    const struct halide_scalar_value_t *min;
    const struct halide_scalar_value_t *max;
};

struct halide_filter_metadata_t {
    /** version of this metadata; currently always 0. */
    int32_t version;

    /** The number of entries in the arguments field. This is always >= 1. */
    int32_t num_arguments;

    /** An array of the filters input and output arguments; this will never be
     * null. The order of arguments is not guaranteed (input and output arguments
     * may come in any order); however, it is guaranteed that all arguments
     * will have a unique name within a given filter. */
    const struct halide_filter_argument_t* arguments;

    /** The Target for which the filter was compiled. This is always
     * a canonical Target string (ie a product of Target::to_string). */
    const char* target;

    /** The function name of the filter. */
    const char* name;
};

/** The functions below here are relevant for pipelines compiled with
 * the -profile target flag, which runs a sampling profiler thread
 * alongside the pipeline. */

/** Per-Func state tracked by the sampling profiler. */
struct halide_profiler_func_stats {
    /** Total time taken evaluating this Func (in nanoseconds). */
    uint64_t time;

    /** The current memory allocation of this Func. */
    uint64_t memory_current;

    /** The peak memory allocation of this Func. */
    uint64_t memory_peak;

    /** The total memory allocation of this Func. */
    uint64_t memory_total;

    /** The peak stack allocation of this Func's threads. */
    uint64_t stack_peak;

    /** The average number of thread pool worker threads active while computing this Func. */
    uint64_t active_threads_numerator, active_threads_denominator;

    /** The name of this Func. A global constant string. */
    const char *name;

    /** The total number of memory allocation of this Func. */
    int num_allocs;
};

/** Per-pipeline state tracked by the sampling profiler. These exist
 * in a linked list. */
struct halide_profiler_pipeline_stats {
    /** Total time spent inside this pipeline (in nanoseconds) */
    uint64_t time;

    /** The current memory allocation of funcs in this pipeline. */
    uint64_t memory_current;

    /** The peak memory allocation of funcs in this pipeline. */
    uint64_t memory_peak;

    /** The total memory allocation of funcs in this pipeline. */
    uint64_t memory_total;

    /** The average number of thread pool worker threads doing useful
     * work while computing this pipeline. */
    uint64_t active_threads_numerator, active_threads_denominator;

    /** The name of this pipeline. A global constant string. */
    const char *name;

    /** An array containing states for each Func in this pipeline. */
    struct halide_profiler_func_stats *funcs;

    /** The next pipeline_stats pointer. It's a void * because types
     * in the Halide runtime may not currently be recursive. */
    void *next;

    /** The number of funcs in this pipeline. */
    int num_funcs;

    /** An internal base id used to identify the funcs in this pipeline. */
    int first_func_id;

    /** The number of times this pipeline has been run. */
    int runs;

    /** The total number of samples taken inside of this pipeline. */
    int samples;

    /** The total number of memory allocation of funcs in this pipeline. */
    int num_allocs;
};

/** The global state of the profiler. */

struct halide_profiler_state {
    /** Guards access to the fields below. If not locked, the sampling
     * profiler thread is free to modify things below (including
     * reordering the linked list of pipeline stats). */
    struct halide_mutex lock;

    /** The amount of time the profiler thread sleeps between samples
     * in milliseconds. Defaults to 1 */
    int sleep_time;

    /** An internal id used for bookkeeping. */
    int first_free_id;

    /** The id of the current running Func. Set by the pipeline, read
     * periodically by the profiler thread. */
    int current_func;

    /** The number of threads currently doing work. */
    int active_threads;

    /** A linked list of stats gathered for each pipeline. */
    struct halide_profiler_pipeline_stats *pipelines;

    /** Retrieve remote profiler state. Used so that the sampling
     * profiler can follow along with execution that occurs elsewhere,
     * e.g. on a DSP. If null, it reads from the int above instead. */
    void (*get_remote_profiler_state)(int *func, int *active_workers);

    /** Sampling thread reference to be joined at shutdown. */
    struct halide_thread *sampling_thread;
};

/** Profiler func ids with special meanings. */
enum {
    /// current_func takes on this value when not inside Halide code
    halide_profiler_outside_of_halide = -1,
    /// Set current_func to this value to tell the profiling thread to
    /// halt. It will start up again next time you run a pipeline with
    /// profiling enabled.
    halide_profiler_please_stop = -2
};

/** Get a pointer to the global profiler state for programmatic
 * inspection. Lock it before using to pause the profiler. */
extern struct halide_profiler_state *halide_profiler_get_state();

/** Get a pointer to the pipeline state associated with pipeline_name.
 * This function grabs the global profiler state's lock on entry. */
extern struct halide_profiler_pipeline_stats *halide_profiler_get_pipeline_state(const char *pipeline_name);

/** Reset profiler state cheaply. May leave threads running or some
 * memory allocated but all accumluated statistics are reset.
 * WARNING: Do NOT call this method while any halide pipeline is
 * running; halide_profiler_memory_allocate/free and
 * halide_profiler_stack_peak_update update the profiler pipeline's
 * state without grabbing the global profiler state's lock. */
extern void halide_profiler_reset();

/** Reset all profiler state.
 * WARNING: Do NOT call this method while any halide pipeline is
 * running; halide_profiler_memory_allocate/free and
 * halide_profiler_stack_peak_update update the profiler pipeline's
 * state without grabbing the global profiler state's lock. */
void halide_profiler_shutdown();

/** Print out timing statistics for everything run since the last
 * reset. Also happens at process exit. */
extern void halide_profiler_report(void *user_context);

/// \name "Float16" functions
/// These functions operate of bits (``uint16_t``) representing a half
/// precision floating point number (IEEE-754 2008 binary16).
//{@

/** Read bits representing a half precision floating point number and return
 *  the float that represents the same value */
extern float halide_float16_bits_to_float(uint16_t);

/** Read bits representing a half precision floating point number and return
 *  the double that represents the same value */
extern double halide_float16_bits_to_double(uint16_t);

// TODO: Conversion functions to half

//@}

#ifdef __cplusplus
} // End extern "C"
#endif

#ifdef __cplusplus

namespace {
template<typename T> struct check_is_pointer;
template<typename T> struct check_is_pointer<T *> {};
}

/** Construct the halide equivalent of a C type */
template<typename T>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of() {
    // Create a compile-time error if T is not a pointer (without
    // using any includes - this code goes into the runtime).
    check_is_pointer<T> check;
    (void)check;
    return halide_type_t(halide_type_handle, 64);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<float>() {
    return halide_type_t(halide_type_float, 32);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<double>() {
    return halide_type_t(halide_type_float, 64);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<bool>() {
    return halide_type_t(halide_type_uint, 1);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<uint8_t>() {
    return halide_type_t(halide_type_uint, 8);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<uint16_t>() {
    return halide_type_t(halide_type_uint, 16);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<uint32_t>() {
    return halide_type_t(halide_type_uint, 32);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<uint64_t>() {
    return halide_type_t(halide_type_uint, 64);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<int8_t>() {
    return halide_type_t(halide_type_int, 8);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<int16_t>() {
    return halide_type_t(halide_type_int, 16);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<int32_t>() {
    return halide_type_t(halide_type_int, 32);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<int64_t>() {
    return halide_type_t(halide_type_int, 64);
}

#endif

#endif // HALIDE_HALIDERUNTIME_H

#ifdef COMPILING_HALIDE_RUNTIME
#include "HalideRuntime.h"
#define HALIDE_BUFFER_HELPER_ATTRS __attribute__((always_inline, weak))
#else
#define HALIDE_BUFFER_HELPER_ATTRS inline
#endif

// Structs are annoying to deal with from within Halide Stmts. These
// utility functions are for dealing with buffer_t in that
// context. They are not intended for use outside of Halide code, and
// not exposed in HalideRuntime.h. The symbols are private to the
// module and should be inlined and then stripped. This blob of code
// also gets copy-pasted into C outputs.

extern "C" {

HALIDE_BUFFER_HELPER_ATTRS
int _halide_buffer_get_dimensions(const halide_buffer_t *buf) {
    return buf->dimensions;
}

HALIDE_BUFFER_HELPER_ATTRS
uint8_t *_halide_buffer_get_host(const halide_buffer_t *buf) {
    return buf->host;
}

HALIDE_BUFFER_HELPER_ATTRS
uint64_t _halide_buffer_get_device(const halide_buffer_t *buf) {
    return buf->device;
}

HALIDE_BUFFER_HELPER_ATTRS
const struct halide_device_interface_t *_halide_buffer_get_device_interface(const halide_buffer_t *buf) {
    return buf->device_interface;
}

HALIDE_BUFFER_HELPER_ATTRS
int _halide_buffer_get_min(const halide_buffer_t *buf, int d) {
    return buf->dim[d].min;
}

HALIDE_BUFFER_HELPER_ATTRS
int _halide_buffer_get_max(const halide_buffer_t *buf, int d) {
    return buf->dim[d].min + buf->dim[d].extent - 1;
}

HALIDE_BUFFER_HELPER_ATTRS
int _halide_buffer_get_extent(const halide_buffer_t *buf, int d) {
    return buf->dim[d].extent;
}

HALIDE_BUFFER_HELPER_ATTRS
int _halide_buffer_get_stride(const halide_buffer_t *buf, int d) {
    return buf->dim[d].stride;
}

HALIDE_BUFFER_HELPER_ATTRS
int _halide_buffer_set_host_dirty(halide_buffer_t *buf, bool val) {
    buf->set_host_dirty(val);
    return 0;
}

HALIDE_BUFFER_HELPER_ATTRS
int _halide_buffer_set_device_dirty(halide_buffer_t *buf, bool val) {
    buf->set_device_dirty(val);
    return 0;
}

HALIDE_BUFFER_HELPER_ATTRS
bool _halide_buffer_get_host_dirty(const halide_buffer_t *buf) {
    return buf->host_dirty();
}

HALIDE_BUFFER_HELPER_ATTRS
bool _halide_buffer_get_device_dirty(const halide_buffer_t *buf) {
    return buf->device_dirty();
}

HALIDE_BUFFER_HELPER_ATTRS
halide_dimension_t *_halide_buffer_get_shape(halide_buffer_t *buf) {
    return buf->dim;
}

HALIDE_BUFFER_HELPER_ATTRS
bool _halide_buffer_is_bounds_query(const halide_buffer_t *buf) {
    return buf->host == NULL && buf->device == 0;
}

HALIDE_BUFFER_HELPER_ATTRS
uint8_t _halide_buffer_get_type_code(const halide_buffer_t *buf) {
    return buf->type.code;
}

HALIDE_BUFFER_HELPER_ATTRS
uint8_t _halide_buffer_get_type_bits(const halide_buffer_t *buf) {
    return buf->type.bits;
}

HALIDE_BUFFER_HELPER_ATTRS
uint16_t _halide_buffer_get_type_lanes(const halide_buffer_t *buf) {
    return buf->type.lanes;
}

HALIDE_BUFFER_HELPER_ATTRS
halide_buffer_t *_halide_buffer_init(halide_buffer_t *dst,
                                     halide_dimension_t *dst_shape,
                                     void *host,
                                     uint64_t device, halide_device_interface_t *device_interface,
                                     int type_code, int type_bits,
                                     int dimensions,
                                     halide_dimension_t *shape,
                                     uint64_t flags) {
    dst->host = (uint8_t *)host;
    dst->device = device;
    dst->device_interface = device_interface;
    dst->type.code = (halide_type_code_t)type_code;
    dst->type.bits = (uint8_t)type_bits;
    dst->type.lanes = 1;
    dst->dimensions = dimensions;
    dst->dim = dst_shape;
    if (shape != dst->dim) {
        for (int i = 0; i < dimensions; i++) {
            dst->dim[i] = shape[i];
        }
    }
    dst->flags = flags;
    return dst;
}

HALIDE_BUFFER_HELPER_ATTRS
halide_buffer_t *_halide_buffer_init_from_buffer(halide_buffer_t *dst,
                                                 halide_dimension_t *dst_shape,
                                                 const halide_buffer_t *src) {
    dst->host = src->host;
    dst->device = src->device;
    dst->device_interface = src->device_interface;
    dst->type = src->type;
    dst->dimensions = src->dimensions;
    dst->dim = dst_shape;
    dst->flags = src->flags;
    for (int i = 0; i < dst->dimensions; i++) {
        dst->dim[i] = src->dim[i];
    }
    return dst;
}

HALIDE_BUFFER_HELPER_ATTRS
halide_buffer_t *_halide_buffer_crop(void *user_context,
                                     halide_buffer_t *dst,
                                     halide_dimension_t *dst_shape,
                                     const halide_buffer_t *src,
                                     const int *min, const int *extent) {
    *dst = *src;
    dst->dim = dst_shape;
    int64_t offset = 0;
    for (int i = 0; i < dst->dimensions; i++) {
        dst->dim[i] = src->dim[i];
        dst->dim[i].min = min[i];
        dst->dim[i].extent = extent[i];
        offset += (min[i] - src->dim[i].min) * src->dim[i].stride;
    }
    if (dst->host) {
        dst->host += offset * src->type.bytes();
    }
    dst->device_interface = 0;
    dst->device = 0;
    if (src->device_interface) {
        src->device_interface->device_crop(user_context, src, dst);
    }
    return dst;
}


// Called on return from an extern stage where the output buffer was a
// crop of some other larger buffer. This happens for extern stages
// with distinct store_at/compute_at levels. Each call to the stage
// only fills in part of the buffer.
HALIDE_BUFFER_HELPER_ATTRS
int _halide_buffer_retire_crop_after_extern_stage(void *user_context,
                                                   void *obj) {
    halide_buffer_t **buffers = (halide_buffer_t **)obj;
    halide_buffer_t *crop = buffers[0];
    halide_buffer_t *parent = buffers[1];

    if (crop->device) {
        if (!parent->device) {
            // We have been given a device allocation by the extern
            // stage. It only represents the cropped region, so we
            // can't just give it to the parent.
            if (crop->device_dirty()) {
                crop->device_interface->copy_to_host(user_context, crop);
            }
            crop->device_interface->device_free(user_context, crop);
        } else {
            // We are a crop of an existing device allocation.
            if (crop->device_dirty()) {
                parent->set_device_dirty();
            }
            crop->device_interface->device_release_crop(user_context, crop);
        }
    }
    if (crop->host_dirty()) {
        parent->set_host_dirty();
    }
    return 0;
}

HALIDE_BUFFER_HELPER_ATTRS
int _halide_buffer_retire_crops_after_extern_stage(void *user_context,
                                                    void *obj) {
    halide_buffer_t **buffers = (halide_buffer_t **)obj;
    while (*buffers) {
        _halide_buffer_retire_crop_after_extern_stage(user_context, buffers);
        buffers += 2;
    }
    return 0;
}

HALIDE_BUFFER_HELPER_ATTRS
halide_buffer_t *_halide_buffer_set_bounds(halide_buffer_t *buf,
                                           int dim, int min, int extent) {
    buf->dim[dim].min = min;
    buf->dim[dim].extent = extent;
    return buf;
}

}

#undef HALIDE_BUFFER_HELPER_ATTRS

#ifndef HALIDE_FUNCTION_ATTRS
#define HALIDE_FUNCTION_ATTRS
#endif



#ifdef __cplusplus
extern "C" {
#endif

int conv(struct halide_buffer_t *_bias_dw_buffer, struct halide_buffer_t *_bias_pw_buffer, struct halide_buffer_t *_filter_dw_buffer, struct halide_buffer_t *_filter_pw_buffer, struct halide_buffer_t *_input_buffer, struct halide_buffer_t *_output_buffer) HALIDE_FUNCTION_ATTRS {
 void * const _ucon = nullptr;
 uint64_t _22 = (uint64_t)(_output_buffer);
 uint64_t _23 = (uint64_t)(0);
 bool _24 = _22 != _23;
 if (!_24)  {
  int32_t _25 = halide_error_buffer_argument_is_null(_ucon, "output");
  return _25;
 }
 uint64_t _26 = (uint64_t)(_input_buffer);
 uint64_t _27 = (uint64_t)(0);
 bool _28 = _26 != _27;
 if (!_28)  {
  int32_t _29 = halide_error_buffer_argument_is_null(_ucon, "input");
  return _29;
 }
 uint64_t _30 = (uint64_t)(_filter_pw_buffer);
 uint64_t _31 = (uint64_t)(0);
 bool _32 = _30 != _31;
 if (!_32)  {
  int32_t _33 = halide_error_buffer_argument_is_null(_ucon, "filter_pw");
  return _33;
 }
 uint64_t _34 = (uint64_t)(_filter_dw_buffer);
 uint64_t _35 = (uint64_t)(0);
 bool _36 = _34 != _35;
 if (!_36)  {
  int32_t _37 = halide_error_buffer_argument_is_null(_ucon, "filter_dw");
  return _37;
 }
 uint64_t _38 = (uint64_t)(_bias_pw_buffer);
 uint64_t _39 = (uint64_t)(0);
 bool _40 = _38 != _39;
 if (!_40)  {
  int32_t _41 = halide_error_buffer_argument_is_null(_ucon, "bias_pw");
  return _41;
 }
 uint64_t _42 = (uint64_t)(_bias_dw_buffer);
 uint64_t _43 = (uint64_t)(0);
 bool _44 = _42 != _43;
 if (!_44)  {
  int32_t _45 = halide_error_buffer_argument_is_null(_ucon, "bias_dw");
  return _45;
 }
 void *_46 = _halide_buffer_get_host(_bias_dw_buffer);
 void * _bias_dw = _46;
 uint8_t _47 = _halide_buffer_get_type_code(_bias_dw_buffer);
 uint8_t _48 = _halide_buffer_get_type_bits(_bias_dw_buffer);
 uint16_t _49 = _halide_buffer_get_type_lanes(_bias_dw_buffer);
 int32_t _50 = _halide_buffer_get_dimensions(_bias_dw_buffer);
 int32_t _51 = _halide_buffer_get_min(_bias_dw_buffer, 0);
 int32_t _52 = _halide_buffer_get_extent(_bias_dw_buffer, 0);
 int32_t _53 = _halide_buffer_get_stride(_bias_dw_buffer, 0);
 void *_54 = _halide_buffer_get_host(_bias_pw_buffer);
 void * _bias_pw = _54;
 uint8_t _55 = _halide_buffer_get_type_code(_bias_pw_buffer);
 uint8_t _56 = _halide_buffer_get_type_bits(_bias_pw_buffer);
 uint16_t _57 = _halide_buffer_get_type_lanes(_bias_pw_buffer);
 int32_t _58 = _halide_buffer_get_dimensions(_bias_pw_buffer);
 int32_t _59 = _halide_buffer_get_min(_bias_pw_buffer, 0);
 int32_t _60 = _halide_buffer_get_extent(_bias_pw_buffer, 0);
 int32_t _61 = _halide_buffer_get_stride(_bias_pw_buffer, 0);
 void *_62 = _halide_buffer_get_host(_filter_dw_buffer);
 void * _filter_dw = _62;
 uint8_t _63 = _halide_buffer_get_type_code(_filter_dw_buffer);
 uint8_t _64 = _halide_buffer_get_type_bits(_filter_dw_buffer);
 uint16_t _65 = _halide_buffer_get_type_lanes(_filter_dw_buffer);
 int32_t _66 = _halide_buffer_get_dimensions(_filter_dw_buffer);
 int32_t _67 = _halide_buffer_get_min(_filter_dw_buffer, 0);
 int32_t _68 = _halide_buffer_get_extent(_filter_dw_buffer, 0);
 int32_t _69 = _halide_buffer_get_stride(_filter_dw_buffer, 0);
 int32_t _70 = _halide_buffer_get_min(_filter_dw_buffer, 1);
 int32_t _71 = _halide_buffer_get_extent(_filter_dw_buffer, 1);
 int32_t _72 = _halide_buffer_get_stride(_filter_dw_buffer, 1);
 int32_t _73 = _halide_buffer_get_min(_filter_dw_buffer, 2);
 int32_t _74 = _halide_buffer_get_extent(_filter_dw_buffer, 2);
 int32_t _75 = _halide_buffer_get_stride(_filter_dw_buffer, 2);
 void *_76 = _halide_buffer_get_host(_filter_pw_buffer);
 void * _filter_pw = _76;
 uint8_t _77 = _halide_buffer_get_type_code(_filter_pw_buffer);
 uint8_t _78 = _halide_buffer_get_type_bits(_filter_pw_buffer);
 uint16_t _79 = _halide_buffer_get_type_lanes(_filter_pw_buffer);
 int32_t _80 = _halide_buffer_get_dimensions(_filter_pw_buffer);
 int32_t _81 = _halide_buffer_get_min(_filter_pw_buffer, 0);
 int32_t _82 = _halide_buffer_get_extent(_filter_pw_buffer, 0);
 int32_t _83 = _halide_buffer_get_stride(_filter_pw_buffer, 0);
 int32_t _84 = _halide_buffer_get_min(_filter_pw_buffer, 1);
 int32_t _85 = _halide_buffer_get_extent(_filter_pw_buffer, 1);
 int32_t _86 = _halide_buffer_get_stride(_filter_pw_buffer, 1);
 void *_87 = _halide_buffer_get_host(_input_buffer);
 void * _input = _87;
 uint8_t _88 = _halide_buffer_get_type_code(_input_buffer);
 uint8_t _89 = _halide_buffer_get_type_bits(_input_buffer);
 uint16_t _90 = _halide_buffer_get_type_lanes(_input_buffer);
 int32_t _91 = _halide_buffer_get_dimensions(_input_buffer);
 int32_t _92 = _halide_buffer_get_min(_input_buffer, 0);
 int32_t _93 = _halide_buffer_get_extent(_input_buffer, 0);
 int32_t _94 = _halide_buffer_get_stride(_input_buffer, 0);
 int32_t _95 = _halide_buffer_get_min(_input_buffer, 1);
 int32_t _96 = _halide_buffer_get_extent(_input_buffer, 1);
 int32_t _97 = _halide_buffer_get_stride(_input_buffer, 1);
 int32_t _98 = _halide_buffer_get_min(_input_buffer, 2);
 int32_t _99 = _halide_buffer_get_extent(_input_buffer, 2);
 int32_t _100 = _halide_buffer_get_stride(_input_buffer, 2);
 int32_t _101 = _halide_buffer_get_min(_input_buffer, 3);
 int32_t _102 = _halide_buffer_get_extent(_input_buffer, 3);
 int32_t _103 = _halide_buffer_get_stride(_input_buffer, 3);
 void *_104 = _halide_buffer_get_host(_output_buffer);
 void * _output = _104;
 uint8_t _105 = _halide_buffer_get_type_code(_output_buffer);
 uint8_t _106 = _halide_buffer_get_type_bits(_output_buffer);
 uint16_t _107 = _halide_buffer_get_type_lanes(_output_buffer);
 int32_t _108 = _halide_buffer_get_dimensions(_output_buffer);
 int32_t _109 = _halide_buffer_get_min(_output_buffer, 0);
 int32_t _110 = _halide_buffer_get_extent(_output_buffer, 0);
 int32_t _111 = _halide_buffer_get_stride(_output_buffer, 0);
 int32_t _112 = _halide_buffer_get_min(_output_buffer, 1);
 int32_t _113 = _halide_buffer_get_extent(_output_buffer, 1);
 int32_t _114 = _halide_buffer_get_stride(_output_buffer, 1);
 int32_t _115 = _halide_buffer_get_min(_output_buffer, 2);
 int32_t _116 = _halide_buffer_get_extent(_output_buffer, 2);
 int32_t _117 = _halide_buffer_get_stride(_output_buffer, 2);
 int32_t _118 = _halide_buffer_get_min(_output_buffer, 3);
 int32_t _119 = _halide_buffer_get_extent(_output_buffer, 3);
 int32_t _120 = _halide_buffer_get_stride(_output_buffer, 3);
 int32_t _121 = _110 + 2;
 int32_t _122 = _113 + 2;
 int32_t _123 = _121 * _122;
 int32_t _124 = _110 + -1;
 int32_t _125 = _124 >> 5;
 int32_t _126 = _125 * 32;
 int32_t _127 = _110 + -32;
 int32_t _128 = ::halide_cpp_min(_126, _127);
 int32_t _129 = ::halide_cpp_min(_110, 32);
 int32_t _130 = _128 - _129;
 int32_t _131 = _113 + -1;
 int32_t _132 = _131 >> 5;
 int32_t _133 = _132 * 32;
 int32_t _134 = _113 + -32;
 int32_t _135 = ::halide_cpp_min(_133, _134);
 int32_t _136 = ::halide_cpp_min(_113, 32);
 int32_t _137 = _135 - _136;
 int32_t _138 = _130 + 64;
 int32_t _139 = _137 + 64;
 int32_t _140 = _138 * _139;
 bool _141 = _halide_buffer_is_bounds_query(_bias_pw_buffer);
 bool _142 = !(_141);
 bool _143 = 0 <= _115;
 int32_t _144 = _116 + _115;
 bool _145 = _144 <= 32;
 bool _146 = _143 && _145;
 bool _147 = _142 || _146;
 if (!_147)  {
  int32_t _148 = _116 + _115;
  int32_t _149 = _148 + -1;
  int32_t _150 = halide_error_constraints_make_required_region_smaller(_ucon, "Input buffer bias_pw", 0, 0, 31, _115, _149);
  return _150;
 }
 bool _151 = _halide_buffer_is_bounds_query(_filter_pw_buffer);
 bool _152 = !(_151);
 bool _153 = 0 <= _115;
 int32_t _154 = _116 + -1;
 int32_t _155 = _154 >> 2;
 int32_t _156 = _155 * 4;
 int32_t _157 = _156 + _115;
 bool _158 = _157 <= 60;
 bool _159 = _153 && _158;
 bool _160 = _152 || _159;
 if (!_160)  {
  int32_t _161 = _116 + -1;
  int32_t _162 = _161 >> 2;
  int32_t _163 = _162 * 4;
  int32_t _164 = _163 + _115;
  int32_t _165 = _164 + 3;
  int32_t _166 = halide_error_constraints_make_required_region_smaller(_ucon, "Input buffer filter_pw", 1, 0, 63, _115, _165);
  return _166;
 }
 bool _167 = _halide_buffer_is_bounds_query(_bias_dw_buffer);
 if (_167)
 {
  struct halide_dimension_t *_168 = _halide_buffer_get_shape(_bias_dw_buffer);
  uint64_t _169 = (uint64_t)(0);
  void *_170 = (void *)(_169);
  struct halide_device_interface_t *_171 = (struct halide_device_interface_t *)(_169);
  struct {
   const int32_t f_0;
   const int32_t f_1;
   const int32_t f_2;
   const int32_t f_3;
  } s0 = {
   0,
   32,
   1,
   0
  };
  struct halide_dimension_t *_172 = (struct halide_dimension_t *)(&s0);
  struct halide_buffer_t *_173 = _halide_buffer_init(_bias_dw_buffer, _168, _170, _169, _171, 0, 8, 1, _172, _169);
  (void)_173;
 } // if _167
 bool _174 = _halide_buffer_is_bounds_query(_bias_pw_buffer);
 if (_174)
 {
  struct halide_dimension_t *_175 = _halide_buffer_get_shape(_bias_pw_buffer);
  uint64_t _176 = (uint64_t)(0);
  void *_177 = (void *)(_176);
  struct halide_device_interface_t *_178 = (struct halide_device_interface_t *)(_176);
  struct {
   const int32_t f_0;
   const int32_t f_1;
   const int32_t f_2;
   const int32_t f_3;
  } s1 = {
   0,
   32,
   1,
   0
  };
  struct halide_dimension_t *_179 = (struct halide_dimension_t *)(&s1);
  struct halide_buffer_t *_180 = _halide_buffer_init(_bias_pw_buffer, _175, _177, _176, _178, 0, 8, 1, _179, _176);
  (void)_180;
 } // if _174
 bool _181 = _halide_buffer_is_bounds_query(_filter_dw_buffer);
 if (_181)
 {
  struct halide_dimension_t *_182 = _halide_buffer_get_shape(_filter_dw_buffer);
  uint64_t _183 = (uint64_t)(0);
  void *_184 = (void *)(_183);
  struct halide_device_interface_t *_185 = (struct halide_device_interface_t *)(_183);
  struct {
   const int32_t f_0;
   const int32_t f_1;
   const int32_t f_2;
   const int32_t f_3;
   const int32_t f_4;
   const int32_t f_5;
   const int32_t f_6;
   const int32_t f_7;
   const int32_t f_8;
   const int32_t f_9;
   const int32_t f_10;
   const int32_t f_11;
  } s2 = {
   0,
   3,
   1,
   0,
   0,
   3,
   3,
   0,
   0,
   32,
   9,
   0
  };
  struct halide_dimension_t *_186 = (struct halide_dimension_t *)(&s2);
  struct halide_buffer_t *_187 = _halide_buffer_init(_filter_dw_buffer, _182, _184, _183, _185, 0, 8, 3, _186, _183);
  (void)_187;
 } // if _181
 bool _188 = _halide_buffer_is_bounds_query(_filter_pw_buffer);
 if (_188)
 {
  struct halide_dimension_t *_189 = _halide_buffer_get_shape(_filter_pw_buffer);
  uint64_t _190 = (uint64_t)(0);
  void *_191 = (void *)(_190);
  struct halide_device_interface_t *_192 = (struct halide_device_interface_t *)(_190);
  struct {
   const int32_t f_0;
   const int32_t f_1;
   const int32_t f_2;
   const int32_t f_3;
   const int32_t f_4;
   const int32_t f_5;
   const int32_t f_6;
   const int32_t f_7;
  } s3 = {
   0,
   32,
   1,
   0,
   0,
   64,
   32,
   0
  };
  struct halide_dimension_t *_193 = (struct halide_dimension_t *)(&s3);
  struct halide_buffer_t *_194 = _halide_buffer_init(_filter_pw_buffer, _189, _191, _190, _192, 0, 8, 2, _193, _190);
  (void)_194;
 } // if _188
 bool _195 = _halide_buffer_is_bounds_query(_input_buffer);
 if (_195)
 {
  struct halide_dimension_t *_196 = _halide_buffer_get_shape(_input_buffer);
  uint64_t _197 = (uint64_t)(0);
  void *_198 = (void *)(_197);
  struct halide_device_interface_t *_199 = (struct halide_device_interface_t *)(_197);
  int32_t _200 = _110 + 2;
  int32_t _201 = _113 + 2;
  int32_t _202 = _123 * 32;
  struct {
   const int32_t f_0;
   const int32_t f_1;
   const int32_t f_2;
   const int32_t f_3;
   const int32_t f_4;
   const int32_t f_5;
   const int32_t f_6;
   const int32_t f_7;
   const int32_t f_8;
   const int32_t f_9;
   const int32_t f_10;
   const int32_t f_11;
   const int32_t f_12;
   const int32_t f_13;
   const int32_t f_14;
   const int32_t f_15;
  } s4 = {
   _109,
   _200,
   1,
   0,
   _112,
   _201,
   _200,
   0,
   0,
   32,
   _123,
   0,
   _118,
   _119,
   _202,
   0
  };
  struct halide_dimension_t *_203 = (struct halide_dimension_t *)(&s4);
  struct halide_buffer_t *_204 = _halide_buffer_init(_input_buffer, _196, _198, _197, _199, 0, 8, 4, _203, _197);
  (void)_204;
 } // if _195
 bool _205 = _halide_buffer_is_bounds_query(_output_buffer);
 if (_205)
 {
  struct halide_dimension_t *_206 = _halide_buffer_get_shape(_output_buffer);
  uint64_t _207 = (uint64_t)(0);
  void *_208 = (void *)(_207);
  struct halide_device_interface_t *_209 = (struct halide_device_interface_t *)(_207);
  int32_t _210 = ::halide_cpp_min(_110, 32);
  int32_t _211 = _210 + _109;
  int32_t _212 = _211 + -32;
  int32_t _213 = _130 + 64;
  int32_t _214 = ::halide_cpp_min(_113, 32);
  int32_t _215 = _214 + _112;
  int32_t _216 = _215 + -32;
  int32_t _217 = _137 + 64;
  int32_t _218 = _116 * _140;
  struct {
   const int32_t f_0;
   const int32_t f_1;
   const int32_t f_2;
   const int32_t f_3;
   const int32_t f_4;
   const int32_t f_5;
   const int32_t f_6;
   const int32_t f_7;
   const int32_t f_8;
   const int32_t f_9;
   const int32_t f_10;
   const int32_t f_11;
   const int32_t f_12;
   const int32_t f_13;
   const int32_t f_14;
   const int32_t f_15;
  } s5 = {
   _212,
   _213,
   1,
   0,
   _216,
   _217,
   _213,
   0,
   _115,
   _116,
   _140,
   0,
   _118,
   _119,
   _218,
   0
  };
  struct halide_dimension_t *_219 = (struct halide_dimension_t *)(&s5);
  struct halide_buffer_t *_220 = _halide_buffer_init(_output_buffer, _206, _208, _207, _209, 0, 8, 4, _219, _207);
  (void)_220;
 } // if _205
 bool _221 = _halide_buffer_is_bounds_query(_output_buffer);
 bool _222 = _halide_buffer_is_bounds_query(_input_buffer);
 bool _223 = _halide_buffer_is_bounds_query(_filter_pw_buffer);
 bool _224 = _halide_buffer_is_bounds_query(_filter_dw_buffer);
 bool _225 = _halide_buffer_is_bounds_query(_bias_dw_buffer);
 bool _226 = _halide_buffer_is_bounds_query(_bias_pw_buffer);
 bool _227 = _225 || _226;
 bool _228 = _224 || _227;
 bool _229 = _223 || _228;
 bool _230 = _222 || _229;
 bool _231 = _221 || _230;
 bool _232 = !(_231);
 if (_232)
 {
  uint8_t _233 = (uint8_t)(0);
  bool _234 = _47 == _233;
  uint8_t _235 = (uint8_t)(8);
  bool _236 = _48 == _235;
  bool _237 = _234 && _236;
  uint16_t _238 = (uint16_t)(1);
  bool _239 = _49 == _238;
  bool _240 = _237 && _239;
  if (!_240)   {
   uint8_t _241 = (uint8_t)(0);
   uint8_t _242 = (uint8_t)(8);
   uint16_t _243 = (uint16_t)(1);
   int32_t _244 = halide_error_bad_type(_ucon, "Input buffer bias_dw", _47, _241, _48, _242, _49, _243);
   return _244;
  }
  bool _245 = _50 == 1;
  if (!_245)   {
   int32_t _246 = halide_error_bad_dimensions(_ucon, "Input buffer bias_dw", _50, 1);
   return _246;
  }
  uint8_t _247 = (uint8_t)(0);
  bool _248 = _55 == _247;
  uint8_t _249 = (uint8_t)(8);
  bool _250 = _56 == _249;
  bool _251 = _248 && _250;
  uint16_t _252 = (uint16_t)(1);
  bool _253 = _57 == _252;
  bool _254 = _251 && _253;
  if (!_254)   {
   uint8_t _255 = (uint8_t)(0);
   uint8_t _256 = (uint8_t)(8);
   uint16_t _257 = (uint16_t)(1);
   int32_t _258 = halide_error_bad_type(_ucon, "Input buffer bias_pw", _55, _255, _56, _256, _57, _257);
   return _258;
  }
  bool _259 = _58 == 1;
  if (!_259)   {
   int32_t _260 = halide_error_bad_dimensions(_ucon, "Input buffer bias_pw", _58, 1);
   return _260;
  }
  uint8_t _261 = (uint8_t)(0);
  bool _262 = _63 == _261;
  uint8_t _263 = (uint8_t)(8);
  bool _264 = _64 == _263;
  bool _265 = _262 && _264;
  uint16_t _266 = (uint16_t)(1);
  bool _267 = _65 == _266;
  bool _268 = _265 && _267;
  if (!_268)   {
   uint8_t _269 = (uint8_t)(0);
   uint8_t _270 = (uint8_t)(8);
   uint16_t _271 = (uint16_t)(1);
   int32_t _272 = halide_error_bad_type(_ucon, "Input buffer filter_dw", _63, _269, _64, _270, _65, _271);
   return _272;
  }
  bool _273 = _66 == 3;
  if (!_273)   {
   int32_t _274 = halide_error_bad_dimensions(_ucon, "Input buffer filter_dw", _66, 3);
   return _274;
  }
  uint8_t _275 = (uint8_t)(0);
  bool _276 = _77 == _275;
  uint8_t _277 = (uint8_t)(8);
  bool _278 = _78 == _277;
  bool _279 = _276 && _278;
  uint16_t _280 = (uint16_t)(1);
  bool _281 = _79 == _280;
  bool _282 = _279 && _281;
  if (!_282)   {
   uint8_t _283 = (uint8_t)(0);
   uint8_t _284 = (uint8_t)(8);
   uint16_t _285 = (uint16_t)(1);
   int32_t _286 = halide_error_bad_type(_ucon, "Input buffer filter_pw", _77, _283, _78, _284, _79, _285);
   return _286;
  }
  bool _287 = _80 == 2;
  if (!_287)   {
   int32_t _288 = halide_error_bad_dimensions(_ucon, "Input buffer filter_pw", _80, 2);
   return _288;
  }
  uint8_t _289 = (uint8_t)(0);
  bool _290 = _88 == _289;
  uint8_t _291 = (uint8_t)(8);
  bool _292 = _89 == _291;
  bool _293 = _290 && _292;
  uint16_t _294 = (uint16_t)(1);
  bool _295 = _90 == _294;
  bool _296 = _293 && _295;
  if (!_296)   {
   uint8_t _297 = (uint8_t)(0);
   uint8_t _298 = (uint8_t)(8);
   uint16_t _299 = (uint16_t)(1);
   int32_t _300 = halide_error_bad_type(_ucon, "Input buffer input", _88, _297, _89, _298, _90, _299);
   return _300;
  }
  bool _301 = _91 == 4;
  if (!_301)   {
   int32_t _302 = halide_error_bad_dimensions(_ucon, "Input buffer input", _91, 4);
   return _302;
  }
  uint8_t _303 = (uint8_t)(0);
  bool _304 = _105 == _303;
  uint8_t _305 = (uint8_t)(8);
  bool _306 = _106 == _305;
  bool _307 = _304 && _306;
  uint16_t _308 = (uint16_t)(1);
  bool _309 = _107 == _308;
  bool _310 = _307 && _309;
  if (!_310)   {
   uint8_t _311 = (uint8_t)(0);
   uint8_t _312 = (uint8_t)(8);
   uint16_t _313 = (uint16_t)(1);
   int32_t _314 = halide_error_bad_type(_ucon, "Output buffer output", _105, _311, _106, _312, _107, _313);
   return _314;
  }
  bool _315 = _108 == 4;
  if (!_315)   {
   int32_t _316 = halide_error_bad_dimensions(_ucon, "Output buffer output", _108, 4);
   return _316;
  }
  bool _317 = _51 <= 0;
  int32_t _318 = _52 + _51;
  bool _319 = 32 <= _318;
  bool _320 = _317 && _319;
  if (!_320)   {
   int32_t _321 = _52 + _51;
   int32_t _322 = _321 + -1;
   int32_t _323 = halide_error_access_out_of_bounds(_ucon, "Input buffer bias_dw", 0, 0, 31, _51, _322);
   return _323;
  }
  bool _324 = 0 <= _52;
  if (!_324)   {
   int32_t _325 = halide_error_buffer_extents_negative(_ucon, "Input buffer bias_dw", 0, _52);
   return _325;
  }
  bool _326 = _59 <= _115;
  int32_t _327 = _116 + _115;
  int32_t _328 = _60 + _59;
  bool _329 = _327 <= _328;
  bool _330 = _326 && _329;
  if (!_330)   {
   int32_t _331 = _116 + _115;
   int32_t _332 = _331 + -1;
   int32_t _333 = _60 + _59;
   int32_t _334 = _333 + -1;
   int32_t _335 = halide_error_access_out_of_bounds(_ucon, "Input buffer bias_pw", 0, _115, _332, _59, _334);
   return _335;
  }
  bool _336 = 0 <= _60;
  if (!_336)   {
   int32_t _337 = halide_error_buffer_extents_negative(_ucon, "Input buffer bias_pw", 0, _60);
   return _337;
  }
  bool _338 = _67 <= 0;
  int32_t _339 = _68 + _67;
  bool _340 = 3 <= _339;
  bool _341 = _338 && _340;
  if (!_341)   {
   int32_t _342 = _68 + _67;
   int32_t _343 = _342 + -1;
   int32_t _344 = halide_error_access_out_of_bounds(_ucon, "Input buffer filter_dw", 0, 0, 2, _67, _343);
   return _344;
  }
  bool _345 = 0 <= _68;
  if (!_345)   {
   int32_t _346 = halide_error_buffer_extents_negative(_ucon, "Input buffer filter_dw", 0, _68);
   return _346;
  }
  bool _347 = _70 <= 0;
  int32_t _348 = _71 + _70;
  bool _349 = 3 <= _348;
  bool _350 = _347 && _349;
  if (!_350)   {
   int32_t _351 = _71 + _70;
   int32_t _352 = _351 + -1;
   int32_t _353 = halide_error_access_out_of_bounds(_ucon, "Input buffer filter_dw", 1, 0, 2, _70, _352);
   return _353;
  }
  bool _354 = 0 <= _71;
  if (!_354)   {
   int32_t _355 = halide_error_buffer_extents_negative(_ucon, "Input buffer filter_dw", 1, _71);
   return _355;
  }
  bool _356 = _73 <= 0;
  int32_t _357 = _74 + _73;
  bool _358 = 32 <= _357;
  bool _359 = _356 && _358;
  if (!_359)   {
   int32_t _360 = _74 + _73;
   int32_t _361 = _360 + -1;
   int32_t _362 = halide_error_access_out_of_bounds(_ucon, "Input buffer filter_dw", 2, 0, 31, _73, _361);
   return _362;
  }
  bool _363 = 0 <= _74;
  if (!_363)   {
   int32_t _364 = halide_error_buffer_extents_negative(_ucon, "Input buffer filter_dw", 2, _74);
   return _364;
  }
  bool _365 = _81 <= 0;
  int32_t _366 = _82 + _81;
  bool _367 = 32 <= _366;
  bool _368 = _365 && _367;
  if (!_368)   {
   int32_t _369 = _82 + _81;
   int32_t _370 = _369 + -1;
   int32_t _371 = halide_error_access_out_of_bounds(_ucon, "Input buffer filter_pw", 0, 0, 31, _81, _370);
   return _371;
  }
  bool _372 = 0 <= _82;
  if (!_372)   {
   int32_t _373 = halide_error_buffer_extents_negative(_ucon, "Input buffer filter_pw", 0, _82);
   return _373;
  }
  bool _374 = _84 <= _115;
  int32_t _375 = _116 + -1;
  int32_t _376 = _375 >> 2;
  int32_t _377 = _376 * 4;
  int32_t _378 = _377 + _115;
  int32_t _379 = _378 + 4;
  int32_t _380 = _85 + _84;
  bool _381 = _379 <= _380;
  bool _382 = _374 && _381;
  if (!_382)   {
   int32_t _383 = _116 + -1;
   int32_t _384 = _383 >> 2;
   int32_t _385 = _384 * 4;
   int32_t _386 = _385 + _115;
   int32_t _387 = _386 + 3;
   int32_t _388 = _85 + _84;
   int32_t _389 = _388 + -1;
   int32_t _390 = halide_error_access_out_of_bounds(_ucon, "Input buffer filter_pw", 1, _115, _387, _84, _389);
   return _390;
  }
  bool _391 = 0 <= _85;
  if (!_391)   {
   int32_t _392 = halide_error_buffer_extents_negative(_ucon, "Input buffer filter_pw", 1, _85);
   return _392;
  }
  bool _393 = _92 <= _109;
  int32_t _394 = _110 + _109;
  int32_t _395 = _394 + 2;
  int32_t _396 = _93 + _92;
  bool _397 = _395 <= _396;
  bool _398 = _393 && _397;
  if (!_398)   {
   int32_t _399 = _110 + _109;
   int32_t _400 = _399 + 1;
   int32_t _401 = _93 + _92;
   int32_t _402 = _401 + -1;
   int32_t _403 = halide_error_access_out_of_bounds(_ucon, "Input buffer input", 0, _109, _400, _92, _402);
   return _403;
  }
  bool _404 = 0 <= _93;
  if (!_404)   {
   int32_t _405 = halide_error_buffer_extents_negative(_ucon, "Input buffer input", 0, _93);
   return _405;
  }
  bool _406 = _95 <= _112;
  int32_t _407 = _113 + _112;
  int32_t _408 = _407 + 2;
  int32_t _409 = _96 + _95;
  bool _410 = _408 <= _409;
  bool _411 = _406 && _410;
  if (!_411)   {
   int32_t _412 = _113 + _112;
   int32_t _413 = _412 + 1;
   int32_t _414 = _96 + _95;
   int32_t _415 = _414 + -1;
   int32_t _416 = halide_error_access_out_of_bounds(_ucon, "Input buffer input", 1, _112, _413, _95, _415);
   return _416;
  }
  bool _417 = 0 <= _96;
  if (!_417)   {
   int32_t _418 = halide_error_buffer_extents_negative(_ucon, "Input buffer input", 1, _96);
   return _418;
  }
  bool _419 = _98 <= 0;
  int32_t _420 = _99 + _98;
  bool _421 = 32 <= _420;
  bool _422 = _419 && _421;
  if (!_422)   {
   int32_t _423 = _99 + _98;
   int32_t _424 = _423 + -1;
   int32_t _425 = halide_error_access_out_of_bounds(_ucon, "Input buffer input", 2, 0, 31, _98, _424);
   return _425;
  }
  bool _426 = 0 <= _99;
  if (!_426)   {
   int32_t _427 = halide_error_buffer_extents_negative(_ucon, "Input buffer input", 2, _99);
   return _427;
  }
  bool _428 = _101 <= _118;
  int32_t _429 = _119 + _118;
  int32_t _430 = _102 + _101;
  bool _431 = _429 <= _430;
  bool _432 = _428 && _431;
  if (!_432)   {
   int32_t _433 = _119 + _118;
   int32_t _434 = _433 + -1;
   int32_t _435 = _102 + _101;
   int32_t _436 = _435 + -1;
   int32_t _437 = halide_error_access_out_of_bounds(_ucon, "Input buffer input", 3, _118, _434, _101, _436);
   return _437;
  }
  bool _438 = 0 <= _102;
  if (!_438)   {
   int32_t _439 = halide_error_buffer_extents_negative(_ucon, "Input buffer input", 3, _102);
   return _439;
  }
  bool _440 = 32 <= _110;
  int32_t _441 = ::halide_cpp_min(_110, 32);
  int32_t _442 = _441 + _109;
  int32_t _443 = _442 + _130;
  int32_t _444 = _443 + 32;
  int32_t _445 = _110 + _109;
  bool _446 = _444 <= _445;
  bool _447 = _440 && _446;
  if (!_447)   {
   int32_t _448 = ::halide_cpp_min(_110, 32);
   int32_t _449 = _448 + _109;
   int32_t _450 = _449 + -32;
   int32_t _451 = _449 + _130;
   int32_t _452 = _451 + 31;
   int32_t _453 = _110 + _109;
   int32_t _454 = _453 + -1;
   int32_t _455 = halide_error_access_out_of_bounds(_ucon, "Output buffer output", 0, _450, _452, _109, _454);
   return _455;
  }
  bool _456 = 32 <= _113;
  int32_t _457 = ::halide_cpp_min(_113, 32);
  int32_t _458 = _457 + _112;
  int32_t _459 = _458 + _137;
  int32_t _460 = _459 + 32;
  int32_t _461 = _113 + _112;
  bool _462 = _460 <= _461;
  bool _463 = _456 && _462;
  if (!_463)   {
   int32_t _464 = ::halide_cpp_min(_113, 32);
   int32_t _465 = _464 + _112;
   int32_t _466 = _465 + -32;
   int32_t _467 = _465 + _137;
   int32_t _468 = _467 + 31;
   int32_t _469 = _113 + _112;
   int32_t _470 = _469 + -1;
   int32_t _471 = halide_error_access_out_of_bounds(_ucon, "Output buffer output", 1, _466, _468, _112, _470);
   return _471;
  }
  bool _472 = 0 <= _116;
  if (!_472)   {
   int32_t _473 = halide_error_buffer_extents_negative(_ucon, "Output buffer output", 2, _116);
   return _473;
  }
  bool _474 = 0 <= _119;
  if (!_474)   {
   int32_t _475 = halide_error_buffer_extents_negative(_ucon, "Output buffer output", 3, _119);
   return _475;
  }
  bool _476 = _53 == 1;
  if (!_476)   {
   int32_t _477 = halide_error_constraint_violated(_ucon, "bias_dw.stride.0", _53, "1", 1);
   return _477;
  }
  bool _478 = _51 == 0;
  if (!_478)   {
   int32_t _479 = halide_error_constraint_violated(_ucon, "bias_dw.min.0", _51, "0", 0);
   return _479;
  }
  bool _480 = _52 == 32;
  if (!_480)   {
   int32_t _481 = halide_error_constraint_violated(_ucon, "bias_dw.extent.0", _52, "32", 32);
   return _481;
  }
  bool _482 = _61 == 1;
  if (!_482)   {
   int32_t _483 = halide_error_constraint_violated(_ucon, "bias_pw.stride.0", _61, "1", 1);
   return _483;
  }
  bool _484 = _59 == 0;
  if (!_484)   {
   int32_t _485 = halide_error_constraint_violated(_ucon, "bias_pw.min.0", _59, "0", 0);
   return _485;
  }
  bool _486 = _60 == 32;
  if (!_486)   {
   int32_t _487 = halide_error_constraint_violated(_ucon, "bias_pw.extent.0", _60, "32", 32);
   return _487;
  }
  bool _488 = _69 == 1;
  if (!_488)   {
   int32_t _489 = halide_error_constraint_violated(_ucon, "filter_dw.stride.0", _69, "1", 1);
   return _489;
  }
  bool _490 = _67 == 0;
  if (!_490)   {
   int32_t _491 = halide_error_constraint_violated(_ucon, "filter_dw.min.0", _67, "0", 0);
   return _491;
  }
  bool _492 = _68 == 3;
  if (!_492)   {
   int32_t _493 = halide_error_constraint_violated(_ucon, "filter_dw.extent.0", _68, "3", 3);
   return _493;
  }
  bool _494 = _70 == 0;
  if (!_494)   {
   int32_t _495 = halide_error_constraint_violated(_ucon, "filter_dw.min.1", _70, "0", 0);
   return _495;
  }
  bool _496 = _71 == 3;
  if (!_496)   {
   int32_t _497 = halide_error_constraint_violated(_ucon, "filter_dw.extent.1", _71, "3", 3);
   return _497;
  }
  bool _498 = _73 == 0;
  if (!_498)   {
   int32_t _499 = halide_error_constraint_violated(_ucon, "filter_dw.min.2", _73, "0", 0);
   return _499;
  }
  bool _500 = _74 == 32;
  if (!_500)   {
   int32_t _501 = halide_error_constraint_violated(_ucon, "filter_dw.extent.2", _74, "32", 32);
   return _501;
  }
  bool _502 = _83 == 1;
  if (!_502)   {
   int32_t _503 = halide_error_constraint_violated(_ucon, "filter_pw.stride.0", _83, "1", 1);
   return _503;
  }
  bool _504 = _81 == 0;
  if (!_504)   {
   int32_t _505 = halide_error_constraint_violated(_ucon, "filter_pw.min.0", _81, "0", 0);
   return _505;
  }
  bool _506 = _82 == 32;
  if (!_506)   {
   int32_t _507 = halide_error_constraint_violated(_ucon, "filter_pw.extent.0", _82, "32", 32);
   return _507;
  }
  bool _508 = _84 == 0;
  if (!_508)   {
   int32_t _509 = halide_error_constraint_violated(_ucon, "filter_pw.min.1", _84, "0", 0);
   return _509;
  }
  bool _510 = _85 == 64;
  if (!_510)   {
   int32_t _511 = halide_error_constraint_violated(_ucon, "filter_pw.extent.1", _85, "64", 64);
   return _511;
  }
  bool _512 = _94 == 1;
  if (!_512)   {
   int32_t _513 = halide_error_constraint_violated(_ucon, "input.stride.0", _94, "1", 1);
   return _513;
  }
  bool _514 = _111 == 1;
  if (!_514)   {
   int32_t _515 = halide_error_constraint_violated(_ucon, "output.stride.0", _111, "1", 1);
   return _515;
  }
  int64_t _516 = (int64_t)(_96);
  int64_t _517 = (int64_t)(_93);
  int64_t _518 = _516 * _517;
  int64_t _519 = (int64_t)(_99);
  int64_t _520 = _518 * _519;
  int64_t _521 = (int64_t)(_102);
  int64_t _522 = _520 * _521;
  int64_t _523 = (int64_t)(_113);
  int64_t _524 = (int64_t)(_110);
  int64_t _525 = _523 * _524;
  int64_t _526 = (int64_t)(_116);
  int64_t _527 = _525 * _526;
  int64_t _528 = (int64_t)(_119);
  int64_t _529 = _527 * _528;
  int64_t _530 = (int64_t)(_72);
  int64_t _531 = (int64_t)(3);
  int64_t _532 = _530 * _531;
  int64_t _533 = (int64_t)(0);
  int64_t _534 = _533 - _532;
  bool _535 = _532 > _533;
  int64_t _536 = (int64_t)(_535 ? _532 : _534);
  uint64_t _537 = (uint64_t)(_536);
  uint64_t _538 = _537;
  uint64_t _539 = (uint64_t)(2147483647);
  bool _540 = _538 <= _539;
  if (!_540)   {
   int64_t _541 = (int64_t)(_72);
   int64_t _542 = (int64_t)(3);
   int64_t _543 = _541 * _542;
   int64_t _544 = (int64_t)(0);
   int64_t _545 = _544 - _543;
   bool _546 = _543 > _544;
   int64_t _547 = (int64_t)(_546 ? _543 : _545);
   uint64_t _548 = (uint64_t)(_547);
   uint64_t _549 = _548;
   uint64_t _550 = (uint64_t)(2147483647);
   int32_t _551 = halide_error_buffer_allocation_too_large(_ucon, "filter_dw", _549, _550);
   return _551;
  }
  int64_t _552 = (int64_t)(_75);
  int64_t _553 = (int64_t)(32);
  int64_t _554 = _552 * _553;
  int64_t _555 = (int64_t)(0);
  int64_t _556 = _555 - _554;
  bool _557 = _554 > _555;
  int64_t _558 = (int64_t)(_557 ? _554 : _556);
  uint64_t _559 = (uint64_t)(_558);
  uint64_t _560 = _559;
  uint64_t _561 = (uint64_t)(2147483647);
  bool _562 = _560 <= _561;
  if (!_562)   {
   int64_t _563 = (int64_t)(_75);
   int64_t _564 = (int64_t)(32);
   int64_t _565 = _563 * _564;
   int64_t _566 = (int64_t)(0);
   int64_t _567 = _566 - _565;
   bool _568 = _565 > _566;
   int64_t _569 = (int64_t)(_568 ? _565 : _567);
   uint64_t _570 = (uint64_t)(_569);
   uint64_t _571 = _570;
   uint64_t _572 = (uint64_t)(2147483647);
   int32_t _573 = halide_error_buffer_allocation_too_large(_ucon, "filter_dw", _571, _572);
   return _573;
  }
  int64_t _574 = (int64_t)(_86);
  int64_t _575 = (int64_t)(64);
  int64_t _576 = _574 * _575;
  int64_t _577 = (int64_t)(0);
  int64_t _578 = _577 - _576;
  bool _579 = _576 > _577;
  int64_t _580 = (int64_t)(_579 ? _576 : _578);
  uint64_t _581 = (uint64_t)(_580);
  uint64_t _582 = _581;
  uint64_t _583 = (uint64_t)(2147483647);
  bool _584 = _582 <= _583;
  if (!_584)   {
   int64_t _585 = (int64_t)(_86);
   int64_t _586 = (int64_t)(64);
   int64_t _587 = _585 * _586;
   int64_t _588 = (int64_t)(0);
   int64_t _589 = _588 - _587;
   bool _590 = _587 > _588;
   int64_t _591 = (int64_t)(_590 ? _587 : _589);
   uint64_t _592 = (uint64_t)(_591);
   uint64_t _593 = _592;
   uint64_t _594 = (uint64_t)(2147483647);
   int32_t _595 = halide_error_buffer_allocation_too_large(_ucon, "filter_pw", _593, _594);
   return _595;
  }
  int64_t _596 = (int64_t)(_93);
  int64_t _597 = (int64_t)(0);
  int64_t _598 = _597 - _596;
  bool _599 = _596 > _597;
  int64_t _600 = (int64_t)(_599 ? _596 : _598);
  uint64_t _601 = (uint64_t)(_600);
  uint64_t _602 = _601;
  uint64_t _603 = (uint64_t)(2147483647);
  bool _604 = _602 <= _603;
  if (!_604)   {
   int64_t _605 = (int64_t)(_93);
   int64_t _606 = (int64_t)(0);
   int64_t _607 = _606 - _605;
   bool _608 = _605 > _606;
   int64_t _609 = (int64_t)(_608 ? _605 : _607);
   uint64_t _610 = (uint64_t)(_609);
   uint64_t _611 = _610;
   uint64_t _612 = (uint64_t)(2147483647);
   int32_t _613 = halide_error_buffer_allocation_too_large(_ucon, "input", _611, _612);
   return _613;
  }
  int64_t _614 = (int64_t)(_96);
  int64_t _615 = (int64_t)(_97);
  int64_t _616 = _614 * _615;
  int64_t _617 = (int64_t)(0);
  int64_t _618 = _617 - _616;
  bool _619 = _616 > _617;
  int64_t _620 = (int64_t)(_619 ? _616 : _618);
  uint64_t _621 = (uint64_t)(_620);
  uint64_t _622 = _621;
  uint64_t _623 = (uint64_t)(2147483647);
  bool _624 = _622 <= _623;
  if (!_624)   {
   int64_t _625 = (int64_t)(_96);
   int64_t _626 = (int64_t)(_97);
   int64_t _627 = _625 * _626;
   int64_t _628 = (int64_t)(0);
   int64_t _629 = _628 - _627;
   bool _630 = _627 > _628;
   int64_t _631 = (int64_t)(_630 ? _627 : _629);
   uint64_t _632 = (uint64_t)(_631);
   uint64_t _633 = _632;
   uint64_t _634 = (uint64_t)(2147483647);
   int32_t _635 = halide_error_buffer_allocation_too_large(_ucon, "input", _633, _634);
   return _635;
  }
  int64_t _636 = (int64_t)(2147483647);
  bool _637 = _518 <= _636;
  if (!_637)   {
   int64_t _638 = (int64_t)(2147483647);
   int32_t _639 = halide_error_buffer_extents_too_large(_ucon, "input", _518, _638);
   return _639;
  }
  int64_t _640 = (int64_t)(_99);
  int64_t _641 = (int64_t)(_100);
  int64_t _642 = _640 * _641;
  int64_t _643 = (int64_t)(0);
  int64_t _644 = _643 - _642;
  bool _645 = _642 > _643;
  int64_t _646 = (int64_t)(_645 ? _642 : _644);
  uint64_t _647 = (uint64_t)(_646);
  uint64_t _648 = _647;
  uint64_t _649 = (uint64_t)(2147483647);
  bool _650 = _648 <= _649;
  if (!_650)   {
   int64_t _651 = (int64_t)(_99);
   int64_t _652 = (int64_t)(_100);
   int64_t _653 = _651 * _652;
   int64_t _654 = (int64_t)(0);
   int64_t _655 = _654 - _653;
   bool _656 = _653 > _654;
   int64_t _657 = (int64_t)(_656 ? _653 : _655);
   uint64_t _658 = (uint64_t)(_657);
   uint64_t _659 = _658;
   uint64_t _660 = (uint64_t)(2147483647);
   int32_t _661 = halide_error_buffer_allocation_too_large(_ucon, "input", _659, _660);
   return _661;
  }
  int64_t _662 = (int64_t)(2147483647);
  bool _663 = _520 <= _662;
  if (!_663)   {
   int64_t _664 = (int64_t)(2147483647);
   int32_t _665 = halide_error_buffer_extents_too_large(_ucon, "input", _520, _664);
   return _665;
  }
  int64_t _666 = (int64_t)(_102);
  int64_t _667 = (int64_t)(_103);
  int64_t _668 = _666 * _667;
  int64_t _669 = (int64_t)(0);
  int64_t _670 = _669 - _668;
  bool _671 = _668 > _669;
  int64_t _672 = (int64_t)(_671 ? _668 : _670);
  uint64_t _673 = (uint64_t)(_672);
  uint64_t _674 = _673;
  uint64_t _675 = (uint64_t)(2147483647);
  bool _676 = _674 <= _675;
  if (!_676)   {
   int64_t _677 = (int64_t)(_102);
   int64_t _678 = (int64_t)(_103);
   int64_t _679 = _677 * _678;
   int64_t _680 = (int64_t)(0);
   int64_t _681 = _680 - _679;
   bool _682 = _679 > _680;
   int64_t _683 = (int64_t)(_682 ? _679 : _681);
   uint64_t _684 = (uint64_t)(_683);
   uint64_t _685 = _684;
   uint64_t _686 = (uint64_t)(2147483647);
   int32_t _687 = halide_error_buffer_allocation_too_large(_ucon, "input", _685, _686);
   return _687;
  }
  int64_t _688 = (int64_t)(2147483647);
  bool _689 = _522 <= _688;
  if (!_689)   {
   int64_t _690 = (int64_t)(2147483647);
   int32_t _691 = halide_error_buffer_extents_too_large(_ucon, "input", _522, _690);
   return _691;
  }
  int64_t _692 = (int64_t)(_110);
  int64_t _693 = (int64_t)(0);
  int64_t _694 = _693 - _692;
  bool _695 = _692 > _693;
  int64_t _696 = (int64_t)(_695 ? _692 : _694);
  uint64_t _697 = (uint64_t)(_696);
  uint64_t _698 = _697;
  uint64_t _699 = (uint64_t)(2147483647);
  bool _700 = _698 <= _699;
  if (!_700)   {
   int64_t _701 = (int64_t)(_110);
   int64_t _702 = (int64_t)(0);
   int64_t _703 = _702 - _701;
   bool _704 = _701 > _702;
   int64_t _705 = (int64_t)(_704 ? _701 : _703);
   uint64_t _706 = (uint64_t)(_705);
   uint64_t _707 = _706;
   uint64_t _708 = (uint64_t)(2147483647);
   int32_t _709 = halide_error_buffer_allocation_too_large(_ucon, "output", _707, _708);
   return _709;
  }
  int64_t _710 = (int64_t)(_113);
  int64_t _711 = (int64_t)(_114);
  int64_t _712 = _710 * _711;
  int64_t _713 = (int64_t)(0);
  int64_t _714 = _713 - _712;
  bool _715 = _712 > _713;
  int64_t _716 = (int64_t)(_715 ? _712 : _714);
  uint64_t _717 = (uint64_t)(_716);
  uint64_t _718 = _717;
  uint64_t _719 = (uint64_t)(2147483647);
  bool _720 = _718 <= _719;
  if (!_720)   {
   int64_t _721 = (int64_t)(_113);
   int64_t _722 = (int64_t)(_114);
   int64_t _723 = _721 * _722;
   int64_t _724 = (int64_t)(0);
   int64_t _725 = _724 - _723;
   bool _726 = _723 > _724;
   int64_t _727 = (int64_t)(_726 ? _723 : _725);
   uint64_t _728 = (uint64_t)(_727);
   uint64_t _729 = _728;
   uint64_t _730 = (uint64_t)(2147483647);
   int32_t _731 = halide_error_buffer_allocation_too_large(_ucon, "output", _729, _730);
   return _731;
  }
  int64_t _732 = (int64_t)(2147483647);
  bool _733 = _525 <= _732;
  if (!_733)   {
   int64_t _734 = (int64_t)(2147483647);
   int32_t _735 = halide_error_buffer_extents_too_large(_ucon, "output", _525, _734);
   return _735;
  }
  int64_t _736 = (int64_t)(_116);
  int64_t _737 = (int64_t)(_117);
  int64_t _738 = _736 * _737;
  int64_t _739 = (int64_t)(0);
  int64_t _740 = _739 - _738;
  bool _741 = _738 > _739;
  int64_t _742 = (int64_t)(_741 ? _738 : _740);
  uint64_t _743 = (uint64_t)(_742);
  uint64_t _744 = _743;
  uint64_t _745 = (uint64_t)(2147483647);
  bool _746 = _744 <= _745;
  if (!_746)   {
   int64_t _747 = (int64_t)(_116);
   int64_t _748 = (int64_t)(_117);
   int64_t _749 = _747 * _748;
   int64_t _750 = (int64_t)(0);
   int64_t _751 = _750 - _749;
   bool _752 = _749 > _750;
   int64_t _753 = (int64_t)(_752 ? _749 : _751);
   uint64_t _754 = (uint64_t)(_753);
   uint64_t _755 = _754;
   uint64_t _756 = (uint64_t)(2147483647);
   int32_t _757 = halide_error_buffer_allocation_too_large(_ucon, "output", _755, _756);
   return _757;
  }
  int64_t _758 = (int64_t)(2147483647);
  bool _759 = _527 <= _758;
  if (!_759)   {
   int64_t _760 = (int64_t)(2147483647);
   int32_t _761 = halide_error_buffer_extents_too_large(_ucon, "output", _527, _760);
   return _761;
  }
  int64_t _762 = (int64_t)(_119);
  int64_t _763 = (int64_t)(_120);
  int64_t _764 = _762 * _763;
  int64_t _765 = (int64_t)(0);
  int64_t _766 = _765 - _764;
  bool _767 = _764 > _765;
  int64_t _768 = (int64_t)(_767 ? _764 : _766);
  uint64_t _769 = (uint64_t)(_768);
  uint64_t _770 = _769;
  uint64_t _771 = (uint64_t)(2147483647);
  bool _772 = _770 <= _771;
  if (!_772)   {
   int64_t _773 = (int64_t)(_119);
   int64_t _774 = (int64_t)(_120);
   int64_t _775 = _773 * _774;
   int64_t _776 = (int64_t)(0);
   int64_t _777 = _776 - _775;
   bool _778 = _775 > _776;
   int64_t _779 = (int64_t)(_778 ? _775 : _777);
   uint64_t _780 = (uint64_t)(_779);
   uint64_t _781 = _780;
   uint64_t _782 = (uint64_t)(2147483647);
   int32_t _783 = halide_error_buffer_allocation_too_large(_ucon, "output", _781, _782);
   return _783;
  }
  int64_t _784 = (int64_t)(2147483647);
  bool _785 = _529 <= _784;
  if (!_785)   {
   int64_t _786 = (int64_t)(2147483647);
   int32_t _787 = halide_error_buffer_extents_too_large(_ucon, "output", _529, _786);
   return _787;
  }
  uint64_t _788 = (uint64_t)(0);
  void *_789 = (void *)(_788);
  bool _790 = _bias_dw != _789;
  if (!_790)   {
   int32_t _791 = halide_error_host_is_null(_ucon, "Input buffer bias_dw");
   return _791;
  }
  uint64_t _792 = (uint64_t)(0);
  void *_793 = (void *)(_792);
  bool _794 = _bias_pw != _793;
  if (!_794)   {
   int32_t _795 = halide_error_host_is_null(_ucon, "Input buffer bias_pw");
   return _795;
  }
  uint64_t _796 = (uint64_t)(0);
  void *_797 = (void *)(_796);
  bool _798 = _filter_dw != _797;
  if (!_798)   {
   int32_t _799 = halide_error_host_is_null(_ucon, "Input buffer filter_dw");
   return _799;
  }
  uint64_t _800 = (uint64_t)(0);
  void *_801 = (void *)(_800);
  bool _802 = _filter_pw != _801;
  if (!_802)   {
   int32_t _803 = halide_error_host_is_null(_ucon, "Input buffer filter_pw");
   return _803;
  }
  uint64_t _804 = (uint64_t)(0);
  void *_805 = (void *)(_804);
  bool _806 = _input != _805;
  if (!_806)   {
   int32_t _807 = halide_error_host_is_null(_ucon, "Input buffer input");
   return _807;
  }
  uint64_t _808 = (uint64_t)(0);
  void *_809 = (void *)(_808);
  bool _810 = _output != _809;
  if (!_810)   {
   int32_t _811 = halide_error_host_is_null(_ucon, "Output buffer output");
   return _811;
  }
  // produce output
  int32_t _812 = _72 * 2;
  int32_t _813 = _116 + 3;
  int32_t _814 = _813 >> 2;
  int32_t _815 = _113 + 31;
  int32_t _816 = _815 >> 5;
  int32_t _817 = _110 + 31;
  int32_t _818 = _817 >> 5;
  int32_t _819 = _118 * _120;
  int32_t _820 = _109 - _819;
  int32_t _821 = _115 * _117;
  int32_t _822 = _820 - _821;
  int32_t _823 = _112 * _114;
  int32_t _824 = _822 - _823;
  int32_t _825 = _824 - _109;
  int32_t _826 = _116 + -1;
  int32_t _827 = _826 >> 2;
  int32_t _828 = _827 * 4;
  int32_t _829 = _828 + 4;
  int32_t _830 = _95 * _97;
  int32_t _831 = _101 * _103;
  int32_t _832 = _98 * _100;
  int32_t _833 = _831 + _832;
  int32_t _834 = _830 + _833;
  int32_t _835 = _834 + _92;
  int32_t _836 = _113 + -32;
  int32_t _837 = _110 + -32;
  for (int _output_s0_n = _118; _output_s0_n < _118 + _119; _output_s0_n++)
  {
   int32_t _838 = _103 * _output_s0_n;
   int32_t _839 = _838 - _835;
   int32_t _840 = _output_s0_n * _120;
   int32_t _841 = _840 + _825;
   for (int _output_s0_y_yo = 0; _output_s0_y_yo < 0 + _816; _output_s0_y_yo++)
   {
    int32_t _842 = _output_s0_y_yo * 32;
    int32_t _843 = ::halide_cpp_min(_842, _836);
    int32_t _844 = _112 + _843;
    for (int _output_s0_x_xo = 0; _output_s0_x_xo < 0 + _818; _output_s0_x_xo++)
    {
     int32_t _845 = _output_s0_x_xo * 32;
     int32_t _846 = ::halide_cpp_min(_845, _837);
     {
      int8_t _input_im_global_wrapper[544];
      {
       int64_t _847 = 32768;
       int16_t *_dw_conv = (int16_t  *)halide_malloc(_ucon, sizeof(int16_t )*_847);
       if (!_dw_conv)
       {
        return halide_error_out_of_memory(_ucon);
       }
       HalideFreeHelper _dw_conv_free(_ucon, _dw_conv, halide_free);
       // produce dw_conv
       int32_t _848 = _109 + _846;
       for (int _dw_conv_s0_c = 0; _dw_conv_s0_c < 0 + 32; _dw_conv_s0_c++)
       {
        int32_t _849 = _dw_conv_s0_c * 1024;
        int32_t _850 = _849 - _848;
        for (int _dw_conv_s0_y = _844; _dw_conv_s0_y < _844 + 32; _dw_conv_s0_y++)
        {
         int32_t _851 = _dw_conv_s0_y - _844;
         int32_t _852 = _851 * 32;
         int32_t _853 = _852 + _850;
         for (int _dw_conv_s0_x = _848; _dw_conv_s0_x < _848 + 32; _dw_conv_s0_x++)
         {
          int8_t _854 = ((const int8_t *)_bias_dw)[_dw_conv_s0_c];
          int16_t _855 = (int16_t)(_854);
          int32_t _856 = _dw_conv_s0_x + _853;
          _dw_conv[_856] = _855;
         } // for _dw_conv_s0_x
        } // for _dw_conv_s0_y
       } // for _dw_conv_s0_c
       int32_t _857 = 3072 - _109;
       int32_t _858 = _857 - _846;
       int32_t _859 = 2048 - _109;
       int32_t _860 = _859 - _846;
       int32_t _861 = 1024 - _109;
       int32_t _862 = _861 - _846;
       int32_t _863 = _109 + _846;
       for (int _dw_conv_s1_c_co_dw = 0; _dw_conv_s1_c_co_dw < 0 + 8; _dw_conv_s1_c_co_dw++)
       {
        int32_t _864 = _dw_conv_s1_c_co_dw * 4;
        int32_t _865 = _864 + 1;
        int32_t _866 = _865 * _75;
        int32_t _867 = _72 + _866;
        int32_t _868 = _812 + _866;
        int32_t _869 = _864 + 2;
        int32_t _870 = _869 * _75;
        int32_t _871 = _72 + _870;
        int32_t _872 = _812 + _870;
        int32_t _873 = _864 + 3;
        int32_t _874 = _873 * _75;
        int32_t _875 = _72 + _874;
        int32_t _876 = _812 + _874;
        int32_t _877 = _dw_conv_s1_c_co_dw * _75;
        int32_t _878 = _877 * 4;
        int32_t _879 = _72 + _878;
        int32_t _880 = _dw_conv_s1_c_co_dw * 4096;
        int32_t _881 = _877 * 2;
        int32_t _882 = _881 + _72;
        int32_t _883 = _882 * 2;
        int32_t _884 = _858 + _880;
        int32_t _885 = _860 + _880;
        int32_t _886 = _862 + _880;
        for (int _dw_conv_s1_y = _844; _dw_conv_s1_y < _844 + 32; _dw_conv_s1_y++)
        {
         int32_t _887 = _dw_conv_s1_y - _844;
         int32_t _888 = _887 * 32;
         int32_t _889 = _880 + _888;
         int32_t _890 = _dw_conv_s1_y + 2;
         bool _891 = _844 < _dw_conv_s1_y;
         int32_t _892 = (int32_t)(_891 ? _890 : _dw_conv_s1_y);
         int32_t _893 = _890 & 3;
         int32_t _894 = _893 * 34;
         int32_t _895 = _dw_conv_s1_y + 1;
         int32_t _896 = _895 & 3;
         int32_t _897 = _896 * 34;
         int32_t _898 = _dw_conv_s1_y & 3;
         int32_t _899 = _898 * 34;
         int32_t _900 = _880 - _863;
         int32_t _901 = _900 + _888;
         int32_t _902 = _884 + _888;
         int32_t _903 = _885 + _888;
         int32_t _904 = _886 + _888;
         int32_t _905 = _dw_conv_s1_y + 3;
         for (int _dw_conv_s1_x = _863; _dw_conv_s1_x < _863 + 32; _dw_conv_s1_x++)
         {
          int32_t _906 = _dw_conv_s1_x + 2;
          bool _907 = _863 < _dw_conv_s1_x;
          int32_t _908 = (int32_t)(_907 ? _906 : _dw_conv_s1_x);
          // produce input_im_global_wrapper
          int32_t _909 = _905 - _892;
          int32_t _910 = _dw_conv_s1_x - _908;
          int32_t _911 = _910 + 3;
          for (int _input_im_global_wrapper_s0__1 = _892; _input_im_global_wrapper_s0__1 < _892 + _909; _input_im_global_wrapper_s0__1++)
          {
           int32_t _912 = _input_im_global_wrapper_s0__1 & 3;
           int32_t _913 = _912 * 34;
           int32_t _914 = _913 - _863;
           int32_t _915 = _97 * _input_im_global_wrapper_s0__1;
           int32_t _916 = _915 + _839;
           for (int _input_im_global_wrapper_s0__0 = _908; _input_im_global_wrapper_s0__0 < _908 + _911; _input_im_global_wrapper_s0__0++)
           {
            int32_t _917 = _input_im_global_wrapper_s0__0 + _914;
            int32_t _918 = _917 * 4;
            int32_t _919 = _input_im_global_wrapper_s0__0 + _916;
            for (int _input_im_global_wrapper_s0__2 = _864; _input_im_global_wrapper_s0__2 < _864 + 4; _input_im_global_wrapper_s0__2++)
            {
             int32_t _920 = _100 * _input_im_global_wrapper_s0__2;
             int32_t _921 = _920 + _919;
             int8_t _922 = ((const int8_t *)_input)[_921];
             int32_t _923 = _input_im_global_wrapper_s0__2 & 3;
             int32_t _924 = _923 + _918;
             _input_im_global_wrapper[_924] = _922;
            } // for _input_im_global_wrapper_s0__2
           } // for _input_im_global_wrapper_s0__0
          } // for _input_im_global_wrapper_s0__1
          // consume input_im_global_wrapper
          int32_t _925 = _dw_conv_s1_x - _863;
          int32_t _926 = _925 + _889;
          int16_t _927 = _dw_conv[_926];
          int8_t _928 = ((const int8_t *)_filter_dw)[_878];
          int32_t _929 = _925 + _899;
          int32_t _930 = _929 * 4;
          int8_t _931 = _input_im_global_wrapper[_930];
          int8_t _932 = _928 * _931;
          int16_t _933 = (int16_t)(_932);
          int16_t _934 = _927 + _933;
          int32_t _935 = _dw_conv_s1_x + _901;
          _dw_conv[_935] = _934;
          int32_t _936 = _dw_conv_s1_x - _863;
          int32_t _937 = _936 + _889;
          int16_t _938 = _dw_conv[_937];
          int32_t _939 = _878 + 1;
          int8_t _940 = ((const int8_t *)_filter_dw)[_939];
          int32_t _941 = _936 + _899;
          int32_t _942 = _941 * 4;
          int32_t _943 = _942 + 4;
          int8_t _944 = _input_im_global_wrapper[_943];
          int8_t _945 = _940 * _944;
          int16_t _946 = (int16_t)(_945);
          int16_t _947 = _938 + _946;
          int32_t _948 = _dw_conv_s1_x + _901;
          _dw_conv[_948] = _947;
          int32_t _949 = _dw_conv_s1_x - _863;
          int32_t _950 = _949 + _889;
          int16_t _951 = _dw_conv[_950];
          int32_t _952 = _878 + 2;
          int8_t _953 = ((const int8_t *)_filter_dw)[_952];
          int32_t _954 = _949 + _899;
          int32_t _955 = _954 * 4;
          int32_t _956 = _955 + 8;
          int8_t _957 = _input_im_global_wrapper[_956];
          int8_t _958 = _953 * _957;
          int16_t _959 = (int16_t)(_958);
          int16_t _960 = _951 + _959;
          int32_t _961 = _dw_conv_s1_x + _901;
          _dw_conv[_961] = _960;
          int32_t _962 = _dw_conv_s1_x - _863;
          int32_t _963 = _962 + _889;
          int16_t _964 = _dw_conv[_963];
          int8_t _965 = ((const int8_t *)_filter_dw)[_879];
          int32_t _966 = _962 + _897;
          int32_t _967 = _966 * 4;
          int8_t _968 = _input_im_global_wrapper[_967];
          int8_t _969 = _965 * _968;
          int16_t _970 = (int16_t)(_969);
          int16_t _971 = _964 + _970;
          int32_t _972 = _dw_conv_s1_x + _901;
          _dw_conv[_972] = _971;
          int32_t _973 = _dw_conv_s1_x - _863;
          int32_t _974 = _973 + _889;
          int16_t _975 = _dw_conv[_974];
          int32_t _976 = _879 + 1;
          int8_t _977 = ((const int8_t *)_filter_dw)[_976];
          int32_t _978 = _973 + _897;
          int32_t _979 = _978 * 4;
          int32_t _980 = _979 + 4;
          int8_t _981 = _input_im_global_wrapper[_980];
          int8_t _982 = _977 * _981;
          int16_t _983 = (int16_t)(_982);
          int16_t _984 = _975 + _983;
          int32_t _985 = _dw_conv_s1_x + _901;
          _dw_conv[_985] = _984;
          int32_t _986 = _dw_conv_s1_x - _863;
          int32_t _987 = _986 + _889;
          int16_t _988 = _dw_conv[_987];
          int32_t _989 = _879 + 2;
          int8_t _990 = ((const int8_t *)_filter_dw)[_989];
          int32_t _991 = _986 + _897;
          int32_t _992 = _991 * 4;
          int32_t _993 = _992 + 8;
          int8_t _994 = _input_im_global_wrapper[_993];
          int8_t _995 = _990 * _994;
          int16_t _996 = (int16_t)(_995);
          int16_t _997 = _988 + _996;
          int32_t _998 = _dw_conv_s1_x + _901;
          _dw_conv[_998] = _997;
          int32_t _999 = _dw_conv_s1_x - _863;
          int32_t _1000 = _999 + _889;
          int16_t _1001 = _dw_conv[_1000];
          int8_t _1002 = ((const int8_t *)_filter_dw)[_883];
          int32_t _1003 = _999 + _894;
          int32_t _1004 = _1003 * 4;
          int8_t _1005 = _input_im_global_wrapper[_1004];
          int8_t _1006 = _1002 * _1005;
          int16_t _1007 = (int16_t)(_1006);
          int16_t _1008 = _1001 + _1007;
          int32_t _1009 = _dw_conv_s1_x + _901;
          _dw_conv[_1009] = _1008;
          int32_t _1010 = _dw_conv_s1_x - _863;
          int32_t _1011 = _1010 + _889;
          int16_t _1012 = _dw_conv[_1011];
          int32_t _1013 = _883 + 1;
          int8_t _1014 = ((const int8_t *)_filter_dw)[_1013];
          int32_t _1015 = _1010 + _894;
          int32_t _1016 = _1015 * 4;
          int32_t _1017 = _1016 + 4;
          int8_t _1018 = _input_im_global_wrapper[_1017];
          int8_t _1019 = _1014 * _1018;
          int16_t _1020 = (int16_t)(_1019);
          int16_t _1021 = _1012 + _1020;
          int32_t _1022 = _dw_conv_s1_x + _901;
          _dw_conv[_1022] = _1021;
          int32_t _1023 = _dw_conv_s1_x - _863;
          int32_t _1024 = _1023 + _889;
          int16_t _1025 = _dw_conv[_1024];
          int32_t _1026 = _883 + 2;
          int8_t _1027 = ((const int8_t *)_filter_dw)[_1026];
          int32_t _1028 = _1023 + _894;
          int32_t _1029 = _1028 * 4;
          int32_t _1030 = _1029 + 8;
          int8_t _1031 = _input_im_global_wrapper[_1030];
          int8_t _1032 = _1027 * _1031;
          int16_t _1033 = (int16_t)(_1032);
          int16_t _1034 = _1025 + _1033;
          int32_t _1035 = _dw_conv_s1_x + _901;
          _dw_conv[_1035] = _1034;
          int32_t _1036 = _dw_conv_s1_x - _863;
          int32_t _1037 = _1036 + _889;
          int32_t _1038 = _1037 + 1024;
          int16_t _1039 = _dw_conv[_1038];
          int8_t _1040 = ((const int8_t *)_filter_dw)[_866];
          int32_t _1041 = _1036 + _899;
          int32_t _1042 = _1041 * 4;
          int32_t _1043 = _1042 + 1;
          int8_t _1044 = _input_im_global_wrapper[_1043];
          int8_t _1045 = _1040 * _1044;
          int16_t _1046 = (int16_t)(_1045);
          int16_t _1047 = _1039 + _1046;
          int32_t _1048 = _dw_conv_s1_x + _904;
          _dw_conv[_1048] = _1047;
          int32_t _1049 = _dw_conv_s1_x - _863;
          int32_t _1050 = _1049 + _889;
          int32_t _1051 = _1050 + 1024;
          int16_t _1052 = _dw_conv[_1051];
          int32_t _1053 = _866 + 1;
          int8_t _1054 = ((const int8_t *)_filter_dw)[_1053];
          int32_t _1055 = _1049 + _899;
          int32_t _1056 = _1055 * 4;
          int32_t _1057 = _1056 + 5;
          int8_t _1058 = _input_im_global_wrapper[_1057];
          int8_t _1059 = _1054 * _1058;
          int16_t _1060 = (int16_t)(_1059);
          int16_t _1061 = _1052 + _1060;
          int32_t _1062 = _dw_conv_s1_x + _904;
          _dw_conv[_1062] = _1061;
          int32_t _1063 = _dw_conv_s1_x - _863;
          int32_t _1064 = _1063 + _889;
          int32_t _1065 = _1064 + 1024;
          int16_t _1066 = _dw_conv[_1065];
          int32_t _1067 = _866 + 2;
          int8_t _1068 = ((const int8_t *)_filter_dw)[_1067];
          int32_t _1069 = _1063 + _899;
          int32_t _1070 = _1069 * 4;
          int32_t _1071 = _1070 + 9;
          int8_t _1072 = _input_im_global_wrapper[_1071];
          int8_t _1073 = _1068 * _1072;
          int16_t _1074 = (int16_t)(_1073);
          int16_t _1075 = _1066 + _1074;
          int32_t _1076 = _dw_conv_s1_x + _904;
          _dw_conv[_1076] = _1075;
          int32_t _1077 = _dw_conv_s1_x - _863;
          int32_t _1078 = _1077 + _889;
          int32_t _1079 = _1078 + 1024;
          int16_t _1080 = _dw_conv[_1079];
          int8_t _1081 = ((const int8_t *)_filter_dw)[_867];
          int32_t _1082 = _1077 + _897;
          int32_t _1083 = _1082 * 4;
          int32_t _1084 = _1083 + 1;
          int8_t _1085 = _input_im_global_wrapper[_1084];
          int8_t _1086 = _1081 * _1085;
          int16_t _1087 = (int16_t)(_1086);
          int16_t _1088 = _1080 + _1087;
          int32_t _1089 = _dw_conv_s1_x + _904;
          _dw_conv[_1089] = _1088;
          int32_t _1090 = _dw_conv_s1_x - _863;
          int32_t _1091 = _1090 + _889;
          int32_t _1092 = _1091 + 1024;
          int16_t _1093 = _dw_conv[_1092];
          int32_t _1094 = _867 + 1;
          int8_t _1095 = ((const int8_t *)_filter_dw)[_1094];
          int32_t _1096 = _1090 + _897;
          int32_t _1097 = _1096 * 4;
          int32_t _1098 = _1097 + 5;
          int8_t _1099 = _input_im_global_wrapper[_1098];
          int8_t _1100 = _1095 * _1099;
          int16_t _1101 = (int16_t)(_1100);
          int16_t _1102 = _1093 + _1101;
          int32_t _1103 = _dw_conv_s1_x + _904;
          _dw_conv[_1103] = _1102;
          int32_t _1104 = _dw_conv_s1_x - _863;
          int32_t _1105 = _1104 + _889;
          int32_t _1106 = _1105 + 1024;
          int16_t _1107 = _dw_conv[_1106];
          int32_t _1108 = _867 + 2;
          int8_t _1109 = ((const int8_t *)_filter_dw)[_1108];
          int32_t _1110 = _1104 + _897;
          int32_t _1111 = _1110 * 4;
          int32_t _1112 = _1111 + 9;
          int8_t _1113 = _input_im_global_wrapper[_1112];
          int8_t _1114 = _1109 * _1113;
          int16_t _1115 = (int16_t)(_1114);
          int16_t _1116 = _1107 + _1115;
          int32_t _1117 = _dw_conv_s1_x + _904;
          _dw_conv[_1117] = _1116;
          int32_t _1118 = _dw_conv_s1_x - _863;
          int32_t _1119 = _1118 + _889;
          int32_t _1120 = _1119 + 1024;
          int16_t _1121 = _dw_conv[_1120];
          int8_t _1122 = ((const int8_t *)_filter_dw)[_868];
          int32_t _1123 = _1118 + _894;
          int32_t _1124 = _1123 * 4;
          int32_t _1125 = _1124 + 1;
          int8_t _1126 = _input_im_global_wrapper[_1125];
          int8_t _1127 = _1122 * _1126;
          int16_t _1128 = (int16_t)(_1127);
          int16_t _1129 = _1121 + _1128;
          int32_t _1130 = _dw_conv_s1_x + _904;
          _dw_conv[_1130] = _1129;
          int32_t _1131 = _dw_conv_s1_x - _863;
          int32_t _1132 = _1131 + _889;
          int32_t _1133 = _1132 + 1024;
          int16_t _1134 = _dw_conv[_1133];
          int32_t _1135 = _868 + 1;
          int8_t _1136 = ((const int8_t *)_filter_dw)[_1135];
          int32_t _1137 = _1131 + _894;
          int32_t _1138 = _1137 * 4;
          int32_t _1139 = _1138 + 5;
          int8_t _1140 = _input_im_global_wrapper[_1139];
          int8_t _1141 = _1136 * _1140;
          int16_t _1142 = (int16_t)(_1141);
          int16_t _1143 = _1134 + _1142;
          int32_t _1144 = _dw_conv_s1_x + _904;
          _dw_conv[_1144] = _1143;
          int32_t _1145 = _dw_conv_s1_x - _863;
          int32_t _1146 = _1145 + _889;
          int32_t _1147 = _1146 + 1024;
          int16_t _1148 = _dw_conv[_1147];
          int32_t _1149 = _868 + 2;
          int8_t _1150 = ((const int8_t *)_filter_dw)[_1149];
          int32_t _1151 = _1145 + _894;
          int32_t _1152 = _1151 * 4;
          int32_t _1153 = _1152 + 9;
          int8_t _1154 = _input_im_global_wrapper[_1153];
          int8_t _1155 = _1150 * _1154;
          int16_t _1156 = (int16_t)(_1155);
          int16_t _1157 = _1148 + _1156;
          int32_t _1158 = _dw_conv_s1_x + _904;
          _dw_conv[_1158] = _1157;
          int32_t _1159 = _dw_conv_s1_x - _863;
          int32_t _1160 = _1159 + _889;
          int32_t _1161 = _1160 + 2048;
          int16_t _1162 = _dw_conv[_1161];
          int8_t _1163 = ((const int8_t *)_filter_dw)[_870];
          int32_t _1164 = _1159 + _899;
          int32_t _1165 = _1164 * 4;
          int32_t _1166 = _1165 + 2;
          int8_t _1167 = _input_im_global_wrapper[_1166];
          int8_t _1168 = _1163 * _1167;
          int16_t _1169 = (int16_t)(_1168);
          int16_t _1170 = _1162 + _1169;
          int32_t _1171 = _dw_conv_s1_x + _903;
          _dw_conv[_1171] = _1170;
          int32_t _1172 = _dw_conv_s1_x - _863;
          int32_t _1173 = _1172 + _889;
          int32_t _1174 = _1173 + 2048;
          int16_t _1175 = _dw_conv[_1174];
          int32_t _1176 = _870 + 1;
          int8_t _1177 = ((const int8_t *)_filter_dw)[_1176];
          int32_t _1178 = _1172 + _899;
          int32_t _1179 = _1178 * 4;
          int32_t _1180 = _1179 + 6;
          int8_t _1181 = _input_im_global_wrapper[_1180];
          int8_t _1182 = _1177 * _1181;
          int16_t _1183 = (int16_t)(_1182);
          int16_t _1184 = _1175 + _1183;
          int32_t _1185 = _dw_conv_s1_x + _903;
          _dw_conv[_1185] = _1184;
          int32_t _1186 = _dw_conv_s1_x - _863;
          int32_t _1187 = _1186 + _889;
          int32_t _1188 = _1187 + 2048;
          int16_t _1189 = _dw_conv[_1188];
          int32_t _1190 = _870 + 2;
          int8_t _1191 = ((const int8_t *)_filter_dw)[_1190];
          int32_t _1192 = _1186 + _899;
          int32_t _1193 = _1192 * 4;
          int32_t _1194 = _1193 + 10;
          int8_t _1195 = _input_im_global_wrapper[_1194];
          int8_t _1196 = _1191 * _1195;
          int16_t _1197 = (int16_t)(_1196);
          int16_t _1198 = _1189 + _1197;
          int32_t _1199 = _dw_conv_s1_x + _903;
          _dw_conv[_1199] = _1198;
          int32_t _1200 = _dw_conv_s1_x - _863;
          int32_t _1201 = _1200 + _889;
          int32_t _1202 = _1201 + 2048;
          int16_t _1203 = _dw_conv[_1202];
          int8_t _1204 = ((const int8_t *)_filter_dw)[_871];
          int32_t _1205 = _1200 + _897;
          int32_t _1206 = _1205 * 4;
          int32_t _1207 = _1206 + 2;
          int8_t _1208 = _input_im_global_wrapper[_1207];
          int8_t _1209 = _1204 * _1208;
          int16_t _1210 = (int16_t)(_1209);
          int16_t _1211 = _1203 + _1210;
          int32_t _1212 = _dw_conv_s1_x + _903;
          _dw_conv[_1212] = _1211;
          int32_t _1213 = _dw_conv_s1_x - _863;
          int32_t _1214 = _1213 + _889;
          int32_t _1215 = _1214 + 2048;
          int16_t _1216 = _dw_conv[_1215];
          int32_t _1217 = _871 + 1;
          int8_t _1218 = ((const int8_t *)_filter_dw)[_1217];
          int32_t _1219 = _1213 + _897;
          int32_t _1220 = _1219 * 4;
          int32_t _1221 = _1220 + 6;
          int8_t _1222 = _input_im_global_wrapper[_1221];
          int8_t _1223 = _1218 * _1222;
          int16_t _1224 = (int16_t)(_1223);
          int16_t _1225 = _1216 + _1224;
          int32_t _1226 = _dw_conv_s1_x + _903;
          _dw_conv[_1226] = _1225;
          int32_t _1227 = _dw_conv_s1_x - _863;
          int32_t _1228 = _1227 + _889;
          int32_t _1229 = _1228 + 2048;
          int16_t _1230 = _dw_conv[_1229];
          int32_t _1231 = _871 + 2;
          int8_t _1232 = ((const int8_t *)_filter_dw)[_1231];
          int32_t _1233 = _1227 + _897;
          int32_t _1234 = _1233 * 4;
          int32_t _1235 = _1234 + 10;
          int8_t _1236 = _input_im_global_wrapper[_1235];
          int8_t _1237 = _1232 * _1236;
          int16_t _1238 = (int16_t)(_1237);
          int16_t _1239 = _1230 + _1238;
          int32_t _1240 = _dw_conv_s1_x + _903;
          _dw_conv[_1240] = _1239;
          int32_t _1241 = _dw_conv_s1_x - _863;
          int32_t _1242 = _1241 + _889;
          int32_t _1243 = _1242 + 2048;
          int16_t _1244 = _dw_conv[_1243];
          int8_t _1245 = ((const int8_t *)_filter_dw)[_872];
          int32_t _1246 = _1241 + _894;
          int32_t _1247 = _1246 * 4;
          int32_t _1248 = _1247 + 2;
          int8_t _1249 = _input_im_global_wrapper[_1248];
          int8_t _1250 = _1245 * _1249;
          int16_t _1251 = (int16_t)(_1250);
          int16_t _1252 = _1244 + _1251;
          int32_t _1253 = _dw_conv_s1_x + _903;
          _dw_conv[_1253] = _1252;
          int32_t _1254 = _dw_conv_s1_x - _863;
          int32_t _1255 = _1254 + _889;
          int32_t _1256 = _1255 + 2048;
          int16_t _1257 = _dw_conv[_1256];
          int32_t _1258 = _872 + 1;
          int8_t _1259 = ((const int8_t *)_filter_dw)[_1258];
          int32_t _1260 = _1254 + _894;
          int32_t _1261 = _1260 * 4;
          int32_t _1262 = _1261 + 6;
          int8_t _1263 = _input_im_global_wrapper[_1262];
          int8_t _1264 = _1259 * _1263;
          int16_t _1265 = (int16_t)(_1264);
          int16_t _1266 = _1257 + _1265;
          int32_t _1267 = _dw_conv_s1_x + _903;
          _dw_conv[_1267] = _1266;
          int32_t _1268 = _dw_conv_s1_x - _863;
          int32_t _1269 = _1268 + _889;
          int32_t _1270 = _1269 + 2048;
          int16_t _1271 = _dw_conv[_1270];
          int32_t _1272 = _872 + 2;
          int8_t _1273 = ((const int8_t *)_filter_dw)[_1272];
          int32_t _1274 = _1268 + _894;
          int32_t _1275 = _1274 * 4;
          int32_t _1276 = _1275 + 10;
          int8_t _1277 = _input_im_global_wrapper[_1276];
          int8_t _1278 = _1273 * _1277;
          int16_t _1279 = (int16_t)(_1278);
          int16_t _1280 = _1271 + _1279;
          int32_t _1281 = _dw_conv_s1_x + _903;
          _dw_conv[_1281] = _1280;
          int32_t _1282 = _dw_conv_s1_x - _863;
          int32_t _1283 = _1282 + _889;
          int32_t _1284 = _1283 + 3072;
          int16_t _1285 = _dw_conv[_1284];
          int8_t _1286 = ((const int8_t *)_filter_dw)[_874];
          int32_t _1287 = _1282 + _899;
          int32_t _1288 = _1287 * 4;
          int32_t _1289 = _1288 + 3;
          int8_t _1290 = _input_im_global_wrapper[_1289];
          int8_t _1291 = _1286 * _1290;
          int16_t _1292 = (int16_t)(_1291);
          int16_t _1293 = _1285 + _1292;
          int32_t _1294 = _dw_conv_s1_x + _902;
          _dw_conv[_1294] = _1293;
          int32_t _1295 = _dw_conv_s1_x - _863;
          int32_t _1296 = _1295 + _889;
          int32_t _1297 = _1296 + 3072;
          int16_t _1298 = _dw_conv[_1297];
          int32_t _1299 = _874 + 1;
          int8_t _1300 = ((const int8_t *)_filter_dw)[_1299];
          int32_t _1301 = _1295 + _899;
          int32_t _1302 = _1301 * 4;
          int32_t _1303 = _1302 + 7;
          int8_t _1304 = _input_im_global_wrapper[_1303];
          int8_t _1305 = _1300 * _1304;
          int16_t _1306 = (int16_t)(_1305);
          int16_t _1307 = _1298 + _1306;
          int32_t _1308 = _dw_conv_s1_x + _902;
          _dw_conv[_1308] = _1307;
          int32_t _1309 = _dw_conv_s1_x - _863;
          int32_t _1310 = _1309 + _889;
          int32_t _1311 = _1310 + 3072;
          int16_t _1312 = _dw_conv[_1311];
          int32_t _1313 = _874 + 2;
          int8_t _1314 = ((const int8_t *)_filter_dw)[_1313];
          int32_t _1315 = _1309 + _899;
          int32_t _1316 = _1315 * 4;
          int32_t _1317 = _1316 + 11;
          int8_t _1318 = _input_im_global_wrapper[_1317];
          int8_t _1319 = _1314 * _1318;
          int16_t _1320 = (int16_t)(_1319);
          int16_t _1321 = _1312 + _1320;
          int32_t _1322 = _dw_conv_s1_x + _902;
          _dw_conv[_1322] = _1321;
          int32_t _1323 = _dw_conv_s1_x - _863;
          int32_t _1324 = _1323 + _889;
          int32_t _1325 = _1324 + 3072;
          int16_t _1326 = _dw_conv[_1325];
          int8_t _1327 = ((const int8_t *)_filter_dw)[_875];
          int32_t _1328 = _1323 + _897;
          int32_t _1329 = _1328 * 4;
          int32_t _1330 = _1329 + 3;
          int8_t _1331 = _input_im_global_wrapper[_1330];
          int8_t _1332 = _1327 * _1331;
          int16_t _1333 = (int16_t)(_1332);
          int16_t _1334 = _1326 + _1333;
          int32_t _1335 = _dw_conv_s1_x + _902;
          _dw_conv[_1335] = _1334;
          int32_t _1336 = _dw_conv_s1_x - _863;
          int32_t _1337 = _1336 + _889;
          int32_t _1338 = _1337 + 3072;
          int16_t _1339 = _dw_conv[_1338];
          int32_t _1340 = _875 + 1;
          int8_t _1341 = ((const int8_t *)_filter_dw)[_1340];
          int32_t _1342 = _1336 + _897;
          int32_t _1343 = _1342 * 4;
          int32_t _1344 = _1343 + 7;
          int8_t _1345 = _input_im_global_wrapper[_1344];
          int8_t _1346 = _1341 * _1345;
          int16_t _1347 = (int16_t)(_1346);
          int16_t _1348 = _1339 + _1347;
          int32_t _1349 = _dw_conv_s1_x + _902;
          _dw_conv[_1349] = _1348;
          int32_t _1350 = _dw_conv_s1_x - _863;
          int32_t _1351 = _1350 + _889;
          int32_t _1352 = _1351 + 3072;
          int16_t _1353 = _dw_conv[_1352];
          int32_t _1354 = _875 + 2;
          int8_t _1355 = ((const int8_t *)_filter_dw)[_1354];
          int32_t _1356 = _1350 + _897;
          int32_t _1357 = _1356 * 4;
          int32_t _1358 = _1357 + 11;
          int8_t _1359 = _input_im_global_wrapper[_1358];
          int8_t _1360 = _1355 * _1359;
          int16_t _1361 = (int16_t)(_1360);
          int16_t _1362 = _1353 + _1361;
          int32_t _1363 = _dw_conv_s1_x + _902;
          _dw_conv[_1363] = _1362;
          int32_t _1364 = _dw_conv_s1_x - _863;
          int32_t _1365 = _1364 + _889;
          int32_t _1366 = _1365 + 3072;
          int16_t _1367 = _dw_conv[_1366];
          int8_t _1368 = ((const int8_t *)_filter_dw)[_876];
          int32_t _1369 = _1364 + _894;
          int32_t _1370 = _1369 * 4;
          int32_t _1371 = _1370 + 3;
          int8_t _1372 = _input_im_global_wrapper[_1371];
          int8_t _1373 = _1368 * _1372;
          int16_t _1374 = (int16_t)(_1373);
          int16_t _1375 = _1367 + _1374;
          int32_t _1376 = _dw_conv_s1_x + _902;
          _dw_conv[_1376] = _1375;
          int32_t _1377 = _dw_conv_s1_x - _863;
          int32_t _1378 = _1377 + _889;
          int32_t _1379 = _1378 + 3072;
          int16_t _1380 = _dw_conv[_1379];
          int32_t _1381 = _876 + 1;
          int8_t _1382 = ((const int8_t *)_filter_dw)[_1381];
          int32_t _1383 = _1377 + _894;
          int32_t _1384 = _1383 * 4;
          int32_t _1385 = _1384 + 7;
          int8_t _1386 = _input_im_global_wrapper[_1385];
          int8_t _1387 = _1382 * _1386;
          int16_t _1388 = (int16_t)(_1387);
          int16_t _1389 = _1380 + _1388;
          int32_t _1390 = _dw_conv_s1_x + _902;
          _dw_conv[_1390] = _1389;
          int32_t _1391 = _dw_conv_s1_x - _863;
          int32_t _1392 = _1391 + _889;
          int32_t _1393 = _1392 + 3072;
          int16_t _1394 = _dw_conv[_1393];
          int32_t _1395 = _876 + 2;
          int8_t _1396 = ((const int8_t *)_filter_dw)[_1395];
          int32_t _1397 = _1391 + _894;
          int32_t _1398 = _1397 * 4;
          int32_t _1399 = _1398 + 11;
          int8_t _1400 = _input_im_global_wrapper[_1399];
          int8_t _1401 = _1396 * _1400;
          int16_t _1402 = (int16_t)(_1401);
          int16_t _1403 = _1394 + _1402;
          int32_t _1404 = _dw_conv_s1_x + _902;
          _dw_conv[_1404] = _1403;
         } // for _dw_conv_s1_x
        } // for _dw_conv_s1_y
       } // for _dw_conv_s1_c_co_dw
       {
        int64_t _1405 = 32;
        int64_t _1406 = _1405 * 32;
        int64_t _1407 = (_1406 > ((int64_t(1) << 31) - 1)) ? _1406 : (_1406 * _829);
        int64_t _1408 = (_1407 > ((int64_t(1) << 31) - 1)) ? _1407 : (_1407 * 1);
        if ((_1408 > ((int64_t(1) << 31) - 1)) || ((_1408 * sizeof(int16_t )) > ((int64_t(1) << 31) - 1)))
        {
         halide_error(_ucon, "32-bit signed overflow computing size of allocation pw_conv\n");
         return -1;
        } // overflow test pw_conv
        int64_t _1409 = _1408;
        int16_t *_pw_conv = (int16_t  *)halide_malloc(_ucon, sizeof(int16_t )*_1409);
        if (!_pw_conv)
        {
         return halide_error_out_of_memory(_ucon);
        }
        HalideFreeHelper _pw_conv_free(_ucon, _pw_conv, halide_free);
        // produce pw_conv
        int32_t _1410 = _109 + _846;
        for (int _pw_conv_s0_c = _115; _pw_conv_s0_c < _115 + _116; _pw_conv_s0_c++)
        {
         int32_t _1411 = _pw_conv_s0_c - _115;
         int32_t _1412 = _1411 * 1024;
         int32_t _1413 = _1412 - _1410;
         for (int _pw_conv_s0_y = _844; _pw_conv_s0_y < _844 + 32; _pw_conv_s0_y++)
         {
          int32_t _1414 = _pw_conv_s0_y - _844;
          int32_t _1415 = _1414 * 32;
          int32_t _1416 = _1415 + _1413;
          for (int _pw_conv_s0_x = _1410; _pw_conv_s0_x < _1410 + 32; _pw_conv_s0_x++)
          {
           int8_t _1417 = ((const int8_t *)_bias_pw)[_pw_conv_s0_c];
           int16_t _1418 = (int16_t)(_1417);
           int32_t _1419 = _pw_conv_s0_x + _1416;
           _pw_conv[_1419] = _1418;
          } // for _pw_conv_s0_x
         } // for _pw_conv_s0_y
        } // for _pw_conv_s0_c
        // consume dw_conv
        int32_t _1420 = 3072 - _109;
        int32_t _1421 = _1420 - _846;
        int32_t _1422 = 2048 - _109;
        int32_t _1423 = _1422 - _846;
        int32_t _1424 = 1024 - _109;
        int32_t _1425 = _1424 - _846;
        int32_t _1426 = _109 + _846;
        for (int _pw_conv_s1_c = _115; _pw_conv_s1_c < _115 + _116; _pw_conv_s1_c++)
        {
         for (int _pw_conv_s1_c_co_pw = 0; _pw_conv_s1_c_co_pw < 0 + _814; _pw_conv_s1_c_co_pw++)
         {
          int32_t _1427 = _pw_conv_s1_c_co_pw * 4;
          int32_t _1428 = _1427 + _115;
          int32_t _1429 = _pw_conv_s1_c_co_pw * 4096;
          int32_t _1430 = _1428 + 1;
          int32_t _1431 = _1430 * _86;
          int32_t _1432 = _1428 + 2;
          int32_t _1433 = _1432 * _86;
          int32_t _1434 = _1428 + 3;
          int32_t _1435 = _1434 * _86;
          int32_t _1436 = _86 * _1428;
          int32_t _1437 = _1429 - _1426;
          int32_t _1438 = _1421 + _1429;
          int32_t _1439 = _1423 + _1429;
          int32_t _1440 = _1425 + _1429;
          for (int _pw_conv_s1_y = _844; _pw_conv_s1_y < _844 + 32; _pw_conv_s1_y++)
          {
           int32_t _1441 = _pw_conv_s1_y - _844;
           int32_t _1442 = _1441 * 32;
           int32_t _1443 = _1442 - _1426;
           int32_t _1444 = _1438 + _1442;
           int32_t _1445 = _1439 + _1442;
           int32_t _1446 = _1440 + _1442;
           int32_t _1447 = _1437 + _1442;
           int32_t _1448 = _1421 + _1442;
           int32_t _1449 = _1423 + _1442;
           int32_t _1450 = _1425 + _1442;
           for (int _pw_conv_s1_x = _1426; _pw_conv_s1_x < _1426 + 32; _pw_conv_s1_x++)
           {
            int32_t _1451 = _pw_conv_s1_x + _1444;
            int32_t _1452 = _pw_conv_s1_x + _1445;
            int32_t _1453 = _pw_conv_s1_x + _1446;
            int32_t _1454 = _pw_conv_s1_x + _1447;
            int32_t _1455 = _pw_conv_s1_x + _1448;
            int32_t _1456 = _pw_conv_s1_x + _1449;
            int32_t _1457 = _pw_conv_s1_x + _1450;
            int32_t _1458 = _pw_conv_s1_x + _1443;
            for (int _pw_conv_s1_r_pw__x_z_o = 0; _pw_conv_s1_r_pw__x_z_o < 0 + 8; _pw_conv_s1_r_pw__x_z_o++)
            {
             {
              int16_t _pw_conv_intm[4];
              // produce pw_conv_intm
              for (int _pw_conv_intm_s0_c = _1428; _pw_conv_intm_s0_c < _1428 + 4; _pw_conv_intm_s0_c++)
              {
               int16_t _1459 = (int16_t)(0);
               int32_t _1460 = _pw_conv_intm_s0_c - _1428;
               _pw_conv_intm[_1460] = _1459;
              } // for _pw_conv_intm_s0_c
              int16_t _1461 = _pw_conv_intm[0];
              int32_t _1462 = _pw_conv_s1_r_pw__x_z_o * 4096;
              int32_t _1463 = _1462 + _1458;
              int16_t _1464 = _dw_conv[_1463];
              int32_t _1465 = _pw_conv_s1_r_pw__x_z_o * 4;
              int32_t _1466 = _1465 + _1436;
              int8_t _1467 = ((const int8_t *)_filter_pw)[_1466];
              int16_t _1468 = (int16_t)(_1467);
              int16_t _1469 = _1464 * _1468;
              int16_t _1470 = _1461 + _1469;
              _pw_conv_intm[0] = _1470;
              int16_t _1471 = _pw_conv_intm[0];
              int32_t _1472 = _pw_conv_s1_r_pw__x_z_o * 4096;
              int32_t _1473 = _1472 + _1457;
              int16_t _1474 = _dw_conv[_1473];
              int32_t _1475 = _pw_conv_s1_r_pw__x_z_o * 4;
              int32_t _1476 = _1475 + _1436;
              int32_t _1477 = _1476 + 1;
              int8_t _1478 = ((const int8_t *)_filter_pw)[_1477];
              int16_t _1479 = (int16_t)(_1478);
              int16_t _1480 = _1474 * _1479;
              int16_t _1481 = _1471 + _1480;
              _pw_conv_intm[0] = _1481;
              int16_t _1482 = _pw_conv_intm[0];
              int32_t _1483 = _pw_conv_s1_r_pw__x_z_o * 4096;
              int32_t _1484 = _1483 + _1456;
              int16_t _1485 = _dw_conv[_1484];
              int32_t _1486 = _pw_conv_s1_r_pw__x_z_o * 4;
              int32_t _1487 = _1486 + _1436;
              int32_t _1488 = _1487 + 2;
              int8_t _1489 = ((const int8_t *)_filter_pw)[_1488];
              int16_t _1490 = (int16_t)(_1489);
              int16_t _1491 = _1485 * _1490;
              int16_t _1492 = _1482 + _1491;
              _pw_conv_intm[0] = _1492;
              int16_t _1493 = _pw_conv_intm[0];
              int32_t _1494 = _pw_conv_s1_r_pw__x_z_o * 4096;
              int32_t _1495 = _1494 + _1455;
              int16_t _1496 = _dw_conv[_1495];
              int32_t _1497 = _pw_conv_s1_r_pw__x_z_o * 4;
              int32_t _1498 = _1497 + _1436;
              int32_t _1499 = _1498 + 3;
              int8_t _1500 = ((const int8_t *)_filter_pw)[_1499];
              int16_t _1501 = (int16_t)(_1500);
              int16_t _1502 = _1496 * _1501;
              int16_t _1503 = _1493 + _1502;
              _pw_conv_intm[0] = _1503;
              int16_t _1504 = _pw_conv_intm[1];
              int32_t _1505 = _pw_conv_s1_r_pw__x_z_o * 4096;
              int32_t _1506 = _1505 + _1458;
              int16_t _1507 = _dw_conv[_1506];
              int32_t _1508 = _pw_conv_s1_r_pw__x_z_o * 4;
              int32_t _1509 = _1508 + _1431;
              int8_t _1510 = ((const int8_t *)_filter_pw)[_1509];
              int16_t _1511 = (int16_t)(_1510);
              int16_t _1512 = _1507 * _1511;
              int16_t _1513 = _1504 + _1512;
              _pw_conv_intm[1] = _1513;
              int16_t _1514 = _pw_conv_intm[1];
              int32_t _1515 = _pw_conv_s1_r_pw__x_z_o * 4096;
              int32_t _1516 = _1515 + _1457;
              int16_t _1517 = _dw_conv[_1516];
              int32_t _1518 = _pw_conv_s1_r_pw__x_z_o * 4;
              int32_t _1519 = _1518 + _1431;
              int32_t _1520 = _1519 + 1;
              int8_t _1521 = ((const int8_t *)_filter_pw)[_1520];
              int16_t _1522 = (int16_t)(_1521);
              int16_t _1523 = _1517 * _1522;
              int16_t _1524 = _1514 + _1523;
              _pw_conv_intm[1] = _1524;
              int16_t _1525 = _pw_conv_intm[1];
              int32_t _1526 = _pw_conv_s1_r_pw__x_z_o * 4096;
              int32_t _1527 = _1526 + _1456;
              int16_t _1528 = _dw_conv[_1527];
              int32_t _1529 = _pw_conv_s1_r_pw__x_z_o * 4;
              int32_t _1530 = _1529 + _1431;
              int32_t _1531 = _1530 + 2;
              int8_t _1532 = ((const int8_t *)_filter_pw)[_1531];
              int16_t _1533 = (int16_t)(_1532);
              int16_t _1534 = _1528 * _1533;
              int16_t _1535 = _1525 + _1534;
              _pw_conv_intm[1] = _1535;
              int16_t _1536 = _pw_conv_intm[1];
              int32_t _1537 = _pw_conv_s1_r_pw__x_z_o * 4096;
              int32_t _1538 = _1537 + _1455;
              int16_t _1539 = _dw_conv[_1538];
              int32_t _1540 = _pw_conv_s1_r_pw__x_z_o * 4;
              int32_t _1541 = _1540 + _1431;
              int32_t _1542 = _1541 + 3;
              int8_t _1543 = ((const int8_t *)_filter_pw)[_1542];
              int16_t _1544 = (int16_t)(_1543);
              int16_t _1545 = _1539 * _1544;
              int16_t _1546 = _1536 + _1545;
              _pw_conv_intm[1] = _1546;
              int16_t _1547 = _pw_conv_intm[2];
              int32_t _1548 = _pw_conv_s1_r_pw__x_z_o * 4096;
              int32_t _1549 = _1548 + _1458;
              int16_t _1550 = _dw_conv[_1549];
              int32_t _1551 = _pw_conv_s1_r_pw__x_z_o * 4;
              int32_t _1552 = _1551 + _1433;
              int8_t _1553 = ((const int8_t *)_filter_pw)[_1552];
              int16_t _1554 = (int16_t)(_1553);
              int16_t _1555 = _1550 * _1554;
              int16_t _1556 = _1547 + _1555;
              _pw_conv_intm[2] = _1556;
              int16_t _1557 = _pw_conv_intm[2];
              int32_t _1558 = _pw_conv_s1_r_pw__x_z_o * 4096;
              int32_t _1559 = _1558 + _1457;
              int16_t _1560 = _dw_conv[_1559];
              int32_t _1561 = _pw_conv_s1_r_pw__x_z_o * 4;
              int32_t _1562 = _1561 + _1433;
              int32_t _1563 = _1562 + 1;
              int8_t _1564 = ((const int8_t *)_filter_pw)[_1563];
              int16_t _1565 = (int16_t)(_1564);
              int16_t _1566 = _1560 * _1565;
              int16_t _1567 = _1557 + _1566;
              _pw_conv_intm[2] = _1567;
              int16_t _1568 = _pw_conv_intm[2];
              int32_t _1569 = _pw_conv_s1_r_pw__x_z_o * 4096;
              int32_t _1570 = _1569 + _1456;
              int16_t _1571 = _dw_conv[_1570];
              int32_t _1572 = _pw_conv_s1_r_pw__x_z_o * 4;
              int32_t _1573 = _1572 + _1433;
              int32_t _1574 = _1573 + 2;
              int8_t _1575 = ((const int8_t *)_filter_pw)[_1574];
              int16_t _1576 = (int16_t)(_1575);
              int16_t _1577 = _1571 * _1576;
              int16_t _1578 = _1568 + _1577;
              _pw_conv_intm[2] = _1578;
              int16_t _1579 = _pw_conv_intm[2];
              int32_t _1580 = _pw_conv_s1_r_pw__x_z_o * 4096;
              int32_t _1581 = _1580 + _1455;
              int16_t _1582 = _dw_conv[_1581];
              int32_t _1583 = _pw_conv_s1_r_pw__x_z_o * 4;
              int32_t _1584 = _1583 + _1433;
              int32_t _1585 = _1584 + 3;
              int8_t _1586 = ((const int8_t *)_filter_pw)[_1585];
              int16_t _1587 = (int16_t)(_1586);
              int16_t _1588 = _1582 * _1587;
              int16_t _1589 = _1579 + _1588;
              _pw_conv_intm[2] = _1589;
              int16_t _1590 = _pw_conv_intm[3];
              int32_t _1591 = _pw_conv_s1_r_pw__x_z_o * 4096;
              int32_t _1592 = _1591 + _1458;
              int16_t _1593 = _dw_conv[_1592];
              int32_t _1594 = _pw_conv_s1_r_pw__x_z_o * 4;
              int32_t _1595 = _1594 + _1435;
              int8_t _1596 = ((const int8_t *)_filter_pw)[_1595];
              int16_t _1597 = (int16_t)(_1596);
              int16_t _1598 = _1593 * _1597;
              int16_t _1599 = _1590 + _1598;
              _pw_conv_intm[3] = _1599;
              int16_t _1600 = _pw_conv_intm[3];
              int32_t _1601 = _pw_conv_s1_r_pw__x_z_o * 4096;
              int32_t _1602 = _1601 + _1457;
              int16_t _1603 = _dw_conv[_1602];
              int32_t _1604 = _pw_conv_s1_r_pw__x_z_o * 4;
              int32_t _1605 = _1604 + _1435;
              int32_t _1606 = _1605 + 1;
              int8_t _1607 = ((const int8_t *)_filter_pw)[_1606];
              int16_t _1608 = (int16_t)(_1607);
              int16_t _1609 = _1603 * _1608;
              int16_t _1610 = _1600 + _1609;
              _pw_conv_intm[3] = _1610;
              int16_t _1611 = _pw_conv_intm[3];
              int32_t _1612 = _pw_conv_s1_r_pw__x_z_o * 4096;
              int32_t _1613 = _1612 + _1456;
              int16_t _1614 = _dw_conv[_1613];
              int32_t _1615 = _pw_conv_s1_r_pw__x_z_o * 4;
              int32_t _1616 = _1615 + _1435;
              int32_t _1617 = _1616 + 2;
              int8_t _1618 = ((const int8_t *)_filter_pw)[_1617];
              int16_t _1619 = (int16_t)(_1618);
              int16_t _1620 = _1614 * _1619;
              int16_t _1621 = _1611 + _1620;
              _pw_conv_intm[3] = _1621;
              int16_t _1622 = _pw_conv_intm[3];
              int32_t _1623 = _pw_conv_s1_r_pw__x_z_o * 4096;
              int32_t _1624 = _1623 + _1455;
              int16_t _1625 = _dw_conv[_1624];
              int32_t _1626 = _pw_conv_s1_r_pw__x_z_o * 4;
              int32_t _1627 = _1626 + _1435;
              int32_t _1628 = _1627 + 3;
              int8_t _1629 = ((const int8_t *)_filter_pw)[_1628];
              int16_t _1630 = (int16_t)(_1629);
              int16_t _1631 = _1625 * _1630;
              int16_t _1632 = _1622 + _1631;
              _pw_conv_intm[3] = _1632;
              // consume pw_conv_intm
              int16_t _1633 = _pw_conv_intm[0];
              int16_t _1634 = _pw_conv[_1454];
              int16_t _1635 = _1633 + _1634;
              _pw_conv[_1454] = _1635;
              int16_t _1636 = _pw_conv_intm[1];
              int16_t _1637 = _pw_conv[_1453];
              int16_t _1638 = _1636 + _1637;
              _pw_conv[_1453] = _1638;
              int16_t _1639 = _pw_conv_intm[2];
              int16_t _1640 = _pw_conv[_1452];
              int16_t _1641 = _1639 + _1640;
              _pw_conv[_1452] = _1641;
              int16_t _1642 = _pw_conv_intm[3];
              int16_t _1643 = _pw_conv[_1451];
              int16_t _1644 = _1642 + _1643;
              _pw_conv[_1451] = _1644;
             } // alloc _pw_conv_intm
            } // for _pw_conv_s1_r_pw__x_z_o
           } // for _pw_conv_s1_x
          } // for _pw_conv_s1_y
         } // for _pw_conv_s1_c_co_pw
        } // for _pw_conv_s1_c
        _dw_conv_free.free();
        // consume pw_conv
        int32_t _1645 = _846 + _841;
        for (int _output_s0_y_yi = 0; _output_s0_y_yi < 0 + 32; _output_s0_y_yi++)
        {
         int32_t _1646 = _output_s0_y_yi * 32;
         int32_t _1647 = _output_s0_y_yi + _844;
         int32_t _1648 = _1647 * _114;
         int32_t _1649 = _1648 + _1645;
         for (int _output_s0_x_xi = 0; _output_s0_x_xi < 0 + 32; _output_s0_x_xi++)
         {
          int32_t _1650 = _output_s0_x_xi + _1649;
          int32_t _1651 = _output_s0_x_xi + _1646;
          for (int _output_s0_c = _115; _output_s0_c < _115 + _116; _output_s0_c++)
          {
           int32_t _1652 = _output_s0_c - _115;
           int32_t _1653 = _1652 * 1024;
           int32_t _1654 = _1653 + _1651;
           int16_t _1655 = _pw_conv[_1654];
           int16_t _1656 = (int16_t)(0);
           int16_t _1657 = ::halide_cpp_max(_1655, _1656);
           int8_t _1658 = (int8_t)(_1657);
           int32_t _1659 = _output_s0_c * _117;
           int32_t _1660 = _1659 + _1650;
           ((int8_t *)_output)[_1660] = _1658;
          } // for _output_s0_c
         } // for _output_s0_x_xi
        } // for _output_s0_y_yi
        _pw_conv_free.free();
       } // alloc _pw_conv
      } // alloc _dw_conv
     } // alloc _input_im_global_wrapper
    } // for _output_s0_x_xo
   } // for _output_s0_y_yo
  } // for _output_s0_n
 } // if _232
 return 0;
}

#ifdef __cplusplus
}  // extern "C"
#endif

