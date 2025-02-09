# HalfCat Custom GSE and DAQ


See attached overview PDF for details
  
  
WHY BUILD A HALFCAT CUSTOM GSE?  
  
The commercial GSE controller design that is detailed in the HalfCat HCR-5100 Mojave Sphinx guidebook works well. It is battle tested with a combined 50+ static fires and launches. It is infinitely easier and cheaper to setup than building your own GSE Controller or DAQ system. HalfCat has always been optimized for low cost, ease and accessibility, and is a great entry point to learn liquid rocketry. 
  
We built our custom HalfCat GSE and DAQ only because we had built ten previous versions for other liquid rockets. It only required minor edits to our existing PCBs, software, and iPad remote. Also, our overall “RocketTalk” project includes a full featured flight computer talking to our hand-held base, as well as the GSE, so we have one integrated hand-held remote talking across three radios to the pad and the rocket. This allows us to fill, launch, and track the rocket all in one, although the GSE Controller and code is separate and can be isolated, especially when doing static fires or testing not involving flight.
  
That said, if you have an amateur EE and some code junkies on your team and you want your own custom GSE controller or DAQ then our open sourced solution can provide you with some good ideas and a starting point. We intentionally built everything with the amateur maker in mind – including Arduino source code, EasyEDA schematics and PCB design, and Apple iOS Swift code. Building your own GSE controller could also be a fun upgrade for a team that has already built and launched their first Mojave Sphinx. 
  
As an added disclaimer… Our HC GSE Controller was developed/modified over the course of 8-10 weeks to support our Mojave Sphinx build, but it is still very much a WIP. We’ve decided to take a snapshot and share our learnings, so student groups can learn, borrow, or reuse some of our ideas and our sloppy code. 
  
FULLY INTEGRATED WITH HALFCAT MOJAVE SPHINX GSE PLUMBING  
  
It should be noted that we used the exact same servos, valves, plumbing, and fittings as specified in HCR 5100 for the Mojave Sphinx GSE. The only addition in our setup is the custom GSE Controller with radio remote and a permanently mounted pressure transducer on the rocket tank (connected back through the umbilical). 
  
GSE CONTROLLER FEATURES  
  
• Control of two pad servo valves for N2O fill and CO2 purging.  
• Control of rocket N2O and fuel main valves  
• Monitoring/recording of rocket tank PT  
• Monitoring/recording of chamber PT (static fire)  
• Igniter firing and local continuity check  
• Full featured three channel DAQ recorder for static fire analysis (tank PT, chamber PT, load cell)  
• Hazard power safety key  
• Automated process tasks for loading and launching to ensure exact timing  
  
iPAD TOUCH REMOTE FEATURES  
  
• Two tab-based control screens including:  
• Main GSE Screen:  
&nbsp;&nbsp;• Manual valve and igniter control  
&nbsp;&nbsp;• Automated processes (launch and load)  
&nbsp;&nbsp;• DAQ control  
&nbsp;&nbsp;• Arming & Disarming  
• GSE Configuration Screen:  
&nbsp;&nbsp;• Launch/Static fire mode  
&nbsp;&nbsp;• Set servo stops  
&nbsp;&nbsp;• Zero PTs  
&nbsp;&nbsp;• Set PT ranges  
   
TECHNOLOGY SUMMARY  
-------------------------------  
  
GSE Controller
  
• Teensy 4.1 Arm7 Microcontroller  
• SD card for DAQ and command logging  
• Arduino IDE / Single C++ Sketch  
• NiceRF LoRa 611Pro 433Mhz serial TTY radio (or use 900 Mhz)  
• Uses 12.6v Lipo battery  
• ADS1115 four channel 16bit ADC  
• DB9 connectors for rocket umbilical  
• DB9 connectors for pad valves  
• 6 pin avionics connector for chamber PT and load cell amplifier (separate)  
• CPU heatsink strapping for thermal management  
• Custom PCB developed using EasyEDA and printed through JLCPCB  
  
iPad Touch-based Remote  
  
• Standard iPad mini (wifi only)  
• Uses RedSerial cable to interface with NiceRF LoRa radio  
• -or- Custom Bluetooth bridge using an ESP32 connected to a NiceRF LoRa 611Pro 433Mhz serial radio  
• Apple xCode IDE with iOS Swift code  
• 7.2v lipo battery and iPad battery  


DISCLAIMER

The open-source code and hardware designs ("Materials") provided here are related to the development and launching of bipropellant liquid rockets. The authors and contributors of these Materials provide them for educational and informational purposes only. The authors and contributors disclaim all responsibility for any injuries, damages, or losses of any nature whatsoever, whether direct, indirect, punitive, incidental, special, consequential, or any other damages, to persons or property, arising out of or connected with the use, misuse, or inability to use the Materials provided here.


Mike & Preston
Sinnoh Sphinx+ SN05
@RocketTalk33 in the Discord
