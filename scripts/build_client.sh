#!/bin/zsh

# Имя папки
FOLDER_NAME="build"

# Проверяем, существует ли папка
if [ ! -d "$FOLDER_NAME" ]; then
    # Создаём папку
    mkdir "$FOLDER_NAME"
    echo "Папка \"$FOLDER_NAME\" была создана."
else
    echo "Папка \"$FOLDER_NAME\" уже существует."
fi


cd build
cmake ../
make
cp bin/libclient.so ../bin/libclient.so
