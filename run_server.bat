@echo off
echo Compiling Backend Server...
g++ -c server.cpp
g++ -c library.cpp
g++ server.o library.o -o server_app.exe -lws2_32
if %errorlevel% neq 0 ( exit /b %errorlevel% )

echo Running Backend Server...
.\server_app.exe
