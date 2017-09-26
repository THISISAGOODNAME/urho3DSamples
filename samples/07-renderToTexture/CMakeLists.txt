# Define target name
set (TARGET_NAME 07-renderToTexture)

file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Define source files
define_source_files ()

# Setup target with resource copying
setup_main_executable ()
