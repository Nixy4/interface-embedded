set(SRCS_DIR "${CMAKE_CURRENT_LIST_DIR}/../src")

if(CONFIG_INTERFACE_EMBEDDED_ENABLED)
    list(APPEND INCLUDE_DIRS "${SRCS_DIR}")

    # --- Logger (independent from OSAL) ---
    if(CONFIG_INTERFACE_EMBEDDED_LOGGER_ENABLED)
        if(CONFIG_INTERFACE_EMBEDDED_LOGGER_ESP_LOG)
            list(APPEND REQUIRES log)
        endif()
        # printf backend needs no extra deps
        # easylogger: future — add REQUIRES here
    endif()

    # --- OSAL ---
    if(CONFIG_INTERFACE_EMBEDDED_OSAL_ENABLED)
        if(CONFIG_INTERFACE_EMBEDDED_OSAL_FREERTOS)
            # FreeRTOS headers come from IDF's freertos component (auto-linked)
        elseif(CONFIG_INTERFACE_EMBEDDED_OSAL_CMSIS_RTOS2)
        elseif(CONFIG_INTERFACE_EMBEDDED_OSAL_POSIX)
        elseif(CONFIG_INTERFACE_EMBEDDED_OSAL_CPP_STD)
        endif()
    endif()
endif()

idf_component_register(
  SRCS ""
  INCLUDE_DIRS ${INCLUDE_DIRS}
  REQUIRES ${REQUIRES}
)