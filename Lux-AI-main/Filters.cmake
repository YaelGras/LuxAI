FUNCTION(POPULATE_TREE files targetdir)

	FOREACH(file ${files})

	    GET_FILENAME_COMPONENT(dir "${file}" DIRECTORY)

	    SET(target_group "${targetdir}\\${dir}")
	    STRING(REPLACE / \\ target_group ${target_group})

	    SOURCE_GROUP("${target_group}" FILES ${file})
	    
	ENDFOREACH(file)
ENDFUNCTION(POPULATE_TREE) 

FUNCTION(CREATE_VISUAL_STUDIO_FILTERS target)
  POPULATE_TREE("${${target}_HEADERS}" "Headers")
  POPULATE_TREE("${${target}_SRC}" "Sources")
ENDFUNCTION(CREATE_VISUAL_STUDIO_FILTERS)