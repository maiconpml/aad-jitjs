# Try to find the CPLEX library

find_path(CPLEX_INCLUDE_DIR ilcplex/cplex.h
    HINTS ${CPLEX_ROOT}/cplex/include
          $ENV{CPLEX_ROOT}/cplex/include
          /opt/ibm/ILOG/CPLEX_Studio*/cplex/include
          /usr/local/include
          /usr/include
)

find_path(CONCERT_INCLUDE_DIR ilconcert/iloenv.h
    HINTS ${CPLEX_ROOT}/concert/include
          $ENV{CPLEX_ROOT}/concert/include
          /opt/ibm/ILOG/CPLEX_Studio*/concert/include
          /usr/local/include
          /usr/include
)

find_library(CPLEX_LIBRARY NAMES cplex cplex1280 cplex1290 cplex12100 cplex2010
    HINTS ${CPLEX_ROOT}/cplex/lib/x86-64_linux/static_pic
          $ENV{CPLEX_ROOT}/cplex/lib/x86-64_linux/static_pic
          /opt/ibm/ILOG/CPLEX_Studio*/cplex/lib/x86-64_linux/static_pic
          /usr/local/lib
          /usr/lib
)

find_library(CONCERT_LIBRARY NAMES concert
    HINTS ${CPLEX_ROOT}/concert/lib/x86-64_linux/static_pic
          $ENV{CPLEX_ROOT}/concert/lib/x86-64_linux/static_pic
          /opt/ibm/ILOG/CPLEX_Studio*/concert/lib/x86-64_linux/static_pic
          /usr/local/lib
          /usr/lib
)

find_library(ILOCPLEX_LIBRARY NAMES ilocplex
    HINTS ${CPLEX_ROOT}/cplex/lib/x86-64_linux/static_pic
          $ENV{CPLEX_ROOT}/cplex/lib/x86-64_linux/static_pic
          /opt/ibm/ILOG/CPLEX_Studio*/cplex/lib/x86-64_linux/static_pic
          /usr/local/lib
          /usr/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CPLEX DEFAULT_MSG
    CPLEX_LIBRARY CPLEX_INCLUDE_DIR
    CONCERT_LIBRARY CONCERT_INCLUDE_DIR
    ILOCPLEX_LIBRARY
)

if(CPLEX_FOUND)
    set(CPLEX_INCLUDE_DIRS ${CPLEX_INCLUDE_DIR} ${CONCERT_INCLUDE_DIR})
    set(CPLEX_LIBRARIES ${ILOCPLEX_LIBRARY} ${CONCERT_LIBRARY} ${CPLEX_LIBRARY})
    # CPLEX often needs pthread and dl
    set(CPLEX_LIBRARIES ${CPLEX_LIBRARIES} pthread dl)
endif()

mark_as_advanced(CPLEX_INCLUDE_DIR CPLEX_LIBRARY CONCERT_INCLUDE_DIR CONCERT_LIBRARY ILOCPLEX_LIBRARY)
