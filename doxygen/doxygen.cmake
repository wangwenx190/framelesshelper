set(DOXYGEN_FILE_DIR ${CMAKE_CURRENT_LIST_DIR})

#[[
Add Doxygen target.

    ck_init_buildsystem(<target>
        [NAME           <name>]
        [VERSION        <version>]
        [DESCRIPTION    <desc>]
        [LOGO           <file>]
        [MDFILE         <file>]
        [OUTPUT_DIR     <dir>]
        
        [INPUT                  <file> ...]
        [INCLUDE_DIRECTORIES    <dir> ...]
        [COMPILE_DEFINITIONS    <NAME=VALUE> ...]
        [TARGETS                <target> ...]
        [ENVIRONMENT_EXPORTS    <key> ...]
        [NO_EXPAND_MACROS       <macro> ...]
        [DEPENDS                <dependency> ...]
    )
]] #
function(setup_doxygen_command _target)
    set(options)
    set(oneValueArgs NAME VERSION DESCRIPTION LOGO MDFILE OUTPUT_DIR INSTALL_DIR GENERATE_TAGFILE)
    set(multiValueArgs INPUT TAGFILES INCLUDE_DIRECTORIES COMPILE_DEFINITIONS TARGETS ENVIRONMENT_EXPORTS
        NO_EXPAND_MACROS DEPENDS
    )
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT DOXYGEN_EXECUTABLE)
        message(FATAL_ERROR "setup_doxygen_command: doxygen executable not defined!")
    endif()

    macro(_set_value _key _maybe_value _default)
        if(${_maybe_value})
            set(${_key} ${${_maybe_value}})
        else()
            set(${_key} ${_default})
        endif()
    endmacro()

    _set_value(_name FUNC_NAME "${PROJECT_NAME}")
    _set_value(_version FUNC_VERSION "${PROJECT_VERSION}")
    _set_value(_desc FUNC_DESCRIPTION "${PROJECT_DESCRIPTION}")
    _set_value(_logo FUNC_LOGO "")
    _set_value(_mdfile FUNC_MDFILE "")
    _set_value(_tagfile FUNC_GENERATE_TAGFILE "")

    if(_desc STREQUAL "")
        set(${_desc} "${_name}")
    endif()

    set(_sep " \\\n    ")

    # Generate include file
    set(_doxy_includes "${CMAKE_CURRENT_BINARY_DIR}/cmake/doxygen_${_target}.inc")
    set(_doxy_output_dir "${CMAKE_CURRENT_BINARY_DIR}/doxygen_${_target}")

    set(_input "")
    set(_tagfiles "")
    set(_includes "")
    set(_defines "")
    set(_no_expand "")

    if(FUNC_INPUT)
        set(_input "INPUT = $<JOIN:${FUNC_INPUT},${_sep}>\n\n")
    else()
        set(_input "INPUT = \n\n")
    endif()

    if(FUNC_TAGFILES)
        set(_tagfiles "TAGFILES = $<JOIN:${FUNC_TAGFILES},${_sep}>\n\n")
    else()
        set(_tagfiles "TAGFILES = \n\n")
    endif()

    if(FUNC_INCLUDE_DIRECTORIES)
        set(_includes "INCLUDE_PATH = $<JOIN:${FUNC_INCLUDE_DIRECTORIES},${_sep}>\n\n")
    else()
        set(_includes "INCLUDE_PATH = \n\n")
    endif()

    if(FUNC_COMPILE_DEFINITIONS)
        set(_defines "PREDEFINED = $<JOIN:${FUNC_COMPILE_DEFINITIONS},${_sep}>\n\n")
    else()
        set(_defines "PREDEFINED = \n\n")
    endif()

    if(FUNC_NO_EXPAND_MACROS)
        set(_temp_list)

        foreach(_item ${FUNC_NO_EXPAND_MACROS})
            list(APPEND _temp_list "${_item}=")
        endforeach()

        set(_no_expand "PREDEFINED += $<JOIN:${_temp_list},${_sep}>\n\n")
        unset(_temp_list)
    endif()

    # Extra
    set(_extra_arguments)

    if(FUNC_TARGETS)
        foreach(item ${FUNC_TARGETS})
            set(_extra_arguments "${_extra_arguments}INCLUDE_PATH += $<JOIN:$<TARGET_PROPERTY:${item},INCLUDE_DIRECTORIES>,${_sep}>\n\n")
            set(_extra_arguments "${_extra_arguments}PREDEFINED += $<JOIN:$<TARGET_PROPERTY:${item},COMPILE_DEFINITIONS>,${_sep}>\n\n")
        endforeach()
    endif()

    if(FUNC_OUTPUT_DIR)
        set(_doxy_output_dir ${FUNC_OUTPUT_DIR})
    endif()

    if(_mdfile)
        set(_extra_arguments "${_extra_arguments}INPUT += ${_mdfile}\n\n")
    endif()

    file(GENERATE
        OUTPUT "${_doxy_includes}"
        CONTENT "${_input}${_tagfiles}${_includes}${_defines}${_extra_arguments}${_no_expand}"
    )

    set(_env)

    foreach(_export ${FUNC_ENVIRONMENT_EXPORTS})
        if(NOT DEFINED "${_export}")
            message(FATAL_ERROR "setup_doxygen_command: ${_export} is not known when trying to export it.")
        endif()

        list(APPEND _env "${_export}=${${_export}}")
    endforeach()

    list(APPEND _env "DOXY_FILE_DIR=${DOXYGEN_FILE_DIR}")
    list(APPEND _env "DOXY_INCLUDE_FILE=${_doxy_includes}")

    list(APPEND _env "DOXY_PROJECT_NAME=${_name}")
    list(APPEND _env "DOXY_PROJECT_VERSION=${_version}")
    list(APPEND _env "DOXY_PROJECT_BRIEF=${_desc}")
    list(APPEND _env "DOXY_PROJECT_LOGO=${_logo}")
    list(APPEND _env "DOXY_MAINPAGE_MD_FILE=${_mdfile}")
    list(APPEND _env "DOXY_GENERATE_TAGFILE=${_tagfile}")

    set(_build_command "${CMAKE_COMMAND}" "-E" "env"
        ${_env} "DOXY_OUTPUT_DIR=${_doxy_output_dir}"
        "${DOXYGEN_EXECUTABLE}" "${DOXYGEN_FILE_DIR}/Doxyfile"
    )

    if(FUNC_DEPENDS)
        set(_dependencies DEPENDS ${FUNC_DEPENDS})
    endif()

    add_custom_target(${_target}
        ${_build_command}
        COMMENT "Build HTML documentation"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        VERBATIM
        ${_dependencies}
    )

    if(FUNC_INSTALL_DIR AND CMAKE_INSTALL_PREFIX)
        get_filename_component(_install_dir ${FUNC_INSTALL_DIR} ABSOLUTE BASE_DIR ${CMAKE_INSTALL_PREFIX})
        set(_install_command "${CMAKE_COMMAND}" "-E" "env"
            ${_env} "DOXY_OUTPUT_DIR=${_install_dir}"
            "${DOXYGEN_EXECUTABLE}" "${DOXYGEN_FILE_DIR}/Doxyfile"
        )

        set(_install_command_quoted)

        foreach(_item ${_install_command})
            set(_install_command_quoted "${_install_command_quoted}\"${_item}\" ")
        endforeach()

        install(CODE "
            message(STATUS \"Install HTML documentation\")
            file(MAKE_DIRECTORY \"${_install_dir}\")
            execute_process(
                COMMAND ${_install_command_quoted}
                WORKING_DIRECTORY \"${CMAKE_CURRENT_SOURCE_DIR}\"
            )
        ")
    endif()
endfunction()
