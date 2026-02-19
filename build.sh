cd "$(dirname "$0")" # перейти в директорию(папку) с данным скриптом
rm -rf build # удалить папку build, если она существует
mkdir build # создать папку build
cd build # перейти в папку build
cmake ..
cmake --build .
echo "Press any key to exit..."
read -n1 -s