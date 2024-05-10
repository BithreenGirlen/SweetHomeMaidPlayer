# SweetHomeMaidPlayer
某寢室再生機  
- Requisite to run
  - Windows OS later than Window 8
  - MSVC 2015-2022 (x64)
 
## How to play

1. Click `Open` menu item in the `Folder` entry.
2. Select a folder like `Stillstill102009` or `VoiceStoryCardcard102009`, which are in the same directory.
<pre>
Resource
├ Stillstill102009
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

## Mouse command functions

| Command | Function |
----|---- 
Mouse wheel| Scale up/down
Left button + mouse wheel| Speed up/down the animation.
Left button click| Go to the next scene.
Left button drag|Move zooming frame. This works only when the size of scaled image is beyond the display resolution.
Middle button|Reset scale/speed to default.
Right button + mouse wheel|Play the next/previous audio file.
Right button + middle button|Hide/show window title and menu bar. Having hidden, the window goes to the origin of the primary display.
Right button + left button|Move window. This works only when the window title/bar are hidden.

## Keyboard functions

| Key | Function |
----|---- 
T | Show/Hide text.

- Installation of `游明朝` font in the machine is necessary in order to show text.
- Scenario files are assumed to be existed in `/Episode/Card` directory.
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
----|---- |---- 
Folder| Open| Show folder select dialogue.
 -| Next| Open the next folder.
 -| Back| Open the previous folder.
Audio| Loop| Set/reset loop setting.
 -| Setting| Show a dialogue for audio volume and rate setting.
Image| Pause| Pause/resume the animation scene. In pause mode, left click gives frame-by-frame going.

## Library
- [JSON for Modern C++ v3.11.3](https://github.com/nlohmann/json/releases/tag/v3.11.3)
