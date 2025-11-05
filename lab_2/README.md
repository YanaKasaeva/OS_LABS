gcc -o monte_carlo monte_carlo.c -lpthread -lm
./monte_carlo 5.0 1000000 4


docker run -it --rm -v $(pwd):/app ubuntu:latest bash -c "
apt-get update && apt-get install -y gcc strace &&
cd /app &&
gcc -o monte_carlo monte_carlo.c -lpthread -lm &&
strace -f -o strace.log ./monte_carlo 5.0 10000000 4
"


docker run -it --rm -v $(pwd):/app ubuntu:latest bash -c "
apt-get update && apt-get install -y gcc procps &&
cd /app &&
gcc -o monte_carlo monte_carlo.c -lpthread -lm &&
(./monte_carlo 5.0 1000000000 4 &); sleep 1; ps -eLf | grep monte_carlo > threads.log; sleep 1; ps -eLf | grep monte_carlo >> threads.log
cat threads.log
"


echo "Исследование зависимости от количества потоков:"
for threads in 1 2 4 8; do
echo "Потоков: $threads"
./monte_carlo 5.0 1000000 $threads
echo
done

echo "Исследование зависимости от объема данных:"
for points in 1000000 10000000 100000000; do
echo "Точек: $points"
./monte_carlo 5.0 $points 4
echo
done