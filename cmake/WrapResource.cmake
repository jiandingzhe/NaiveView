function(wrap_resource)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "" "INPUT;PREFIX;NAMESPACE;TARGET;SOURCE_VAR" "")
    set(script ${PROJECT_SOURCE_DIR}/src/spawn_resource.pl)
    set(input_abs ${CMAKE_CURRENT_SOURCE_DIR}/${ARG_INPUT})
    
    set(outputs ${CMAKE_CURRENT_BINARY_DIR}/${ARG_PREFIX}.h ${CMAKE_CURRENT_BINARY_DIR}/${ARG_PREFIX}.cpp)
    set(cmdline ${PERL_EXECUTABLE} ${script} ${input_abs} ${CMAKE_CURRENT_BINARY_DIR} ${ARG_PREFIX})
    if(DEFINED ARG_NAMESPACE)
        list(APPEND cmdline ${ARG_NAMESPACE})
    endif()
    execute_process(COMMAND ${cmdline})
    add_custom_command(
        OUTPUT ${outputs}
        COMMAND ${cmdline}
        DEPENDS ${script} ${input_abs}
    )
    if(DEFINED ARG_TARGET)
        add_custom_target(${ARG_TARGET} DEPENDS ${outputs})
    endif()
    if(DEFINED ARG_SOURCE_VAR)
        set(re_tmp ${${ARG_SOURCE_VAR}})
        list(APPEND re_tmp ${outputs})
        set(${ARG_SOURCE_VAR} ${re_tmp} PARENT_SCOPE)
    endif()
endfunction()