# Extremely Slow Movie Player - EPD47
An application which loads every X minutes an image from an PHP script.

The php script needs png images in the correct size 960x540 and converts them into a 4 byte per pixel raw bytestream.

After every request it increases the framenumber. 

So a 90 minutes movie (24fps) is played in 450 days if the frame changes every 5 minutes.

This script can also easily modified to realise a picture display for the EPD47.

# Configuration
Configure your paths and values in PHP/get.php

And also your URL and parameters in env.h for the Arduino application

You need a webserver to serve the php file.


# Movie conversion
I used ffmpeg for movie conversion:

For example the Big Buck Bunny Movie:
ffmpeg -i big_buck_bunny_1080p_h264.mov -i palette.png -filter_complex "[0:v]scale=960:540,hue=s=0,curves=all='0/0.0 0.25/0.27 0.5/0.5 0.75/0.67 1/1'[v];[v][1]paletteuse" 'images/bbb_%06d.png'

The video is scaled, hue to zero means grayscale, and then I changed the curve a little bit, because in the darks I fiind the display to dark. 
At the end I used a palette.png with a linear 16 color grayscale palette. 

If you have a widescreen movie, you can additionally crop the frames:
ffmpeg -i "MyRippedMovie1920x816.mp4" -i palette.png -filter_complex "[0:v]crop=1451:816,scale=960:540,hue=s=0,curves=all='0/0.0 0.25/0.27 0.5/0.5 0.75/0.67 1/1'[v];[v][1]paletteuse" 'img/movie_%06d.png'

# Credits
Initial code is from https://github.com/travisvesbach/picture-display-epd47

Inspiration comes from https://hackaday.com/2018/12/30/the-very-slow-movie-player-does-it-with-e-ink/

An application for the [LilyGo EPD47 ESP32 E-Ink board](https://github.com/Xinyuan-LilyGO/LilyGo-EPD47)
