x11fs X window virtual filesystem
=================================

*This is heavily based off wmutils. [Check them out](http://wmutils.io).*

x11fs is a tool for manipulating X windows.
It creates a vitual filesystem to represent open windows, similar to what /proc does for processes.
This allows windows to be controlled using any language or tool with simple file IO, in a true unix fashion.


Build
-----

After installing the FUSE and XCB developement packages, x11fs can be built using the `make` command. Installation can be done by invoking `make install`.

On distros, the development packages are:

* Gentoo: `x11-libs/libxcb x11-libs/xcb-util-wm sys-fs/fuse`
* Ubuntu: `libxcb1-dev libxcb-icccm4-dev libfuse-dev`

Usage
-----

See the [wiki](https://github.com/sdhand/x11fs/wiki) for more information.
Or go [here](https://github.com/sdhand/x11fs/wiki/Example-Setup) for a simple example setup.

Contact
-------

Please join #x11fs on freenode if you have any questions or just want to talk about the project. Please be patient however, there are not many members of the channel so it may be inactive at times.

Thanks to
---------

[Luiz de Milon](https://github.com/kori) for helping to come up with the initial idea.
The creators of [wmutils](https://wmutils.io) for providing some inspiration, and some basic xcb code to study.
