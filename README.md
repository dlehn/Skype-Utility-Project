Skype Utility Project
=====================

Skype for Windows Desktop plugin, adding some useful functionality the developers were too lazy to implement.

Welcome to the Skype Utility Project!

Features as of now are:
- Enable/Disable text formatting: The latest Skype version started automatically formatting sent messages based on some special characters (e.g. \*bold\* or ~strike~) which annoyed a lot of people (including us). You can now turn that of with just one click!
- Change notification window positioning: Notification window positioning now works smoothly with multiple connected displays. You can, of course, also choose on which display and where on that display your notification windows are supposed to appear.

Interested in a feature that's not included yet? Send us a message! If it's possible, we'll integrate it.

How do I get started?
======================

The Skype Utility Project consists of a single `.dll` file which you need to place in your Skype installation directory, right next to the `Skype.exe` file.

The next time you start Skype, you will notice a new menu item called "Util" has appeared in the menu bar of Skype's contact list window.

![Screenshot](/res/menu.png?raw=true "Screenshot")

From this menu, you can conveniently access all the configuration options and change them according to your preferences.
And that's everything you need to know!

Advanced Options
================

The Skype Utility Project stores its settings in the `%appdata%/Skype/` folder, in a text file named `sup.ini`. You can open this file in a text editor and manually change one or two settings not (yet) accessible via the application menu.

These values are:
- Section `[config]`
  - `notifOffsetX`: Horizontal offset of notification windows from the edge of the display in pixels.
  - `notifOffsetY`: Vertical offset of notification windows from the edge of the display in pixels.

Who did it?
===========

This project is a combined effort of ![Moritz Kretz](http://kretzmoritz.wordpress.com/ "Portfolio") and myself (![David Lehn](http://blog.mountain-view.de "Portfolio")).

We hope this will be of use to somebody. If not, well... at least you didn't pay for it, I guess.
