========================================================================
                   The CasLink Project: CasLink v3.2
                Copyright (c) 1999-2022 Alexey Podrezov
========================================================================

Contents
--------

 1. Overview
 2. Caslink3 Features
 3. Caslink3 Files
 4. Caslink3 Command Line Options
 5. CasLink/Caslink3 Cable
 6. Tips and Tricks
 7. Project History
 8. Copyrights
 9. Credits
10. Contact Information


1. Overview
-----------

The main goal of CasLink projects is to allow loading programs and games into MSX computers from different 
audio devices. All MSX computers are equipped with a cassette recorder interface that was designed to save
and load software from tapes. What CasLink/Caslink3 does is basically simulating this interface.

Caslink allows to transform MSX binary and ASCII files into WAV or MP3 files that can be played back to MSX
using a PC sound card, a CD or MP3 player or even a mobile phone through a simple audio cable. See the cable
schematics below.

CasLink was primarily created for MSX users who don't have floppy disk drives in their MSX computers. It can
be also a relief for those, whose disk drives are broken. Those MSX users will be able to play their favorite
games on their MSX computers again with the help of Caslink.

The primary location of CasLink project pages and other projects of Alexey Podrezov:

http://www.podrezov.com/


2. Caslink Features
--------------------

Caslink supports encoding of the following MSX formats:

  BAS - tokenized or detokenized basic programs (ASCII format)
  BIN - binary images of ROM cartridges with startup code
  ROM - pure images of ROM cartridges
  SCR - screenshots (BIN files with startup code to transfer contents to VRAM)

Caslink3 supports encoding of the following BIN and ROM formats:

  BIN - 8/16/24 kb
  ROM - 8/16/24/32/49 kb

NOTE: the 49kb ROMs are only supported if they don't require a mapper and if they start from address 0x0000.

Caslink also supports multi-file mode that allows to upload games up to 131072 bytes (1 MegaRom) long to MSX.
With 3000 baud speed it takes about 7 minutes. The multi-file mode supports encoding up to 20 MSX files into
a single WAV or MP3 file. The Caslink in multi-file mode allows to upload several binary modules with or
without the special basic loader before them (some games do have special basic loaders to properly setup the 
MSX and load binary files in a specific order). If only binary files are uploaded to MSX, each of them
prepares a system for uploading the next binary, so after the first binary is uploaded, all further uploads
are handled automatically.

In addition to auto-upload, CRC checking of uploaded modules was added to make sure that the modules are
transferred to MSX without errors.

Caslink creates output files in the following formats:

 - WAV (8 bit, mono, 11 kHz)
 - WAV (16 bit, stereo, 44 kHz)
 - MP3 (uses external LAME MP3 encoder)

The following MP3 bitrates are supported: 

 - 192 kbps
 - 256 kbps
 - 320 kbps

The following MSX CAS interface speeds are supported:

 - 1200 baud
 - 2400 baud
 - 3000 baud (may not work on certain MSX hardware)

The really new thing in Caslink3 project is the ability to create optimized MP3 files for different devices:
PC sound card, CD player and MP3 players from major various manufacturers: Creative, Apple, various mobile
phones and so on. During testing it was noticed that Creative and Apple MP3 players require different waveform
to be created, so Caslink asks to specify normal or inverted waveform in order to create the appropriate output
file. 

Caslink was tested on the following MSX hardware:

 - Various MSX1 machines
 - Various MSX2 machines
 - Various MSX2+ machines
 - OpenMSX emulator with different machine configurations

Caslink was tested with the following audio devices:

 - Sony CD player
 - Various Apple MP3 players
 - Various Creative MP3 players
 - Various mobile phones
 - Various PC sound cards


3. Caslink Files
-----------------

The Caslink project contains the following files:

 - Caslink3.exe   (the main executable file)
 - Caslink3.cpp   (source code, main C code)
 - Caslink3.h     (source code, header file)
 - l32k1.bin      (compiled 32k ROM loader, part 1)
 - l32k1.mac      (32k ROM loader source, part 1)
 - l32k2.bin      (compiled 32k ROM loader, part 2)
 - l32k2.mac      (32k ROM loader source, part 2)
 - l32k2rst.bin   (compiled 32k ROM loader with reset, part 2)
 - l32k2rst.mac   (32k ROM loader with reset source, part 2)
 - l49k1.bin      (compiled 49k ROM loader, part 1)
 - l49k1.mac      (49k ROM loader source, part 1)
 - l49k2.bin      (compiled 49k ROM loader, part 2)
 - l49k2.mac      (49k ROM loader source, part 2)
 - l49k3.bin      (compiled 49k ROM loader, part 3)
 - l49k3.mac      (49k ROM loader source, part 3)
 - l49k2rst.bin   (compiled 49k ROM loader with reset, part 3)
 - l49k2rst.mac   (49k ROM loader with reset source, part 3)
 - mulfload.bin   (compiled multi-file loader)
 - mulfload.mac   (multi-file loader source)
 - preload.bin    (compiled multi-file preloader)
 - preload.mac    (multi-file preloader source)
 - rloader.bin    (compiled ROM loader)
 - rloader.mac    (ROM loader source)
 - rloadrst.bin   (compiled ROM loader with reset)
 - rloadrst.mac   (ROM loader with reset source)
 - binload.bin    (compiled single file loader)
 - binload.mac    (single file loader source)
 - readme.txt     (this readme file)
 - disclaimer.txt (disclaimer file)
 - whats.new      (list of new features)
 - bin2inc.py     (utility to convert .bin files into .inc files, requires Python)

To be able to use the Python utility that converts .bin files (loaders) into hex byte arrays for the caslink.h
header file you need to install the Python programming environment. Go to www.python.org for the installer.
Once Python is installed, the utility can be run by typing (here preloader.bin is an example):

python bin2inc.py preload.bin preload.inc

This will convert the binary preloader's data into a hex byte array that you can insert into the source header
file so that the preloader's binary code gets included into the Caslink's executable file.

The following files are required for the MP3 conversion:

 - lame.exe
 - lame_enc.dll

These files belong to the LAME MP3 encoder. The encoder should be made available together with the Caslink
project files in it's original distribution form (zip), with all the license files and readmes. It is advised
to use only the codec that was tested with the Caslink project, namely version 3.96 MMX.

In order for Caslink main executable to work, it is required to install Visual Studio 2012 runtime libraries
that are supplied in the vcredist_x86.exe and vcredist_x64.exe files. This file is downloadable from
Microsoft's website, but it is advised to always put it together with Caslink files.


4. Caslink Command Line Options
--------------------------------

Caslink is a Windows console application, so it should be started from a command interpeter (CMD.EXE) or
(better) from a file manager that supports console output. The best one is Far Manager, it is highly recommended:

 http://www.farmanager.com/?l=en

If Caslink is run without command line options, it will show the command line option (key) to get help. To get
help on command line options run Caslink executable as follows:

 Caslink3 /?

The following command line options are supported:

 /e - encode one file
 /r - encode one file, start ROM after reset
 /m - encode multiple files (up to 20)

Here are some examples of command lines:

 Caslink3 /e File1.rom
 Caslink3 /r File1.rom
 Caslink3 /m File1.rom File2.rom
 Caslink3 /m File1.bas File2.rom File3.rom
 Caslink3 /m File1.bas File2.bin File3.bin File4.bin 

Please note that file names should be no longer than 12 bytes (8 bytes name, 3 bytes extension and a "."). Please
see the Tips and Tricks section to get some tips and tricks on how to encode files of different types. The files
to be encoded into a WAV or MP3 file should be in the same folder with the Caslink3.exe file.

After Caslink encodes the needed file into WAV or MP3 file, the only thing you need is to play this file to MSX
via a simple audio cable (see instructions below). In order to MSX to start receiving the file you need to type
LOAD, CLOAD or BLOAD command depending on what type of file is being uploaded. Here are some examples:

 cload"name"   - loads a tokenized Basic program named "name"
 load"cas:"    - loads a detokenized Basic program
 load"cas:",r  - loads a detokenized Basic program and runs it
 run"cas:"     - loads a detokenized Basic program and runs it
 bload"cas:"   - loads a binary program
 bload"cas:",r - loads a binary program and runs it

For more information please see the Tips and Tricks section of this readme file.


5. Caslink Cable
-------------------------

Here's a brief schematics for CasLink cable in ASCII mode. The Caslink project pages will contain a more detailed
one together with a few pictures of the actual cable.

The Caslink cable is a simple audio cable that is quite easy to make. Both connectors can be bought almost in
every electronics shop. To make a Caslink cable you will need a 5 or 8 pin male connector on MSX side (to plug
into MSX cassette interface) and a 3.5mm stereo jack on the other side (to connect to a PC sound card, mobile
phone, CD or MP3 player or cassette player/recorder output).

Here's the layout of pins for the required connectors:

      2
      *                 --------------
 5 *     * 4          /               |------.----.---
                ======                       |    |   )
  *   *   *           \               |------`----`---
                        --------------    \     \    \      
   *     *                                 \     \    \___ Audio 1
                                            \     \    (L)
                                             \     \    
Viewed from the front                         \     \_____ Audio 2
  side of connector                            \       (R)
                                                \     
     5 - CasIN                                   \________ Ground
     4 - CasOUT                                   ////////
     2 - Ground             

To make the cable (allows only uploading files to MSX) you need to solder pin 2 of the cassette connector to
the 'Ground' contact inside the audio jack and then to solder pin 5 of the cassette connector to the 'Audio 1'
contact of the audio jack. Keep all other pins/contacts untouched.


6. Tips and Tricks

The first most important tip is for setting up the sound output hardware. If you are using a PC sound card, CD
or MP3 player, mobile phone or other audio device, please make sure that the output sound is not altered in
any way. You need to put Bass and Treble controls on PC to their default position, disable Bass Boost and
Equilizer (or put all equilized controls to the middle - flat mode) on CD or MP3 player. If the sound is
"improved" by the hardware in any way, MSX most likely will not be able to get any data from your audio source!

The second most important tip is for setting the output volume for the sound device so that MSX could "recognize"
the transmission. Usually MSX is not very sensitive, so it should not take more that a few minutes to set up
the volume correctly.

First, it is recommended to set the output volume to about 80%, type CLOAD, LOAD or BLOAD command (depending on
what file is being played back) and start the playback of that file on your audio hardware. If MSX does not
react at all, increase or decrease the volume by 10% and try again. In most of cases it does not make sense
to lower the volume below 60% as MSX will not accept such a low signal.

If MSX starts to receive a file (showing FOUND or LOADING) message, upload the file fully and check that it is
intact (in case of a basic program). In case of a binary file, the Caslink will automatically check CRC of the
uploaded module and show CRC ERROR message if uploading failed. The CRC check is performed if a binary file is
run.

In case you get a "Device I/O Error" message during uploading, try to increase or decrease the volume by 3-5%
and re-try uploading. Once the volume is set correctly, memorize the settings - it will be easier to set it up
next time. If you receive this error on files with normal waveforms, try to create a file with inverted
waveform.

It is recommended to choose a file encoded with 1200 baud in order to initially set up the sound volume. In
case you are using MP3 file, choose the safe bitrate (256 kbps) and 1200 baud. A short basic program would be
just perfect for such operation. Here's an example of such program:

 10 cls
 20 print "Hello World!"
 30 for i = 1 to 1000
 40 next i
 50 end

Apple's hardware showed the best uploading results at all speeds and bitrates during Caslink project testing.
But Creative players had certain problems, for example Zen player could not upload anything at 3000 baud at all.
Muvo players could upload at 3000 baud, but only with 256kbps and 320kbps bitrates. The volume had to be set
very precisely, otherwise upload was failing due to CRC errors. Mobile phones were also quite tricky, for example
HTC phone had no trouble with any files, however Samsung phones required a much more precise volume setting.


More tips:

 - The LAME MP3 encoder files lame.exe and lame_enc.dll should be placed into the same folder with Caslink3.exe file

 - The files to be encoded into a WAV or MP3 file should be in the same folder with the Caslink3.exe file

 - It is advised to convert all tokenized Basic programs to detokenized (ASCII) format, so they can be easily
   modified on a PC, the is a utility to do that conversion called LIST.EXE, it is available on some of MSX sites

 - All 8, 16, 24, 32 and 49kb ROM and some BIN files should be uploaded without a Basic loader in front of them,
   here's an example of the command line:

    Caslink3 /e File1.rom
    Caslink3 /m File1.rom File2.rom

 - When uploading several BIN files, please note that some games require a special Basic loader in order for the
   game to work, so you will need to modify that loader yourself, to make it work with Caslink, here is an example:

    ----- Before -----
    30 A%=&Hc000
    40 POKE A%,0
    50 A%=A%+1
    60 POKE &Hf676,PEEK(VARPTR(A%))
    70 POKE &Hf677,PEEK(VARPTR(A%)+1)
    80 RUN"zanac-ex.GA0"

    ----- After -----
    30 A%=&Hc000
    40 POKE A%,0
    50 A%=A%+1
    60 POKE &Hf676,PEEK(VARPTR(A%))
    70 POKE &Hf677,PEEK(VARPTR(A%)+1)
    80 LOAD"cas:",R

   In this case the "zanac-ex.GA0" file is a Basic program that was run from a disk drive using the RUN command. Both
   files were converted to ASCII format and encoded by Caslink3 with the following command line:

    Caslink3 /m File1.bas File2.bas File3.bin File4.bin

   where first 2 Basic programs are loaders and the rest are game binaries. So in order for the first Basic program
   to load and start the second Basic program from a cassette interface, we had to change the RUN command into the
   LOAD"cas:",R command. 

   Another example:

    ----- Before -----
    1 DEFUSR=&Hd000
    2 BLOAD"WOODYPOC.VRP":GOSUB9
    3 BLOAD"WOODYPOC.GA2",-&H1000:A=USR(0)
    4 BLOAD"WOODYPOC.GA1",-&H1000:POKE&Hd002,&H85:A=USR(0)
    5 FORT=1TO7:OUT&Hfe,T
    6 IFT=2THENBLOAD"WOODYPOC.GA8",-&H1000:GOTO8
    7 BLOAD"WOODYPOC.GA"+HEX$(T),-&H1000
    8 NEXT:OUT&Hfe,1:OUT&Hfd,2:BLOAD"WOODYPOC.STR",R:GOSUB9
    9 POKE&H87A4,247:POKE&H87A5,12:RETURN

    ----- After -----
    1 DEFUSR=&Hd000
    2 BLOAD"CAS:":GOSUB9
    3 BLOAD"CAS:",-&H1000:A=USR(0)
    4 BLOAD"CAS:",-&H1000:POKE&Hd002,&H85:A=USR(0)
    5 FORT=1TO7:PRINTT:OUT&Hfe,T
    6 IFT=2THENBLOAD"CAS:",-&H1000:GOTO8
    7 BLOAD"CAS:",-&H1000
    8 NEXT:OUT&Hfe,1:OUT&Hfd,2:BLOAD"CAS:",R:GOSUB9
    9 POKE&H87A4,247:POKE&H87A5,12:RETURN

   As you see, in this case the program loads multiple binary files in a specific order using the BLOAD"name"
   command. To make the program work with Caslink, we need to change all file names to "cas:" and then encode
   all files in that specific order as the original Basic program loads them. Here's the command line for this
   particular case (the WOODYPOC.BAS is the above shown Basic loader):

    Caslink3 /m WOODYPOC.BAS WOODYPOC.VRP WOODYPOC.GA2 
    WOODYPOC.GA1 WOODYPOC.GA1 WOODYPOC.GA8 WOODYPOC.GA3 
    WOODYPOC.GA4 WOODYPOC.GA5 WOODYPOC.GA6 WOODYPOC.GA7 
    WOODYPOC.STR

 - Sometimes a Basic loader tries to free memory and load itself again, but to a different memory address. In
   this case it is needed to encode such basic loader twice before the binary files. Here's an example of such
   loader and how it should be converted to work with Caslink:

    ----- Before -----
    10 POKE-1,170:WIDTH 80
    20 IFPEEK(&HF677)=128THENCLEAR20:MAXFILES=0:POKE&HE100,0:
       POKE&HF677,&HE1:RUN"KINGV2.BAS"
    30 CLS:PRINT"Please wait - King Valley II is loading..."
    40 FORA=1TO6:BLOAD"KINGV2."+HEX$(A),R:NEXT

    ----- After -----
    10 POKE-1,170:WIDTH 80
    20 IFPEEK(&HF677)=128THENCLEAR20:MAXFILES=0:POKE&HE100,0:
       POKE&HF677,&HE1:LOAD"cas:",R
    30 CLS:PRINT"Please wait - King Valley II is loading..."
    40 FORA=1TO6:BLOAD"cas:",R:NEXT

   The following command line should be used in case Basic loader may reload itself:

    Caslink3 /m File1.bas File1.bas File2.bin File3.bin File4.bin
    File5.bin File6.bin File7.bin

 - If a game loader loads a screenshot, it is advised to remove that code and not encode the screenshot file to
   save loading time, however make sure that the initial screen for the game is set in the loader

 - The optimum encoding parameters are: 3000 baud for MSX cassette interface speed and 256kbps bitrate for an MP3
   file

 - Yamaha and other quality MSX hardware was loading games at 3000 baud, which is very good because uploading a
   128kb game takes 7 minutes compraing to 12 minutes with 2400 baud rate

It should be noted that not all multi-file megarom games can be loaded to MSX using Caslink. In some cases it
takes a lot of time to modify the game loaders so they correctly work with Caslink. In some rare cases certain
games can not be loaded at all..

On Caslink project pages there are a few already converted games and programs in WAV and MP3 format for testing
of uploading at 2400 and 3000 baud speeds. So please download and test them on your MSX hardware with different
audio players. I would be happy to receive your feedback!

Note that not all MSX hardware supports 3000 baud speed! And not all MP3 players or mobile phones can upload
data to MSX at 3000 baud!


7. Project History
------------------

Below is the history of changes made in the CasLink projects by its developer:

    Name            Date         Description
----------------------------------------------------------------------
 Podrezov   07/09/1999   Initial version created
 Podrezov   26/08/2005   Sources ported to Visual Studio .NET
 Podrezov   28/08/2005   Added support for 16 bit, stereo, 44 kHz
 Podrezov   29/08/2005   Modified ROM loaders
 Podrezov   30/08/2005   Added sound file playback
 Podrezov   05/09/2005   Increased parts number to 15, multifile loader modified
 Podrezov   12/09/2005   Fixed detokenized Basic file loading bug in multifile loader routine
 Podrezov   25/09/2005   Added preloader for BINs, ROMs
 Podrezov   26/09/2005   Modified and recompiled ROM loaders, created BIN2INC utility
 Podrezov   29/09/2005   Added preloader for multi-file ROMs
 Podrezov   03/10/2005   Wrote WAV converter from CD to RD quality
 Podrezov   14/03/2008   Sources ported to Visual Studio 2005
 Podrezov   17/03/2008   Added MP3 conversion capability, added colors for messages
 Podrezov   19/03/2008   Disabled signal level question (now it always is 100), fixed waveform of CD quality WAV file
 Podrezov   21/03/2008   Added option for normal and inverted waveforms for different MP3 players
 Podrezov   22/03/2008   Fixed a bug in WAV file length calculation when only binary files are encoded
 Podrezov   23/03/2008   Added copyright message to WAV files, added wrong CR+LF check for input ASCII files
 Podrezov   26/03/2008   Re-wrote all ROM loaders, created single BIN file loader, added CRC check
 Podrezov   28/03/2008   Added 3000 baud MSX cassette I/O speed support
 Podrezov   05/08/2008   Added missing detokenized file encoding code in single file encoding routine
 Podrezov   29/10/2008   Fixed a bug in 32kb ROM loading routine, now all such ROMs should start normally
 Podrezov   08/02/2016   Changed the second loader for 32kb files so it does not reset a computer. Added new loader that resets
                         the computer and changed /r option to be used with it, removed decoding and playback functions
 Podrezov   13/02/2016   Rewrote 8-16kb ROM loaders to work correctly with files that start from different addresses, introduced
                         the final 32kb loader with reset and fixed the bug in the 32kb loader without reset
 Podrezov   14/02/2016   Added 49kb ROM loaders and handling routines, fixed MP3 file waiting routine
 Podrezov   18/02/2016   Fixed bug in 3rd 49kb loader routine (with reset)
 Podrezov   23/02/2016   Refined file recognition routine to exclude files that can't be loaded, added rudimentary detection of
                         ROM files with mappers
 Podrezov   07/03/2016   Fixed the 49kb final loader, tested on numerous MSXs (1,2,2+), corrected patching routine for multi file
                         loader (after its code optimization)
 Podrezov   05/05/2016   Fixed the 49kb final non-reset loader, now BIOS is enabled before passing control to the ROM
 Podrezov   06/05/2016   Added a patcher for 3 erased bytes at 8000H into the loaders with reset functionality
 Podrezov   07/05/2016   Increased the max amount of loaded files to 20
 Podrezov   08/05/2016   The data in loaded BIN files is also moved to the destination address from the end to avoid corruption
 Podrezov   09/05/2016   Wrong input does not cause the program to exit with error any more, only correct input is accepted; BIN
                         loaders now start from 8200h to load 24kb BIN files
 Podrezov   21/05/2016   Fixed missing C flag clearing before ADC and SBC operations - the set flag could affect the operations
 Podrezov   04/03/2018   Fixed the 2400 baud file generation so that it is also accepted by emulators; adjusted the length of headers,
                         verified the highest baud rate to be around 3000 baud
 Podrezov   14/08/2022   Fixed the patcher routine in 32kb and 49kb loaders with reset (patcher call address left on the stack)


8. Copyrights
-------------

The CasLink project, was developed by Alexey Podrezov. The project is distributed with the source code, so everyone
can use it to develop his own MSX related tools.

The project files can be put on any public ftp and web servers, however they should be always accompanied by this
readme.txt file.

If any part of CasLink code is used in any third-party utility, please mention the original author's name in the
credits. You do not need to ask the author's permission to use Caslink code, however it would be nice to send a
message to the author to let him know that his code will be used to keep MSX scene alive.

The only restriction is: CasLink code should not be used for commercial purposes. However if someone finds the way
to make money out of it, I would be interested to hear it. Just out of curiosity...


9. Credits
----------

I would like to thank the following people who helped or inspired me to create and continue the development of
CasLink project:

 Pavel Zakharov (RBSC)
 Dmitry Pugachev (RBSC)
 Alexey Krasivsky (RBSC)
 Manuel Bilderbeek
 Wouter Vermaelen
 Danilo Drago
 Egor Voznessenskiy (R.I.P.)
 Gennadiy Kurapov
 Sergey Kirov
 Ekaterina Kirova
 Sean Young (not the actor Sean Young)
 Arnaud [The File-Hunter]
 IgorX (whom I know only from ICQ)
 Mihail Volkov
 Timo Soilamaa (Nyyrikki)
 Ahti Soilamaa
 Saku Taipale (STT)
 Walter van Niftrik


10. Contact Information and Donations

If you wish to contact the author of Caslink projects - Alexey Podrezov, please feel free to send a message to
the following e-mail address:

 alexey.podrezov@gmail.com

If you would like to make a small donation as appreciation of the author's effort, please use the above e-mail
address to send a few bucks via PayPal.

========================================================================
