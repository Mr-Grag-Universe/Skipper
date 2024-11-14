#!/bin/zsh

# качаем и устанавливаем DynamoRIO
wget https://github.com/DynamoRIO/dynamorio/releases/download/cronbuild-11.0.20034/DynamoRIO-Linux-11.0.20034.tar.gz
tar -xzvf DynamoRIO-Linux-11.0.20034.tar.gz
mv DynamoRIO-Linux-11.0.20034 dynamorio
rm DynamoRIO-Linux-11.0.20034.tar.gz

# качаем json
# git clone --depth 1 https://github.com/nlohmann/json.git
# можно не качать, ибо я в CMakeLists.txt засунул

# обновляем cmake
echo "deb http://deb.debian.org/debian bullseye-backports main" | sudo tee -a /etc/apt/sources.list
sudo apt -t bullseye-backports install cmake -y

# качаем llvm
sudo chmod +x scripts/build_llvm.sh
./scripts/build_llvm.sh