# Configure clang-tidy to run during compilation
# This will show warnings in VS Code Problems panel during build
find_program(CLANG_TIDY_EXE NAMES "clang-tidy")

if(CLANG_TIDY_EXE)
	set(USING_CLANG_TIDY ON)

	# clang-tidy reads configuration from .clang-tidy file
	# The --header-filter overrides the HeaderFilterRegex from .clang-tidy
	# to ensure ONLY project files are analyzed (not Qt, system headers, etc)
	set(CMAKE_CXX_CLANG_TIDY 
		"${CLANG_TIDY_EXE}"
		"--header-filter=.*/pgmodeler/(apps|libs|priv-plugins)/.*\\.(h|hpp|cpp)$"
	)
endif()
