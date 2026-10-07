dw2003recomp @VERSION@ for Windows (x86_64)
===========================================

Digimon World 2003 (PlayStation, Europe) rebuilt from its decompiled code as a native program.
https://github.com/gascarcella/dw2003recomp

What you need
-------------
* Windows 10 or newer, 64-bit.
* Your own disc image of Digimon World 2003 (PlayStation, Europe, SLES-03936), as a .cue with its .bin (or the .bin
  alone). No game data is included: the launcher checks the image's SHA-1 and only the European disc is accepted.

Starting
--------
1. Unzip this folder anywhere (its files must stay together: the launcher finds the game beside itself).
2. Run dw2003-launcher.exe. The programs are not signed, so Windows may show a SmartScreen warning the first time:
   choose "More info", then "Run anyway".
3. On the Disc screen, pick your disc image (the file dialog, a drag-and-drop or a typed path).
4. Press Play. The launcher hides while the game runs and comes back when it ends.

Settings, Controls and Mods are the launcher's other screens. Keyboard defaults: arrows the D-pad, X cross, C
circle, Z square, S triangle, Enter start, Backspace select, Q/E L1/R1, 1/3 L2/R2; P pauses, F11 fullscreen, Tab
holds fast-forward; a gamepad works as on a PlayStation. Everything can be rebound on the Controls screen.

Where things are kept
---------------------
%APPDATA%\dw2003\  (usually C:\Users\<you>\AppData\Roaming\dw2003\):
  settings.json           the launcher's settings
  card1.mcd, card2.mcd    the memory cards (also readable by an emulator)
  logs\last-run.log       the last game run's output (and last-run.1.log, the one before)
  crashes\                crash reports (crash-<date>.txt) and minidumps (crash-<date>.dmp)
A portable.txt file beside dw2003-launcher.exe keeps all of this in the launcher's own folder instead.

Reporting a problem
-------------------
This build was tested on Linux through Wine and Proton, not yet on real Windows. If the game crashes, the launcher's
Play screen shows the exit status and the last lines: press "Copy" and paste the text into a new issue at
https://github.com/gascarcella/dw2003recomp/issues (the bug report form), and attach the matching crash-<date>.dmp
from the crashes folder if there is one. Your disc image and your saves are never part of a report.

LICENSES\README.txt lists the licences of what this package contains.
