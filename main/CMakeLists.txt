set(srcs "garden_main.c"
	"display.c"
	"lora.c"
    )

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ".")
