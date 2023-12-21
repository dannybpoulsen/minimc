project (smt CXX)
add_subdirectory (src)



add_library(MiniMC::smt ALIAS smt)
install (TARGETS smt EXPORT MiniMCTargets FILE_SET HEADERS
)
