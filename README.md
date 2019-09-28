## Image Viewer
A simple imageviewer for POSIX

### Build
```
make
```

### Build dependencies

#### SDL2

install in arch:

```
# pacman -S sdl2
```
#### SDL2_Image

install in arch:

```
# pacman -S sdl2_image
```

### Usage

```
./ImageViewer myimagesdirectory/
```

### Keys
b or z: toggle zoom

s or f: start slideshow

q: quit

space: next image

arrows: move image

t: toggle thumbnail mode

##### While in slideshow keys

b or z: toggle zoom

q: quit slideshow

+/-: increase/decrease wait time length

##### While in thumnail mode

space or .: next batch of thumbnails

backspace or ,: previous batch of thumbnails

q: exit thimbnail mode
