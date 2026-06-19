#!/bin/bash

echo "УСТАНОВКА CRYPTUM"

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

cd "$ROOT_DIR"

echo "Сборка проекта..."
mkdir -p build
cd build

cmake ..
make -j$(nproc)

if [ $? -eq 0 ]; then
    echo "Сборка выполнена успешно"
else
    echo "ОШИБКА: сборка не удалась"
    exit 1
fi

echo "Копирование исполняемого файла..."
sudo cp cryptum /usr/local/bin/

if [ $? -eq 0 ]; then
    echo "cryptum установлен в /usr/local/bin/"
else
    echo "ОШИБКА: не удалось скопировать файл"
    exit 1
fi

echo "Копирование библиотек..."
sudo mkdir -p /usr/local/lib/cryptum
sudo cp lib*.so /usr/local/lib/cryptum/

if [ $? -eq 0 ]; then
    echo "Библиотеки установлены в /usr/local/lib/cryptum/"
else
    echo "ОШИБКА: не удалось скопировать библиотеки"
    exit 1
fi

echo "УСТАНОВКА ЗАВЕРШЕНА"
echo "Для запуска: cryptum --help"
echo "Библиотеки находятся в: /usr/local/lib/cryptum/"
