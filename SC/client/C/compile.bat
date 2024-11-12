gcc -o websocket websocket.c -LC:/msys64/mingw64/lib -lwebsockets_static -lssl -lcrypto -lz -lws2_32 -lcrypt32 -static

pause
cls
websocket.exe
pause
exit