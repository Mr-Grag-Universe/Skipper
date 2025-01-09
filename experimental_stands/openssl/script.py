def filter_commands(makefile_path, substr):
    mont_commands = []
    with open(makefile_path, 'r') as file:
        command = ""
        for line in file:
            if substr in line:
                mont_commands.append(line)
            # line = line.strip()
            # if line.endswith('\\'):  # Если строка заканчивается на \
            #     command += line[:-1].strip() + " "  # Убираем \ и добавляем к команде
            # else:
            #     command += line.strip()  # Добавляем последнюю строку команды
            #     if "mont" in command:  # Проверяем, содержит ли команда "mont"
            #         mont_commands.append(command.strip())
            #     command = ""  # Сбрасываем команду для следующей
    return mont_commands

# Укажите путь к вашему Make-файлу
makefile_path = 'openssl/make.logs'
mont_commands = filter_commands(makefile_path, "mont.pl")
print(len(mont_commands))
print('\n'.join(mont_commands))

makefile_path = 'openssl/make.logs'
mont_commands = filter_commands(makefile_path, "x86_64-mont.s")
print(len(mont_commands))
print('\n'.join(mont_commands))
