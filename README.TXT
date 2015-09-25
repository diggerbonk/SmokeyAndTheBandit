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

Sorry for waiting so long to provide some information on this, I've been preoccupied with a list of other things. 

Background

The goal of this project is to create a Smokey and the Bandit arcade game using all original game code and artwork. All technologies, artwork, literature used to make the game should be period appropriate, so that the finished product looks like it was built in the late 70s to early 80s.

Platform

I looked into several options for coding the game; including pulling hardware out of home game consoles, using modern tools like flash, and original arcade hardware. Ultimately I decided that using actual hardware from the era would be too time consuming mostly because of the necessity to use assembly. The other end of the spectrum, something like Flash, would not be true to the goals of the project and would also require using a full, modern computing platoform of some sort under the hood. Luckily I found the Uzebox project. Uzebox is a modern game platform built to 80s era specs. It has an 8 bit processor and similar sound/graphics capabilitites to the NES, but games can be built with modern programming languages. Well, c anyhow. The JAMMA version of Uzebox uses the JAMMA stanard interface, giving it plug and play compatability with many arcade cabinets of the era. 

Game

Smokey and the Bandit was released in 1977, a couple of years before the so-called golden age of arcade games, and a full year before the release of the seminal title Space Invaders. This was a time before games had color graphics and before they had capable computer opponents. It would be difficult to make an engaging game using state of the art technology from '77. So, it is imagined that the release of this game has been timed to coincide with the release of Smokey and the Bandit II. 1980. That provides for the use of color, more sophisticated enemy AI, better graphics, simple music and grainy audio samples. 

The game will be a top-down vertically scrolling racer similar in appearance to the original Monaco GP or Spy Hunter. The goal of the game is to escort Snowman as far as you can to deliver Coors from Texarkana to Atlanta. Like all games from the era, most of the details and narrative are left to the imagination. On the way, the Bandit must avoid cars and other road hazards, all while keeping up the pace enough to stay ahead of Buford T. Justice.

Game Elements

* The game opens with the audio clip "This is sheriff Burford T. Justice. I'm in hot pursuit of a black Trans Am, he's all mine; so stay outta the way!!," leading into the background music (east bound and down in midi)
* Cledus progress is tracked on the right, his truck is shown on a line showing relative progress to Atlanta.
* When the Bandit falls behend pace, Sheriff Justice appears behind and an audio warning is played "My handle's Smokey Bear, and I'm tail-grabbin your ass right now!". All the bandit has to do is pick up the pace a bit and the Sherrif will have an untimely accident.
* The bandit must avoid running into cars on the roadway. Hitting a car causes a crash and lost time.
* Going off-road slows down the car
* Occasionally road blocks are set up. These are forwarned with the audio "Set up a roadblock". 
* The bandit must jump over the roadblock. If he succeeds, the player gets an audio reward "You sum'bitches couldn't close an umbrella"
* The game ends when 

Graphics



Release Collateral

* Arcade cabinet and internals. This will be re-used generic cabinet, monitor, control panel, and power supply from 1981.
* Game code.
* Artwork: Marquee, monitor bezel, side art, and control panel overlay.
* Promotional material: 8 1/2" x 11" game flyer.
