set(NANORT_DIRS 
	~/lib/nanort
	"F:/local/nanort"
	"C:/local/nanort")

find_path(NANORT_INCLUDE_DIRS
	NAMES
		nanort.h
	PATHS
		${NANORT_DIRS}
)

