install(
    TARGETS stamd_exe
    RUNTIME COMPONENT stamd_Runtime
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
