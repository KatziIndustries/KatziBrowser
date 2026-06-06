mkdir -p build
gcc win/tab.cpp tab/*.cpp -o build/tab.exe -mwindows -luser32 -lgdi32 -lws2_32 -lstdc++
echo "[Katzi] Katzi-Tab Compiled"
gcc win/main.cpp -o katzi -mwindows -luser32 -lgdi32 -lws2_32 -lstdc++
echo "[Katzi] Finished! <Enter>"
read

case "$(uname -s)" in
    MINGW*|MSYS*|CYGWIN*)
        read
        ;;
esac