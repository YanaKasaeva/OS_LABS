gcc -o child child.c
gcc -o parent parent.c
./parent

docker run -it --rm -v $(pwd):/app ubuntu:latest bash -c "
apt-get update && apt-get install -y gcc strace &&
cd /app &&
gcc -o parent parent.c -lpthread &&
gcc -o child child.c -lpthread &&
strace -f -o strace.log ./parent
"