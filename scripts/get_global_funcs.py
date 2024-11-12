import subprocess
import re
import argparse

def extract_global_symbols(object_file):
    # Выполняем команду readelf для получения информации о символах
    try:
        result = subprocess.run(['readelf', '-s', '-W', object_file], capture_output=True, text=True, check=True)
    except subprocess.CalledProcessError as e:
        print(f"Ошибка при выполнении команды: {e}")
        return []

    # Ищем глобальные символы в выводе
    global_symbols = []
    for line in result.stdout.splitlines():
        if 'GLOBAL' in line.split():
            global_symbols.append(line.split()[-1])
        # Используем обновленный шаблон для поиска глобальных символов
        # match = re.match(r'\s*\d+:\s+(\w+)\s+(\w+)\s+(FUNC|OBJECT|NOTYPE)\s+GLOBAL\s+(\w+)\s+(\w+)\s+(.*)', line)
        # if match:
        #     print(match)
        #     name = match.group(4)  # Имя символа
        #     global_symbols.append(name)

    return global_symbols

def main():
    # Настройка парсера аргументов
    parser = argparse.ArgumentParser(description='Извлечение глобальных символов из .o файла.')
    parser.add_argument('object_file', type=str, help='Путь к .o файлу')

    args = parser.parse_args()

    global_symbols = extract_global_symbols(args.object_file)
    
    print("Глобальные символы:")
    for symbol in global_symbols:
        print(symbol)

if __name__ == "__main__":
    main()
