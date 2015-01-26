Skype Utility Project
=====================

Welcome to the Skype Utility Project!

## What is it?

The Skype Utility Project is a plugin for *Skype for Windows Desktop*, adding some useful functionality the original developers were too lazy to implement. (Please note that this plugin works **only** with the Windows Desktop version of Skype, other platforms are not supported.)

## What can it do?

The most notable features are:
- Enable/disable text formatting: The latest Skype version started automatically formatting sent messages based on some special characters (e.g. \*bold\* or ~strike~) which annoyed a lot of people (including us). You can now turn that off permanently with just one click!
- Change notification window positioning: Notification window positioning now works smoothly with multiple connected displays. You can, of course, also choose on which display and where on that display your notification windows are supposed to appear.
- Hide Ads: Hide the advertisement banners above conversations. This will not affect Ads on the Skype "Home" page.

For a full list of features, check out the [feature description](#what-do-those-options-do) below. Interested in a feature that's not included yet? Send us a message! If it's possible, we'll integrate it.

## How do I get started?

### Where can I get SUP?

You can download the latest compiled version of this plugin from the rather inconspicuous "Releases" section right [here](https://github.com/dlehn/Skype-Utility-Project/releases).  
For those of you using [Chocolatey](https://chocolatey.org/), you can also install SUP from the command line by typing:  
`choco install skype-utility-project`

Alternatively, if you don't trust us, download the source code, open the included Visual Studio project and hit compile!

### I have the file, what now?

The Skype Utility Project consists of a single file called `d3d9.dll` which you need to place in your Skype installation directory, right next to the `Skype.exe` file (usually that will be in `C:\Program Files (x86)\Skype\Phone\`).

The next time you start Skype, you will notice a new menu item called "Advanced [SUP]" has appeared in the "Tools" menu of Skype's contact list window.

![Screenshot](/res/menu.png?raw=true "Screenshot")

From this menu, you can conveniently access all the configuration options and change them according to your preferences.
And that's everything you need to know!

## What do those options do?

Below is a detailed list of descriptions of the individual menu items, grouped according to their locations in the `SUP` menu.
- **Disable Chat Formatting**: When selected, special characters will no longer be used to format text (e.g. \*bold\* or ~strike~) but will appear as normal characters.
- **Layout**: Provides options to customize Skype's window layout.
  - **Hide Ads**: Hides/shows the banner ads above chat conversations. Does not apply to ads displayed on the "Home" page.
  - **Hide Home Toolbar**: Hides/shows the panel containing the "Home" and "Call phones" buttons.
  - **Hide Identity Panel**: Hides/shows the panel containing your user name and current status. Hiding this is probably not very convenient, but may still help e.g. on a laptop display where space is scarce.
- **Show Notifications**: Options to customize the location of notification popups.
  - **On Display**: If you have multiple monitors, you can choose on which of the monitors you wish the notification popups to appear.
  - **At Location**: Select in which corner of the selected monitor you want the notifications to appear. Default is bottom right, near your system tray. If you want the notifications to appear more towards the center of the screen, you can customize the exact positions by manually setting the values mentioned in the [Advanced Options](#Advanced-Options) chapter below.
- **Language**: Change `SUP`'s menu language. So far English, German and Russian are supported. (Feel free to contact us if you can provide translations to other languages or simply create a pull request.)
- **Help**: Provides information about `SUP` as well as quick links to this page and the release download page for new versions.

## Advanced Options

The Skype Utility Project stores its settings in the `%appdata%\Skype\` folder, in a text file named `sup.ini`. You can open this file in a text editor and manually change one or two settings not (yet) accessible via the application menu.

These values are:
- Section `[config]`
  - `notifOffsetX`: Horizontal offset of notification windows from the edge of the display in pixels.
  - `notifOffsetY`: Vertical offset of notification windows from the edge of the display in pixels.

Note that in order to apply changes to these values, you will have to restart Skype.

## Who did it?

This project is a combined effort of [David Lehn](http://blog.mountain-view.de "Portfolio"), [Moritz Kretz](http://kretzmoritz.wordpress.com/ "Portfolio") and [NDS](https://github.com/nestdimon).  
Thanks to [Chris McGrath](https://github.com/cpmcgrath) for the Chocolatey installer script!

We hope this will be of use to somebody. If not, well... at least you didn't pay for it, I guess.
