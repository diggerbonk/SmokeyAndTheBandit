Build Notes


    * To update the the background tiles:
        * cd ./data
        * gconvert background.xml
    * To update the sprites 
        * cd ./data
        * gconvert sprites.xml


CONVERTING MIDI FILES FOR UZEBOX

    1) Find a MIDI song
    2) Open it with rosegarden
    3) Reduce the tracks to 4 or less (preferably 2)
    4) The uzebox doesn't support polyphony, so try and eliminate 
       tracks that have it.
    5) Remove all markers from the file
    6) Create markers S (start) and E (end) at the start and end of the song
    7) Save the resulting file (midi export)
    8) Convert the file from midi type 1 to type 0 (wine ./tools/gn1to0.exe, "
    9) Convert the midi file to an uzebox patch file (java -cp 
       /home/trent/Downloads/uzetools.jar 
       com.belogic.uzebox.tools.converters.midi.MidiConvert 
       -f 6.5 input.mid output.h)

BACKSTORY

BACKGROUND

The goal of this project is to create a Smokey and the Bandit arcade game using all original game code and artwork. All technologies, artwork, literature used to make the game should be period appropriate, so that the finished product looks like it was built in the late 70s to early 80s.

Platform

Several platform options were investigated for creating the game, including: pulling hardware out of home game consoles, using modern tools like flash, and using original arcade hardware. Ultimately it was decided that using actual hardware from the era would be too time consuming mostly because of the necessity to use assembly. At the other end of the spectrum, using something like Flash, would not be true to the goals of the project and would also require using a full, modern computing platoform of some sort under the hood. Luckily we came across the Uzebox project. Uzebox is a modern retro game platform built to 80s era hardware specs. It has an 8 bit processor and similar sound/graphics capabilitites to the Colecovision, but games can be built with modern programming languages. Well, c anyhow. The JAMMA version of Uzebox uses the JAMMA stanard interface, giving it plug and play compatability with many arcade cabinets and hardware. 

Game

Smokey and the Bandit was released in 1977, a couple of years before the so-called golden age of arcade games, and a full year before the release of the seminal arcade title Space Invaders. This was a time before games had color graphics and before they had capable computer opponents. It would be difficult to make an engaging game using state of the art technology from 1977. So, it is imagined that the release of this game has been timed to coincide with the release of Smokey and the Bandit II. 1980. That provides for the use of color, more sophisticated enemy AI, better graphics, simple music and grainy audio samples. 

The game is a top-down vertically scrolling racer similar in appearance to games like Monaco GP, Bump n' Jump, or Spy Hunter. The goal of the game is to collect all the Coors that Snowman has dropped on the highway on the way from Texarkana to Atlanta. Like all games from the era, most of the details and narrative are left to the imagination. On the way, the Bandit must avoid road hazards, all while keeping up the pace enough to stay ahead of Buford T. Justice.

Release Collateral

* Arcade cabinet and internals. The cabinet is a Midway Galaga/Ms.Pac replica.
* Game code.
* Artwork: Marquee, monitor bezel, side art, and control panel overlay.
* Promotional material: 8 1/2" x 11" game flyer.
