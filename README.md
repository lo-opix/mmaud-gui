# Minecraft Mods Auto-Updater With Drive GUI (MMAUD-GUI)
This is a GUI program that downloads and updates the 
Minecraft mods from a remote Google Drive folder.<br>
It compares the local mod files with the remote ones and downloads any missing or 
outdated ones.

## GUI version of [mmaud-cli](https://github.com/lo-opix/mmaud/)
<hr>

### Aussi disponible en [Francais](README_FR.md)

<hr>


## Requirements
- Windows 10 64 bit or higher
- Java 8 or higher
- Minecraft launcher installed by MS Store

## Usage
### As a Player

1. Download the latest release from the [releases page](https://github.com/lo-opix/mmaud/releases/)
2. Unzip the file
3. Execute the .exe file
4. Windows will show you a warning, click "More info" and then "Run anyway".
>If you don't want to do this, you can compile the program yourself
5. Set the correct key provided by you server owner by clicking on the settings icon
4. The program will download all the mods from google drive and put them in the `%Appdata%/.minecraft/mods` folder
<hr>

### As an Minecraft server Owner

 * You need to provide the id of your google drive folder which contains the mods.
You can get the id from the url of the folder: 
 * In this url `https://drive.google.com/drive/folders/1Z2X3C4V5B6N7M8K9J0H1G2F3D4S5A6Q7?usp=sharing` the id is `1Z2X3C4V5B6N7M8K9J0H1G2F3D4S5A6Q7`
 * Rename the folder to the name of your server
>Please note that your folder have to be set on enveryone that have the link can view the folder

## Contributing

If you want to contribute, you can do so by opening a pull request or an issue.

## Used third-party tools
- [Json](https://github.com/nlohmann/json) for parsing Google Drive HTTP responses 

## Known issues
- Closing the settings window won't stop the loading screen
