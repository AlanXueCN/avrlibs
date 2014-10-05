# Основная цель.
TARGET    = main

# Объектные файлы.
OBJECTS   = 

# Собственные библиотеки в исходниках.
SRC_LIBS += 

# Библиотеки.
LIBS     += c

# Оптимизация, вторая часть флага компилятора -O.
OPTIMIZE  = s

# Флаги отладки.
DEBUG     =

# МК.
# Семейство МК.
MCU_FAMILY = mega
# Номер МК в семействе.
MCU_NUMBER = 16
# Имя МК.
MCU_NAME   = at$(MCU_FAMILY)$(MCU_NUMBER)
# Частота тактирования.
F_CPU      = 16000000ULL

# Макросы.
# Частота тактирования.
DEFINES   += F_CPU=$(F_CPU)
# Макрос семейства МК.
DEFINES   += __AVR_AT$(MCU_FAMILY)$(MCU_NUMBER)__
# Прочие.
DEFINES   += 

# Программатор
PROGRAMMER = usbasp
# AVRDude
AVRDUDE = /usr/bin/avrdude

# Путь к собственным библиотекам в исходниках.
SRC_LIBS_PATH     = ../lib
# Пути ко всем собственным библиотекам в исходниках.
SRC_LIBS_ALL_PATH = $(wildcard $(addsuffix /*, $(SRC_LIBS_PATH)))

# Пути библиотек.
LIBS_PATH += /opt/cross/avr/avr/lib/avr4

# Флаги МК.
MCUFLAGS  += -mmcu=$(MCU_NAME)

# Пути заголовочных файлов.
INCS     += .
INCS     += $(SRC_LIBS_PATH)

# Пути с исходниками.
VPATH   += .
VPATH   += $(SRC_LIBS_ALL_PATH)

# Объектные файлы.
# Собственные библиотеки в исходниках.
OBJECTS += $(addsuffix .o, $(SRC_LIBS))

# Ассемблер листинги.
ASM  += $(patsubst %.o, %.s, $(OBJECTS))

# Флаги компилятора С.
# Использование каналов.
CFLAGS    += -pipe
# Флаги МК.
CFLAGS    += $(MCUFLAGS)
# Флаги оптимизации.
CFLAGS    += -O$(OPTIMIZE)
# Флаги отладки.
CFLAGS    += $(DEBUG)
# Выводить все предупреждения.
CFLAGS    += -Wall
# Без встроенных функций. (включено в -ffreestanding)
# CFLAGS    += -fno-builtin
# Код будет выполнятся без ОС.
CFLAGS    += -ffreestanding
# Помещать функции в отдельные секции.
CFLAGS    += -ffunction-sections
# Помещать данные в отдельные секции.
CFLAGS    += -fdata-sections
# Пути поиска заголовочных файлов.
CFLAGS    += $(addprefix -I, $(INCS))
# Макросы.
CFLAGS    += $(addprefix -D, $(DEFINES))

# Флаги компоновщика.
# Флаги МК.
LDFLAGS   += $(MCUFLAGS)
# Удаление ненужных секций.
LDFLAGS   += -Wl,--gc-sections
# Генерация карты исполнимого файла.
LDFLAGS   += -Wl,-Map=$(TARGET_MAP),--cref
# Пути поиска библиотек.
LDFLAGS   += $(addprefix -L, $(LIBS_PATH))
# Библиотеки.
LDFLAGS   += $(addprefix -l, $(LIBS))

# Флаги ассемблера.
# Флаги МК.
ASFLAGS   += $(MCUFLAGS)
ASFLAGS   += 

# Тулкит.
TOOLKIT_PREFIX=avr-
AS      = $(TOOLKIT_PREFIX)gcc
CC      = $(TOOLKIT_PREFIX)gcc
LD      = $(TOOLKIT_PREFIX)gcc
OBJCOPY = $(TOOLKIT_PREFIX)objcopy
STRIP   = $(TOOLKIT_PREFIX)strip
SIZE    = $(TOOLKIT_PREFIX)size

# Прочие утилиты.
RM      = rm -f

# Побочные цели.
# Файл прошивки.
TARGET_HEX = $(TARGET).hex
# Файл карты бинарного файла.
TARGET_MAP = $(TARGET).map
# Бинарный файл прошивки.
TARGET_BIN = $(TARGET).bin


all: $(TARGET) size bin hex

strip: $(TARGET)
	$(STRIP) $(TARGET)

size: $(TARGET)
	$(SIZE) -B $(TARGET)

hex: $(TARGET)
	$(OBJCOPY) -O ihex $(TARGET) $(TARGET_HEX)

bin: $(TARGET)
	$(OBJCOPY) -O binary $(TARGET) $(TARGET_BIN)

asm: $(ASM)

$(TARGET): $(OBJECTS)
	$(LD) -o $@ $(LDFLAGS) $^

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<

%.o: %.s
	$(AS) -c -o $@ $(ASFLAGS) $<

%.s: %.c
	$(CC) -S -o $@ $(CFLAGS) $<

clean:
	$(RM) $(OBJECTS)
	$(RM) $(ASM)

clean_all: clean
	$(RM) $(TARGET)
	$(RM) $(TARGET_BIN)
	$(RM) $(TARGET_HEX)
	$(RM) $(TARGET_MAP)

burn: $(TARGET_HEX)
	$(AVRDUDE) -c $(PROGRAMMER) -p $(MCU_NAME) -e -U flash:w:$(TARGET_HEX)

