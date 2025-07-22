set -xe

gcc -o app main.c list.c -Wall -Wextra -Wpedantic -fsanitize=address,undefined -g -std=c11
./app