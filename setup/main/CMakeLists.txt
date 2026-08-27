idf_component_register(SRCS "main.c" "wifi.c" "http-server.c"
	INCLUDE_DIRS "."
	EMBED_TXTFILES web/index.html
)
