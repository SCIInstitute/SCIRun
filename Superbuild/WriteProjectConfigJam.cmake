if(DEFINED EXE AND EXE AND DEFINED VERSION AND VERSION)
  file(WRITE "${OUTPUT_FILE}"
"using python : ${VERSION} : ${EXE} : ${INCLUDE} : ${LIBDIR} ;
")
else()
  # Explicitly write an empty or comment-only config
  file(WRITE "${OUTPUT_FILE}"
"# Python disabled — no python toolset configured\n")
endif()