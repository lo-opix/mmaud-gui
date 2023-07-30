# Mise à jour automatique des mods Minecraft avec l'interface graphique du lecteur (MMAUD-GUI)
Il s'agit d'un programme GUI qui télécharge et met à jour le
Mods Minecraft à partir d'un dossier Google Drive distant.<br>
Il compare les fichiers de mod locaux avec les fichiers distants et télécharge tous les fichiers manquants ou
les obsolètes.

## Version graphique de [mmaud-cli](https://github.com/lo-opix/mmaud/)
<h>

### Also aviable in [English](README.md)

<h>


## Exigences
- Windows 10 64 bits ou supérieur
- Java 8 ou supérieur
- Lanceur Minecraft installé par MS Store

## Utilisation
### En tant que joueur

1. Téléchargez la dernière version depuis la [page des versions](https://github.com/lo-opix/mmaud-gui/releases/)
2. Décompressez le fichier
3. Exécutez le fichier .exe
4. Windows vous montrera un avertissement, cliquez sur "Plus d'infos" puis sur "Exécuter quand même".
> Si vous ne voulez pas faire cela, vous pouvez compiler le programme vous-même
5. Définissez la clé correcte fournie par le propriétaire de votre serveur en cliquant sur l'icône des paramètres
4. Le programme téléchargera tous les mods de Google Drive et les placera dans le dossier `%Appdata%/.minecraft/mods`
<h>

### En tant que propriétaire de serveur Minecraft

  * Vous devez fournir l'identifiant de votre dossier Google Drive qui contient les mods.
Vous pouvez obtenir l'identifiant à partir de l'url du dossier :
  * Dans cette URL `https://drive.google.com/drive/folders/1Z2X3C4V5B6N7M8K9J0H1G2F3D4S5A6Q7?usp=sharing` l'identifiant est `1Z2X3C4V5B6N7M8K9J0H1G2F3D4S5A6Q7`
  * Renommez le dossier au nom de votre serveur
>Veuillez noter que votre dossier doit être défini sur tous ceux qui ont le lien peuvent voir le dossier

## Contribuant

Si vous souhaitez contribuer, vous pouvez le faire en ouvrant une pull request ou un ticket.

## Outils tiers utilisés
- [Json](https://github.com/nlohmann/json) pour analyser les réponses HTTP de Google Drive

## Problèmes connus
- La fermeture de la fenêtre des paramètres n'arrêtera pas l'écran de chargement
