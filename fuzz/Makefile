# Константы
APP_NAME = fuzz_app  # Задайте имя вашего приложения
ASM_FILES = factorial.asm # $(wildcard *.asm)
CPP_FILES = fuzz.cpp # $(wildcard *.cpp)
ASM_OBJ_FILES = $(ASM_FILES:.asm=.o)
OBJ_FILES = $(ASM_OBJ_FILES) $(CPP_FILES:.cpp=.o)
LL_FILES = $(CPP_FILES:.cpp=.ll)
DISASM_FILES = $(ASM_FILES:.asm=.dis)

# Папки для сборки
BUILD_DIR = build
BIN_DIR = $(BUILD_DIR)/bin
LL_DIR = $(BUILD_DIR)/ll
O_DIR = $(BUILD_DIR)/object
D_DIR = $(BUILD_DIR)/disasm

# Опции компиляции
CLANG_FLAGS = -g -O1 -std=c++17
NASM_FLAGS = -f elf64
# Параметры линковки
LINK_FLAGS = -rdynamic -L../bin/ -l:libbn256.a

# Проверка аргумента pack
ifeq ($(pack), 1)
    # Создание папок, если они не существуют
    $(shell mkdir -p $(BIN_DIR) $(LL_DIR) $(O_DIR) $(D_DIR))
endif

# Правила сборки
all: $(OBJ_FILES) $(LL_FILES) $(DISASM_FILES) $(APP_NAME)

# Сборка .o файлов из .asm
%.o: %.asm
	$(info "objects files (from asm) compilation...")
	nasm $(NASM_FLAGS) $< -o $@


# Сборка .ll файлов из .cpp
%.ll: %.cpp
	$(info "ll files (from cpp) compilation...")
	clang $(CLANG_FLAGS) -S -emit-llvm $< -o $@

# Сборка .o файлов из .cpp
%.o: %.cpp
	echo "objects files (from cpp) compilation..."
	clang $(CLANG_FLAGS) -fsanitize=fuzzer -c $< -o $@

%.dis: %.asm
	$(info "asm llvm disasm files generation...")
	$(info $*)
	llvm-objdump -d $*.o -S > $(D_DIR)/$*.dis

# @for file in $(ASM_OBJ_FILES); do \
# 	llvm-objdump -d $$file -S > $(D_DIR)/$(basename $$file).dis; \
# done

# Сборка приложения
$(APP_NAME): $(OBJ_FILES)
	clang -fsanitize=fuzzer $(OBJ_FILES) $(LINK_FLAGS) -o $(BIN_DIR)/$(APP_NAME)

# Распределение файлов по папкам
ifneq ($(pack), 0)
	$(info packing...)
	@for file in $(OBJ_FILES); do \
		mv "$$file" "$(O_DIR)/"; \
	done
	@for file in $(LL_FILES); do \
		mv "$$file" "$(LL_DIR)/"; \
	done
endif

# Очистка
clean:
	rm -rf $(OBJ_FILES) $(LL_FILES) $(DISASM_FILES) $(BIN_DIR)/$(APP_NAME) $(BUILD_DIR)

.PHONY: all clean
