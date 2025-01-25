# Emscripten

## Build and link

emcc -fno-rtti -fno-exceptions -flto -O3 *.c -o index.html -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sSDL2_IMAGE_FORMATS='["jpg"]' -sUSE_SDL_MIXER=2 -sSDL2_MIXER_FORMATS='["mod","wav"]' -sASYNCIFY -sASYNCIFY_ONLY=["main","SDL_Delay","SDL_RenderPresent","GLES2_RenderPresent","Emscripten_GLES_SwapWindow","dynCall_v"] -sASYNCIFY_IGNORE_INDIRECT -sENVIRONMENT=web --preload-file data/ -Wl,-u,fileno --closure 1 -sEXPORTED_RUNTIME_METHODS=['allocate']
