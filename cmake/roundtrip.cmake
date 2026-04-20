# Round-trip integration test: encrypt then decrypt and compare with the original.
# Variables passed in via -D:
#   CRYFA    – path to the cryfa executable
#   PASS     – path to the key/passphrase file
#   INPUT    – path to the input file (example/in.fq)
#   WORKDIR  – scratch directory (created fresh each run)

file(REMOVE_RECURSE "${WORKDIR}")
file(MAKE_DIRECTORY "${WORKDIR}")

set(ENCRYPTED "${WORKDIR}/in.fq.crf")
set(DECRYPTED "${WORKDIR}/in.fq.dec")

# Encrypt
execute_process(
    COMMAND "${CRYFA}" -k "${PASS}" "${INPUT}"
    OUTPUT_FILE "${ENCRYPTED}"
    RESULT_VARIABLE rc
)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "cryfa encryption failed (exit code ${rc})")
endif()

# Decrypt
execute_process(
    COMMAND "${CRYFA}" -k "${PASS}" -d "${ENCRYPTED}"
    OUTPUT_FILE "${DECRYPTED}"
    RESULT_VARIABLE rc
)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "cryfa decryption failed (exit code ${rc})")
endif()

# Compare
execute_process(
    COMMAND "${CMAKE_COMMAND}" -E compare_files "${INPUT}" "${DECRYPTED}"
    RESULT_VARIABLE rc
)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "Round-trip mismatch: decrypted output differs from original input")
endif()

message(STATUS "Round-trip test passed.")
