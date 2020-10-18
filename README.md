# Modular synthesizer

Characteristics:

  - Eurorack form factor modules
  - Banana jack connectors
  - 10Vpp audio signals
  - 5V control signals
  - Non standard power connector

# Finished modules

  - [Sequencer](https://github.com/ernesto-g/modular-synth/tree/master/FINISHED/SEQUENCER): 8-step sequencer.
  - [Vintage VCA](https://github.com/ernesto-g/modular-synth/tree/master/FINISHED/VCA): 2-transistor discrete VCA.
  - [Simple VCO](https://github.com/ernesto-g/modular-synth/tree/master/FINISHED/VCO): Triangle and square VCO. 1V/Ocv control voltage.
  - [Slew limiter](https://github.com/ernesto-g/modular-synth/tree/master/FINISHED/SLEW_LIMITER): Slew limiter and LFO
  - [IOs](https://github.com/ernesto-g/modular-synth/tree/master/FINISHED/IOS): Audio line and Eurorack level adapter.
  - [Power Supply](https://github.com/ernesto-g/modular-synth/tree/master/FINISHED/POWER_SUPPLY): Power Supply Board for modules.
  - [Sound Source](https://github.com/ernesto-g/modular-synth/tree/master/FINISHED/SOUND_SOURCE): Macro-oscillator based on Mutable Instruments Braids.
  - [LFOx3](https://github.com/ernesto-g/modular-synth/tree/master/FINISHED/LFOx3): Low frequency oscillator and sequencer.
  - [ATV & Mixer](https://github.com/ernesto-g/modular-synth/tree/master/FINISHED/ATVMixer): Atenuverter X3 and 3-channel mixer.
  

# Unfinished work

  - Industrial kick
  - Case blueprints
  
# Project structure

Each module has the following structure:

  * Firmware: Microcontroller code (Arduino, Bluepill,etc.)
  * Manual: PDF and DOC file
  * XXXXXXX_modular_PCB: Folder
    * XXXXXXX_modular: Kicad project
    * DIY: PDF files for schematic, PCB and front panel
  * Front Panel Blueprint: Corel Draw files