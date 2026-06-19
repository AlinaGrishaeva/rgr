#!/bin/bash

echo "УДАЛЕНИЕ CRYPTUM"

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "Удаление исполняемого файла..."
sudo rm -f /usr/local/bin/cryptum

if [ $? -eq 0 ]; then
    echo "cryptum удалён из /usr/local/bin/"
else
    echo "ПРЕДУПРЕЖДЕНИЕ: не удалось удалить /usr/local/bin/cryptum"
fi

echo "Удаление библиотек..."
sudo rm -rf /usr/local/lib/cryptum

if [ $? -eq 0 ]; then
    echo "Библиотеки удалены из /usr/local/lib/cryptum/"
else
    echo "ПРЕДУПРЕЖДЕНИЕ: не удалось удалить /usr/local/lib/cryptum/"
fi

echo "Очистка build директории..."
rm -rf "$ROOT_DIR/build"

echo "УДАЛЕНИЕ ЗАВЕРШЕНО"
echo "Программа cryptum удалена из системы"
