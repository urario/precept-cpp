# Examples are documentation that has to keep compiling. They are built and run as part of an
# ordinary test run so that an API change cannot leave them behind, and they link nothing beyond
# Precept::Precept — an example shows what a consumer writes, not how the library is tested.
function(precept_add_example name)
  add_executable(precept_example_${name} ${name}.cpp)
  target_link_libraries(precept_example_${name} PRIVATE Precept::Precept)
  target_compile_options(
    precept_example_${name}
    PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4;/permissive-;/EHsc>
            $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall;-Wextra;-Wpedantic>)
  add_test(NAME example_${name} COMMAND precept_example_${name})
endfunction()

precept_add_example(packet_parsing)
precept_add_example(fixed_block_processing)
precept_add_example(non_empty_processing)
precept_add_example(aligned_buffer_processing)
precept_add_example(index_below_fixed_tables)
precept_add_example(nonzero_scaling)
precept_add_example(set_once_configuration)
precept_add_example(narrow_exact_wire_fields)
precept_add_example(non_overlapping_buffers)
