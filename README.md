# scrab
**Scr**een gr**ab**
adds a configurable hotkey to quickly select a screen region and add it as an image source to the current scene

The plugin saves all screenshots into your home directory by default. If you wish to change that open your obs-studio
and edit it:

### Windows:
Open ``%appdata%\obs-studio\global.ini``
### Linux:
Open ``~/.config/obs-studio/global.ini``

Then add the follwing at the end of the file and replace ``{path}`` with whatever folder you want scrab to use for screenshots:
```ini
[scrab]
image_folder={path}
```

![](out.gif)
