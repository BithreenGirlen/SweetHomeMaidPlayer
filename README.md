# SweetHomeMaidPlayer

某寢室再生機  
- Requisite to run
  - Windows OS later than Window 8
  - MSVC 2015-2022 (x64)
 
## How to play

1. Click `Open` menu item in the `Folder` entry.
2. Select a folder like `AdvStillstill102009` or `VoiceStoryCardcard102009`, which are in the same directory.
<pre>
Resource
├ AdvStillstill102009
│   ├ 102009_1.jpg
│   ├ 102009_2.jpg
│   ├ ...
│   └ 102009_63.jpg
├  ...
├ VoiceStoryCardcard102009
│   ├ voice_story1020093_102_001.mp3
│   ├ voice_story1020093_102_002.mp3
│   ├ ...
│   └ voice_story1020093_102_035.mp3
└ ...
</pre>

The other one will also be loaded after selecting the either.

## Mouse functions

| Input | Function |
| ---- |---- |
| Wheel scroll | Scale up/down |
| L-pressed + whell scroll | Speed up/down the animation. |
| L-click | Switch to the next still/animation. |
| L-drag | Move view point. This works only when the size of scaled image is beyond the display resolution. |
| M-click | Reset scale/speed to default. |
| R-pressed + wheel scroll | Play the next/previous audio file. |
| R-pressed + M-click | Hide/show window's frame and menu bar. Having hidden, the window goes to the origin of the primary display. |
| R-pressed + L-click | Move window. This works only when the window's frame/bar are hidden. |

## Keyboard functions

| Key | Function |
| ---- | ---- |
| <kbd>C</kbd> | Toggle text colour between black and white. |
| <kbd>T</kbd> | Show/Hide text. |
| <kbd>∧</kbd> | Open the previous folder. |
| <kbd>∨</kbd> | Open the next folder. |

- Installation of `游明朝` font in the machine is necessary in order to show text.
- Scenario files are assumed to exist in `/Episode/Card` directory.
<pre>
...
├ Episode
│  ├ Card
│  │  ├ story1010011.json
│  │  └ ...
│  └ ...
└ Resource
   └ ...
</pre>

## Window menu functions

| Entry | Item | Function |
| ---- | ---- | ---- |
| Folder| Open | Open folder-select-dialogue. |
| - | Next | Open the next folder. |
| - | Back | Open the previous folder. |
| Audio | Loop | Set/reset audio loop setting. |
| - | Setting | Show a dialogue for audio volume and rate setting. |
| Image | Pause | Pause/resume the animation scene. In pause mode, left click gives frame-by-frame going. |
