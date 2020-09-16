# Sound Source: Macro-oscillator
Sound Source is a voltage-controlled monophonic digital oscillator. Each algorithm is controlled by two continuously variable parameters, TIMBRE and COLOR, both of them voltage controllable.

This project is based on Mutable Intruments ["Braids"] (https://mutable-instruments.net/modules/braids). 
The code was adapted for [Blue Pill](https://stm32-base.org/boards/STM32F103C8T6-Blue-Pill.html) microcontroller.

Characteristics:

  - Format: Eurorack
  - Dimensions: 12HP, 80mm deep
  - 10Vpp audio signals
  - 5V control signals
  - 43 Waveforms
  - 96kHz, 8-bit audio processing
  - 12-bit CV capture.

Download [the manual](https://github.com/ernesto-g/modular-synth/raw/master/FINISHED/SOUND_SOURCE/Manual/Sound%20Source%20Manual.pdf)   
 
# Project structure

  * Firmware: Code for Blue Pill using ST Cube IDE
  * Manual: PDF and DOC file
  * SOUND_SOURCE_modular_PCB: Folder
    * SOUND_SOURCE_modular: Kicad project
    * DIY: PDF files for schematic, PCB and front panel
  * Front Panel Blueprint: Corel Draw files