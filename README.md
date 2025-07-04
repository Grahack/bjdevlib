# bjdevlib
[BJ Devices TB series MIDI controller](http://bjdevices.com)  open source library (bjdevlib) provide functionality for custom controller firmware creation.
BJ Devices TB series MIDI controllers based on ATmega64 microcontroller. Library was created in Atmel Studio. 
Currently there are 5 models of controllers - TB-12 (12 footbuttons), TB-5 (5 buttons), TB-8 (8 buttons), TB-6P (6 buttons and expression pedal) and TB-11P (11 buttons and expression pedal).
All modification builds on same hardware, only buttons connection is differ, so bjdevlib works for all devices.
You can find out wiring diagram:
for tb-12 http://bjdevices.com/manuals/tb12_diagram.pdf
for tb-5 http://bjdevices.com/manuals/tb5_diagram.pdf

Library used third-party code for LCD display. You can use provided code, or add your own LCD library to your project and use it.

How to start:

1. Download this library. You should save it in your project dir

2. Create new Atmel Studio project (GCC C Executable Project, ATmega64 device)

3. Go to project properties -> AVR/GNU C Compiler -> Symbols, add symbol TB_12_DEVICE or TB_5_DEVICE (depends of your model), add F_CPU=8000000UL symbol for all configurations

4. Go to AVR/GNU C Compiler -> Directories, add include path for "bjdevlib/tbseries/include" folder for all configuratoin

 4.1 If you want to use same LCD library, as in example, add "lcdlib" to include path for all configurations

5. In project explorer create folder "bjdevlib", add existing items - all files from "bjdevlib/tbseries/src" folder

 5.1 If you want to use same LCD library, also add "lcd_tb.c" from "lcdlib" folder

6. Copy code from examples to your main file, compile it to make sure that everything is configured correctly. You can also upload compiled hex file to hardware, check BJ devices TB-series manual to update firmware (chapter "Firmware Update") http://bjdevices.com/manuals/manualtb12_en.pdf 

7. Learn example to understand how to work with library

8. Write your own best of the world firmware and update your TB-12 or TB-5 device!!

9. Find bugs in library and report us. Improve library and send pull request! It is strongly welcome!

How to debug:

You able to send debug message to UART1. UART1 connects to COM-USB converter and you will able to see your messages on any PC terminal. LOG macro accept at least two parameters - log severity and formatted string, the same as printf() function.

For example you can write following:

LOG(SEV_INFO, "Value : %d", 2+3);

and you will see following message in terminal:

"INFO: Value : 5"

Logger have 4 severity levels: SEV_TRACE = 0, SEV_INFO = 1, SEV_WARNING = 2, SEV_ERROR = 3
If you turn on logger, you should specify severity level. All log messages with severity lower then specified will ingonred

1. Define LOG_ENABLED in predefined symbols. If LOG_ENABLED is defined, UART1 will initialize to 19200 baudrate

2. Define log severity in predefined symbols, for example to set SEV_INFO level define LOG_SEVERITY=1
 
3. Add LOG messages in your code

4. Connect USB port of midi controller to your PC. Drivers will install automatically. You can see "USB serial port" in the device manager -> COM ports. Open serial terminal and connect to nesessary com port

5. Run your application and see messages in terminal

6. After your projects is finished, you can remove LOG_ENABLED symbol, it reduce code size.

Creating commercial firmware:

BJ Devices TB series Gen2 midi controllers contain DS2411 Silicon Serial Number, and user can see unique ID in the menu of default firmware. Firmware developer able to lock firmware on particular device, using this ID. Firmware just need to compare actual device ID with user provided ID and run only if IDs matched. Here is example of roadmap:

 1. User placed order on website, made payment and send ID to you.
 2. You can automatically or manual compile sources with ID as predefined symbol, your firmware should compare ID on startup
 3. Compiled binaries immediately sends to user. User install firmware without housing demolition via free bootloader. No need to burn physical IC and send it to user. All processes, including purchase, compilation and delivery to user, can be highly automated on web server

You can see ID reading and comparsion example in library "examples" folder

