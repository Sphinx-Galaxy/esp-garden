set(srcs "garden_main.c"
	"display.c"
    )

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ".")
