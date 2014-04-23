Oculus FPV
===========

###English info

####[Abstract (pdf)](https://github.com/Matsemann/oculus-fpv/raw/master/abstract.pdf), two pages in English about the project.

####[Video](https://www.youtube.com/watch?v=ANSjwWomIJ8), the product in action

This is a project where we use an Oculus Rift to display stereo video from a quadcopter, and rotate the cameras on the quadcopter based on head rotation.


#####Products used:

**Type** | **Component** | **Nr**
:---------|:---------------|:---------
Quadcopter | DJI Phantom 2 | 1
Camera | CMOS Camera Module - 728x488 SEN-11745 | 2
Videolink | 5.8GHz FPV AV 600mW RC832 | 2
Videoconverter | Hauppauge USB-LIVE 2 | 2
VR-HMD | Oculus Rift Developer Kit | 1
Servo | Servo Sub-Micro ROB-09065 | 2
USB to Serial | FT232RL Breakout BOB-00718 | 1
Serial link | Long Range 433MHz UM96 WRL-00155 | 2
PC | Arbitrary, must run Windows because of converters | 1


Info
----

####[Rapporten (pdf)](https://github.com/Matsemann/oculus-fpv/raw/master/prosjektrapport.pdf), detaljert beskrivelse av prosjektet.

I mikro mappen ligger koden som er skrevet for mikrokontrolleren på kretskortet. Tar seg av å styre servoene basert på input den får fra en mottaker.

I pc mappen ligger koden for programmet som kjører på pcen. Leser inn en videostrøm fra to usb enheter og viser dem på Oculus Rift, og leser hodebevegelser fra oculus rift og sender opp til flyet. I mappen ligger også graf-filene som setter opp direct show grafen, oculus bibliotekene som er brukt samt bibliotekene for breakout-kortet.

Bygging
-------
For å bygge programmet som kjører på PCen kreves det at man har Windows SDK installert. Prosjektet bør være konfigurert slik som OculusWorldDemoapp som følger med når man laster ned Oculus SDK. Altså at CommonSrc er inkludert og at libraries er lenket til i kompileren. Så må man legge til include filer og libraries for directshow, og for breakout-kortet.

Kildene til dette finnes i rapporten.


license
-------
The code is released with the Apache 2 license where possible, and the needed license where the code is a derivative work.
