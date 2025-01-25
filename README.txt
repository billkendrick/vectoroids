README.txt for "Vectoroids"

(Based on "Agendaroids", in turn loosely based on "Asteroids", Atari Inc. 1979)

by Bill Kendrick
bill@newbreedsoftware.com
http://www.newbreedsoftware.com/vectoroids/

November 30, 2001 - January 25, 2025

Version 1.1.2


About:
------
  "Vectoroids" is a vector-based rock-shooting game similar to the
  arcade classic "Asteroids."  It is an SDL game based on the source for
  "Agendaroids," an X-Window game written for the Agenda VR3 Linux-based PDA
  written by the same author.

  (This game is being entered into the "SDL Game Under 1 Megabyte" contest
  held by No Starch Press / Loki Entertainment Software in late 2001.)


Installation Procedure:
-----------------------
  Requirements:
  -------------
    "Vectoroids" uses the Simple DirectMedia Layer multimedia library,
    version 2 (aka "SDL2"), as well as two helper libraries:
    "SDL2_image" and, optionally (for music and sound) "SDL2_mixer".


    All three of these are available from the official SDL website:

        https://www.libsdl.org/


  Compilation:
  ------------
    To compile the game from its source, simply invoke the command "make":

        $ make


    If you wish to build the game with no sound support (ie, SDL_mixer is
    not available, or you don't have a sound card), you can build the
    'nosound' target:

        $ make nosound


    If you wish to build the game with no joystick support (ie, you're
    using a very old version of libSDL, which doesn't support joysticks),
    you can add the setting "JOY=NO" to the 'make' command.  For example:

        $ make JOY=NO


    If you wish to have Vectoroids and/or it's data files (sound, music
    and graphics) installed somewhere other than the default location
    of "/usr/local/bin/" and "/usr/local/share/vectoroids/", you can
    override the "PREFIX" and/or "DATA_PREFIX" values used by the Makefile.

    For example:

        $ make PREFIX=/home/username/

    ...will cause the "vectoroids" program to be copied into
    the directory '/home/username/bin/', the data files copied into
    the directory '/home/username/share/vectoroids/', and the man pages
    copied into '/home/username/man/man6/'.

    Or:

       $ make PREFIX=/usr DATA_PREFIX=/opt/games/vectoroids-data/

    ...will cause the program to be copied into '/usr/bin/',
    the data files into '/opt/games/vectoroids-data/', etc.

    (You can also edit the Makefile manually, if you wish.)


  Installation:
  -------------
    Once compiled, you must now install the program.  You do this
    by running 'make' with an "install" target.

    If you're copying the files to a directory you can't write into
    (eg, normal users should not be able to write into "/usr/local/bin/"),
    you will need to temporarily switch to the 'superuser' (aka "root").

        $ su
        Password: [enter the root password]
        # make install
        # exit


  Clean Up:
  ---------
    If, for some reason, you wish to keep the unarchived "vectoroids/"
    directory around, but want to delete the compiled object and program
    files, you can run:

        make clean


Running Vectoroids:
-------------------
  Once installed (assuming the directory in which the "vectoroids"
  program file was copied is listed somewhere in your shell's "$PATH"
  environment variable; it should be), simply call the program:

    $ vectoroids &       [the "&" is optional, and just puts the game's
                          process in the 'background,' so that your terminal
                          remains available for more commands]


  Available command-line options:

  Info Options:
  -------------
    --help              Displays a brief help message explaining the game
    -h                  and its controls, and then quits.

    --usage             Displays the available command-line options, and
    -u                  then quits.

    --version           Displays the version of the program which is being
    -v                  run, and then quits.

    --copying           Displays copyright information, and then quits.
    -c


  Settings:
  ---------
    --fullscreen        If possible, the game will run in fullscreen mode,
    -f                  rather than in a window.

    --nosound           Disables sound and music.
    -q


Title Screen:
-------------
  The title screen displays the title and credits.

  The high score is displayed at the top of the screen.
  If a game has been played since loading Vectoroids, the last score
  is displayed just below.  (If it is the same as the high score, it will
  be blinking.)

    * To begin a game, click the word "START" with the mouse.

    * If a paused game is available, the word "CONTINUE" will appear
      (and it will show "START OVER" for the other option).

    * Click either option (you may use the [Up] and [Down] keys to
      select) to either start a new game, or continue a paused game,
      respectively.

    * To quit, either close the game's window, or press the [Escape] key
      on your keyboard.  (Note: Any currently-paused game will be saved,
      so when you run Vectoroids again later, you can continue where you
      left of.)

The Game:
---------
  Game Controls:
  --------------
    The game can be played with either the keyboard, a joystick
    (the joystick must have at least two axes (directions) and
    two fire buttons), or using a mouse or touchscreen by
    clicking/tapping in the plus (+) shaped control zone at the
    lower part of the screen.

    * [Left] / [Right] on the keyboard
      [Left] / [Right] on the joystick
      Left / Right sides of the on-screen control zone

      Rotate ship counter-clockwise and clockwise, respectively.


    * [Up] on the keyboard
      [Fire-A] on the joystick
      Upper area of the on-screen control zone

      Thrusts the ship in the direction it is currently facing.


    * [Space] on the keyboard
      [Fire-B] on the joystick
      Lower area of the on-screen control zone

      Fires a bullet in the direction the ship is facing.


    * Either [Shift] on the keyboard
      Any other [Fire] on the joystick

      Re-spawns your ship after you die, even if the game thinks there
      are still too many asteroids near the center of the screen...


  Status Display:
  ---------------
    The following is displayed at the top of the screen during the game,
    from left to right:

    * Score

      Your current score.


    * Level

      The level of the game you are currently playing.


    * Lives

      Miniature spaceships which represent how many extra lives you have left.


    Also, sometimes text (for example, what level you are on when you enter
    a new level) appears on the center of the screen for a moment.


  Scoring:
  --------
    Each rock you shoot (or crash into) gains you points.  The smaller the
    rock, the more points you gain.

    Every 10,000 points, you also receive an extra ship.


  Levels:
  -------
    After all rocks have been destroyed, you move on to the next level,
    which begins with more rocks than the previous started with...


Credits:
--------
  Programming:
  ------------
    Original "Agendaroids":
      by Bill Kendrick
      bill@newbreedsoftware.com
      May 21, 2001 - October 9, 2001  (version of code-base used)
      http://www.newbreedsoftware.com/agendaroids/

    SDL Port:
      by Bill Kendrick
      November 30, 2001 - December 1, 2001
      http://www.newbreedsoftware.com/vectoroids/

  Graphics:
  ---------
    Jupiter photo: 
      Obtained by NASA's Voyager 1 probe - In the Public Domain
      Photo taken on February 25, 1979
      http://nssdc.gsfc.nasa.gov/photo_gallery/photogallery-jupiter.html
      Photo ID: P-2115
      (Manipulated using The Gimp; http://www.gimp.org/ )

    Game Icon:
      Bill Kendrick

  Sounds:
  -------
    Obtained from various free sound-effect archives on the web.
    (Some effects altered using Sox, by Chris Bagwell;
    http://home.sprynet.com/~cbagwell/sox.html )

  Music:
  ------
    "Decision"  (decision.s3m)
    by Mike Faltiss (Hadji / Digital Music Kings)
    deadchannel@hotmail.com
    April 1995


Contact Information:
--------------------
  Bill Kendrick
  Email: bill@newbreedsoftware.com

