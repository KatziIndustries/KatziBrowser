mkdir -p build
gcc tab/main.cpp -o build/tab -mwindows -luser32 -lgdi32 -lws2_32 -lstdc++
echo "[Katzi] Katzi-Tab Compiled"
gcc win/main.cpp -o katzi -mwindows -luser32 -lgdi32 -lws2_32 -lstdc++
echo "[Katzi] Finished! <Enter>"

case "$(uname -s)" in
    MINGW*|MSYS*|CYGWIN*)
        read
        ;;
esac