import os
import sys
import argparse
from elftools.elf.elffile import ELFFile

def get_global_functions(file_path):
    functions = []
    try:
        with open(file_path, 'rb') as f:
            elffile = ELFFile(f)
            # Получаем секцию символов
            symtab = elffile.get_section_by_name('.symtab')
            if symtab is None:
                print(f"No symbol table found in '{file_path}'.", file=sys.stderr)
                return functions
            
            # Ищем глобальные функции
            for symbol in symtab.iter_symbols():
                if symbol['st_info']['bind'] == 'STB_GLOBAL':
                    # print(symbol.name, symbol['st_info'])
                    # if symbol['st_info']['type'] == 'STT_FUNC' and symbol['st_shndx'] != 'SHN_UNDEF':
                    functions.append(symbol.name)
    except FileNotFoundError:
        print(f"File '{file_path}' not found.", file=sys.stderr)
    except Exception as e:
        print(f"Error processing file '{file_path}': {e}", file=sys.stderr)
    
    return functions

def main():
    # Настройка argparse для обработки аргументов командной строки
    parser = argparse.ArgumentParser(description="Extract global functions from ELF files.")
    parser.add_argument('files', metavar='FILE', type=str, nargs='+', 
                        help='Paths to ELF files to analyze')
    parser.add_argument('-v', '--verbose', action='store_true', 
                        help='Verbose output of found functions')

    args = parser.parse_args()

    all_functions = {}

    # Обрабатываем каждый файл
    for file_path in args.files:
        if os.path.isfile(file_path):
            functions = get_global_functions(file_path)
            all_functions[file_path] = functions
            if args.verbose:
                print(f"Global functions in '{file_path}': {', '.join(functions) if functions else 'None'}")
        else:
            print(f"Warning: '{file_path}' is not a valid file.")

    # Если verbose не включен, можно вывести общее количество функций
    if not args.verbose:
        for file_path, functions in all_functions.items():
            print(f"File '{file_path}' has {len(functions)} global functions.")

if __name__ == "__main__":
    main()
