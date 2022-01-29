execute_process(
  COMMAND git rev-parse HEAD
  OUTPUT_VARIABLE COMMIT_ID
  OUTPUT_STRIP_TRAILING_WHITESPACE
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

execute_process(
  COMMAND git status --short
  OUTPUT_VARIABLE MODIFIED_FILES
  OUTPUT_STRIP_TRAILING_WHITESPACE
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
string(
  REPLACE
  "\n"
  "\\n\\\n"
  MODIFIED_FILES
  "${MODIFIED_FILES}"
)

string(TIMESTAMP BUILD_TIMESTAMP UTC)

execute_process(
  COMMAND git log -1 "--format=%at"
  OUTPUT_VARIABLE COMMIT_UNIX_TIMESTAMP
  OUTPUT_STRIP_TRAILING_WHITESPACE
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

if("${MODIFIED_FILES}" STREQUAL "")
  set(DIRTY "false")
else()
  set(DIRTY "true")
endif()

configure_file(
  ${INPUT_FILE}
  ${OUTPUT_FILE}
  @ONLY
)