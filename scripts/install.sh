#!/bin/bash

echo "УСТАНОВКА CRYPTUM"

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
INSTALL_DIR="/usr/local/lib/cryptum"
LAUNCHER="/usr/local/bin/cryptum"

cd "$ROOT_DIR"

echo "Сборка проекта..."
mkdir -p build
cd build

cmake ..
make -j"$(nproc)"

if [ $? -ne 0 ]; then
    echo "ОШИБКА: сборка не удалась"
    exit 1
fi

echo "Сборка выполнена успешно"

echo "Копирование файлов программы..."
sudo mkdir -p "$INSTALL_DIR"

sudo cp cryptum "$INSTALL_DIR/"
sudo cp lib*.so "$INSTALL_DIR/"

if [ $? -ne 0 ]; then
    echo "ОШИБКА: не удалось скопировать файлы программы"
    exit 1
fi

echo "Создание команды cryptum..."
sudo tee "$LAUNCHER" > /dev/null <<EOF
#!/bin/bash
cd "$INSTALL_DIR"
exec "$INSTALL_DIR/cryptum" "\$@"
EOF

sudo chmod +x "$LAUNCHER"

if [ $? -ne 0 ]; then
    echo "ОШИБКА: не удалось создать команду cryptum"
    exit 1
fi

echo "УСТАНОВКА ЗАВЕРШЕНА"
echo "Для запуска: cryptum --help"
echo "Файлы программы находятся в: $INSTALL_DIR"
