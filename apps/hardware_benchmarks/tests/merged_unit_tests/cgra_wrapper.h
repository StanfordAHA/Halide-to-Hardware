#pragma once

#include "coreir.h"
#include "coreir/simulator/interpreter.h"

// Dummy file
//
// p
//
struct halide_buffer_t;

//void halide_error_bad_type(void *user_context, const char*) {
  //assert(false);
//}

int halide_error_explicit_bounds_too_small(void *user_context, const char *func_name, const char *var_name,
                                                      int min_bound, int max_bound, int min_required, int max_required) {
  assert(false);
  return 0;
}

int halide_error_bad_type(void *user_context, const char *func_name,
                                 uint32_t type_given, uint32_t correct_type)// N.B. The last two args are the bit representation of a halide_type_t {
{
  std::cerr << "Expected type " << correct_type << ", but got: " << type_given << std::endl;
  return -1;
}

int halide_error_bad_dimensions(void *user_context, const char *func_name,
                                       int32_t dimensions_given, int32_t correct_dimensions) {
  std::cerr << "Error (Bad Dimensions): In " << func_name << ", expecting " << correct_dimensions << ", but got " << dimensions_given << std::endl;
  assert(false);
  return 0;
}

int halide_error_access_out_of_bounds(void *user_context, const char *func_name,
                                             int dimension, int min_touched, int max_touched,
                                             int min_valid, int max_valid) {
  std::cerr << "Error (Access out of bounds): in " << func_name << ", dimension: " << dimension << ", min_touched = " << min_touched << " " <<
    "max_touched = " << max_touched << " " << 
    "min_valid = " << min_valid << " " <<
    "max_valid = " << max_valid << std::endl;
  return -1;
}
int halide_error_buffer_allocation_too_large(void *user_context, const char *buffer_name,
                                                    uint64_t allocation_size, uint64_t max_size) {
  assert(false);
  return 0;
}
int halide_error_buffer_extents_negative(void *user_context, const char *buffer_name, int dimension, int extent) {
  assert(false);
  return 0;
}

int halide_error_buffer_extents_too_large(void *user_context, const char *buffer_name,
                                                 int64_t actual_size, int64_t max_size) {
  assert(false);
  return 0;
}
//extern int halide_error_constraints_make_required_region_smaller(void *user_context, const char *buffer_name,
                                                                 //int dimension,
                                                                 //int constrained_min, int constrained_extent,
                                                                 //int required_min, int required_extent);
int halide_error_constraint_violated(void *user_context, const char *var, int val,
                                            const char *constrained_var, int constrained_val) {
  assert(false);
  return 0;
}
//extern int halide_error_param_too_small_i64(void *user_context, const char *param_name,
                                            //int64_t val, int64_t min_val);
//extern int halide_error_param_too_small_u64(void *user_context, const char *param_name,
                                            //uint64_t val, uint64_t min_val);
//extern int halide_error_param_too_small_f64(void *user_context, const char *param_name,
                                            //double val, double min_val);
//extern int halide_error_param_too_large_i64(void *user_context, const char *param_name,
                                            //int64_t val, int64_t max_val);
//extern int halide_error_param_too_large_u64(void *user_context, const char *param_name,
                                            //uint64_t val, uint64_t max_val);
//extern int halide_error_param_too_large_f64(void *user_context, const char *param_name,
                                            //double val, double max_val);
int halide_error_out_of_memory(void *user_context) {
  assert(false);
  return 0;
}
int halide_error_buffer_argument_is_null(void *user_context, const char *buffer_name) {
  assert(false);
  return 0;
}
//extern int halide_error_debug_to_file_failed(void *user_context, const char *func,
                                             //const char *filename, int error_code);
//extern int halide_error_unaligned_host_ptr(void *user_context, const char *func_name, int alignment);
int halide_error_host_is_null(void *user_context, const char *func_name) {
  assert(false);
  return 0;
}
//extern int halide_error_failed_to_upgrade_buffer_t(void *user_context,
                                                   //const char *input_name,
                                                   //const char *reason);
//extern int halide_error_failed_to_downgrade_buffer_t(void *user_context,
                                                     //const char *input_name,
                                                     //const char *reason);
//extern int halide_error_bad_fold(void *user_context, const char *func_name, const char *var_name,
                                 //const char *loop_name);
//extern int halide_error_bad_extern_fold(void *user_context, const char *func_name,
                                        //int dim, int min, int extent, int valid_min, int fold_factor);

//extern int halide_error_fold_factor_too_small(void *user_context, const char *func_name, const char *var_name,
                                              //int fold_factor, const char *loop_name, int required_extent);
//extern int halide_error_requirement_failed(void *user_context, const char *condition, const char *message);
//extern int halide_error_specialize_fail(void *user_context, const char *message);
//extern int halide_error_no_device_interface(void *user_context);
//extern int halide_error_device_interface_no_device(void *user_context);
//extern int halide_error_host_and_device_dirty(void *user_context);
//extern int halide_error_buffer_is_null(void *user_context, const char *routine);
//extern int halide_error_integer_division_by_zero(void *user_context);
void halide_error(void *user_context, const char *) {
  assert(false);
}

static inline size_t halide_malloc_alignment() {
  return 64;
}

void *halide_malloc(void *user_context, size_t x) {
  std::cout << "Mallocing: " << x << std::endl;
  //return malloc(x);
  
  // Allocate enough space for aligning the pointer we return.
  const size_t alignment = halide_malloc_alignment();
  void *orig = malloc(x + alignment);
  if (orig == NULL) {
    // Will result in a failed assertion and a call to halide_error
    return NULL;
  }
  // We want to store the original pointer prior to the pointer we return.
  void *ptr = (void *)(((size_t)orig + alignment + sizeof(void*) - 1) & ~(alignment - 1));
  ((void **)ptr)[-1] = orig;
  return ptr;
}

void halide_free(void *user_context, void *ptr) {
  std::cout << "Freeing pointer: " << ptr << std::endl;
  free(((void**)ptr)[-1]);
  //free(ptr);
}

//template<typename T>
//void *_halide_buffer_get_host(T& tp) {
  //return tp->
    //return (void*) (&tp);
//}


// CGRA Specific code
class CGRAWrapper {
  public:

    CoreIR::Context* c;
    CoreIR::Module* m;
    CoreIR::SimulatorState* state;
    std::vector<uint16_t> pixelOutputs;
    std::string inputName;

    CGRAWrapper();
    ~CGRAWrapper();

    void produce_subimage(halide_buffer_t* sourceBuf, int32_t sourceOffset,
        int src_stride_0, int src_subimage_extend_0,
        int src_stride_1, int src_subimage_extend_1,
        int src_stride_2, int src_subimage_extend_2,
        int src_stride_3, int src_subimage_extend_3,
        
        halide_buffer_t* destBuf, int32_t destOffset,
        int dest_stride_0, int dest_subimage_extend_0,
        int dest_stride_1, int dest_subimage_extend_1,
        int dest_stride_2, int dest_subimage_extend_2,
        int dest_stride_3, int dest_subimage_extend_3);
    
    void subimage_to_stream(halide_buffer_t* buf, int32_t offset,
        int stride_0 = 1, int subimage_extend_0 = 1,
        int stride_1 = 1, int subimage_extend_1 = 1,
        int stride_2 = 1, int subimage_extend_2 = 1,
        int stride_3 = 1, int subimage_extend_3 = 1);

    void stream_to_subimage(halide_buffer_t* buf, int32_t offset,
        int stride_0 = 1, int subimage_extend_0 = 1,
        int stride_1 = 1, int subimage_extend_1 = 1,
        int stride_2 = 1, int subimage_extend_2 = 1,
        int stride_3 = 1, int subimage_extend_3 = 1);
};
