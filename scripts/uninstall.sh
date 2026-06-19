#!/bin/bash

echo "УДАЛЕНИЕ CRYPTUM"

INSTALL_DIR="/usr/local/lib/cryptum"
LAUNCHER="/usr/local/bin/cryptum"

echo "Удаление команды cryptum..."
sudo rm -f "$LAUNCHER"

if [ $? -eq 0 ]; then
    echo "Команда cryptum удалена из /usr/local/bin/"
else
    echo "ПРЕДУПРЕЖДЕНИЕ: не удалось удалить $LAUNCHER"
fi

echo "Удаление файлов программы..."
sudo rm -rf "$INSTALL_DIR"

if [ $? -eq 0 ]; then
    echo "Файлы программы удалены из $INSTALL_DIR"
else
    echo "ПРЕДУПРЕЖДЕНИЕ: не удалось удалить $INSTALL_DIR"
fi

echo "УДАЛЕНИЕ ЗАВЕРШЕНО"
