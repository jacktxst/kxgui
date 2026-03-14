emcc main.c -I . -I sokol -I kxgui -s USE_WEBGL2=1 -s FULL_ES3=1 -s ALLOW_MEMORY_GROWTH=1 -DSOKOL_GLES3 -DSOKOL_IMPL --shell-file shell.html -o docs/wasm-demo.html
