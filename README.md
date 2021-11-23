# Uxn-Esp32

A port of the [Uxn](https://wiki.xxiivv.com/site/uxn.html) virtual machine and [Varvara](https://wiki.xxiivv.com/site/varvara.html) environment to the esp32 platform.

# How to build it

Install [PlatforimIO Core](https://platformio.org/install/cli) if you want to use the command line only. It is also available as a plugin for several IDEs/editors (Emacs, vim, VSCode, Atom, etc).


```
git clone https://github.com/max22-/uxn-esp32
cd uxn-esp32
pio run -t uploadfs
pio run -t upload
```

Do not forget to edit [User_Setup.h](https://github.com/Bodmer/TFT_eSPI/blob/master/User_Setup.h) in the TFT_eSPI lib directory (to configure the screen pins).

I will try to make it buildable with Arduino IDE as well in the future.

The roms must be in the "data" folder. (they are uploaded with "pio run -t uploadfs").

# Devices available

Console, screen, file. More to come later !