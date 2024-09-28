# Используем образ Fedora
FROM fedora:39
# latest

# Создаем рабочую директорию
RUN mkdir /app

# Копируем файлы и каталог
COPY ./ /app/
# Устанавливаем необходимые зависимости (если нужны)

RUN dnf -y update
RUN dnf -y upgrade
RUN dnf install -y clang cmake g++ doxygen git zlib-devel libunwind-devel snappy-devel lz4-devel

# Очищаем кэш DNF
RUN dnf clean all

WORKDIR /app

# Команда на исполнение
# CMD [".DynamoRIO-Linux/bin64/drrun", "-c", "./bin/libclient.so", "--", "./bin/fuzz_app", "-max_len=64", "-len_control=1", "out/corpus"]

