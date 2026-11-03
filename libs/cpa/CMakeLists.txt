project (cpa CXX)

add_library(cpa_headers INTERFACE)
target_include_directories (cpa_headers INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:/include>)



add_subdirectory(internal) 

add_subdirectory(concrete) 
add_subdirectory(symbolic) 

# The Pathformula CPA is intentionally excluded from the build. Its implementation
# is legacy and is not kept in sync with the active CPA/VM interfaces. Re-enable
# only after updating it for the current domain-based Engine/MultiEvaluator API,
# reconciling its state/register-store memory contract, and adding build/test
# coverage for that backend.


add_subdirectory (src)

add_library(MiniMC::cpa ALIAS cpa)
