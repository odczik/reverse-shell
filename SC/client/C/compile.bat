gcc -o websocket main.c auth.c tools.c -LC:/msys64/mingw64/lib -lwebsockets_static -lssl -lcrypto -lz -lws2_32 -lcrypt32 -static

pause
cls
websocket.exe
pause
exit