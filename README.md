# SweetHomeMaidPlayer
某寢室再生機  
- Requisite for run
  - Windows OS later than Window 8
  - MSVC
 
## How to play

Select a folder like `Stillstill102009` or `VoiceStoryCardcard102009`, which are in the same directory, from window menu.
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

After selecting either, the other one would be searched, though some scenes may lack audios or images.

## Mouse input description

The scene action is heavily dependent on mouse inputs.
| Input | Action |
----|---- 
Mouse wheel| Scale up/down
Left button + mouse wheel| Speed up/down
Left button click| Move on to next scene.
Left button drag|Move Zooming frame. This works only when the scaling is beyond the display resolution.
Middle button|Reset scaling/speed to default.
Right button + mouse wheel|Play next/previous audio file.
Right button + middle button|Remove/show window title bar and menu bar. In removing, the window goes to the origin of the display.
Right button + left button|Move window. This works only when the window bars are removed.

## Window menu description

| Menu | Item | Action |
----|---- |---- 
Folder| Open| Show folder select dialogue.
 -| Next| Open the next folder in naming order.
 -| Back| Open the previous folder in naming order.
Audio| Next| Play next audio file.
 -| Back| Play previous audio file.
 -| Play| Replay the current audio file.
 -| Loop| Set/reset loop setting.
 -| Setting| Show audio setting dialogue for the volume and the rate.
Image| Pause| Pause/resume the animation scene. In pause mode, left click gives frame-by-frame going.
