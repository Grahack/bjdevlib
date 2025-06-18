CFLAGS_c_o="-g -Os -Wall -mcall-prologues -mmcu=atmega64 -I bjdevlib/tbseries/include -I bjdevlib/lcdlib -DF_CPU=8000000UL -DTB_5_DEVICE"

mkdir -p build

files=(adc bjdevlib_tb button crc8 expression led lldled midi onewire pedal_led portio timer uart unique_id)

for file in "${files[@]}"
do
 echo "Compiling bjdevlib/tbseries/src/$file.c to build/$file.o..."
 /usr/bin/avr-gcc -c $CFLAGS_c_o -o build/lcd_tb.o bjdevlib/lcdlib/lcd_tb.c
done

 echo "Compiling bjdevlib/tbseries/src/$file.c to build/$file.o..."
 /usr/bin/avr-gcc -c $CFLAGS_c_o -o build/$file.o bjdevlib/tbseries/src/$file.c

echo "Compiling Banana.c to build/Banana.o..."
/usr/bin/avr-gcc -c $CFLAGS_c_o -o build/Banana.o Banana.c

echo "Assembling all .o files to build/Banana.obj..."
/usr/bin/avr-gcc $CFLAGS_c_o -o build/Banana.obj build/*.o

echo "objcopy build/Banana.obj to Banana.hex"
/usr/bin/avr-objcopy  -R .eeprom -O ihex build/Banana.obj Banana.hex
