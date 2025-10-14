gcc -o parent parent.c
gcc -o child child.c
./parent


docker run -it --rm -v $(pwd)/src:/app/src ubuntu:latest bash -c "
    apt-get update && apt-get install -y gcc strace &&
    cd /app/src &&
    gcc -o parent parent.c &&
    gcc -o child child.c &&
    strace -f -o strace.log ./parent
"