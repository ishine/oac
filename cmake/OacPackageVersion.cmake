if(__oac_version)
  return()
endif()
set(__oac_version INCLUDED)

function(get_package_version PACKAGE_VERSION PROJECT_VERSION)

  find_package(Git)
  if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_LIST_DIR}/.git")
    execute_process(COMMAND ${GIT_EXECUTABLE}
                    --git-dir=${CMAKE_CURRENT_LIST_DIR}/.git describe
                    --tags --match "v*" OUTPUT_VARIABLE OAC_PACKAGE_VERSION)
    if(OAC_PACKAGE_VERSION)
      string(STRIP ${OAC_PACKAGE_VERSION}, OAC_PACKAGE_VERSION)
      string(REPLACE \n
                     ""
                     OAC_PACKAGE_VERSION
                     ${OAC_PACKAGE_VERSION})
      string(REPLACE ,
                     ""
                     OAC_PACKAGE_VERSION
                     ${OAC_PACKAGE_VERSION})

      string(SUBSTRING ${OAC_PACKAGE_VERSION}
                       1
                       -1
                       OAC_PACKAGE_VERSION)
      message(STATUS "Oac package version from git repo: ${OAC_PACKAGE_VERSION}")
    endif()

  elseif(EXISTS "${CMAKE_CURRENT_LIST_DIR}/package_version"
         AND NOT OAC_PACKAGE_VERSION)
    # Not a git repo, lets' try to parse it from package_version file if exists
    file(STRINGS package_version OAC_PACKAGE_VERSION
         LIMIT_COUNT 1
         REGEX "PACKAGE_VERSION=")
    string(REPLACE "PACKAGE_VERSION="
                   ""
                   OAC_PACKAGE_VERSION
                   ${OAC_PACKAGE_VERSION})
    string(REPLACE "\""
                   ""
                   OAC_PACKAGE_VERSION
                   ${OAC_PACKAGE_VERSION})
    # In case we have a unknown dist here we just replace it with 0
    string(REPLACE "unknown"
                   "0"
                   OAC_PACKAGE_VERSION
                   ${OAC_PACKAGE_VERSION})
      message(STATUS "Oac package version from package_version file: ${OAC_PACKAGE_VERSION}")
  endif()

  if(OAC_PACKAGE_VERSION)
    string(REGEX
      REPLACE "^([0-9]+.[0-9]+\\.?([0-9]+)?).*"
               "\\1"
               OAC_PROJECT_VERSION
               ${OAC_PACKAGE_VERSION})
  else()
    # fail to parse version from git and package version
    message(WARNING "Could not get package version.")
    set(OAC_PACKAGE_VERSION 0)
    set(OAC_PROJECT_VERSION 0)
  endif()

  message(STATUS "Oac project version: ${OAC_PROJECT_VERSION}")

  set(PACKAGE_VERSION ${OAC_PACKAGE_VERSION} PARENT_SCOPE)
  set(PROJECT_VERSION ${OAC_PROJECT_VERSION} PARENT_SCOPE)
endfunction()
