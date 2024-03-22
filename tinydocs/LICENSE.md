Original Source: https://github.com/tinygettext/tinygettext

tinygettext - A gettext replacement that works directly on .po files

Copyright (c) 2004-2016
Copyright (c) 2006 Ingo Ruhnke <grumbel@gmail.com>

* Bastiaan Zapf <bzapf@example.org>
* Christoph Sommer <mail@christoph-sommer.de>
* Georg Kilzer (leper) <leper@wildfiregames.com>
* Ingo Ruhnke <grumbel@gmail.com>
* Mathnerd314 <mathnerd314.gph@gmail.com>
* Matt McCutchen <matt@mattmccutchen.net>
* Matthias Braun <matze@braunis.de>
* Nathan Phillip Brink <ohnobinki@ohnopublishing.net>
* Poren Chiang <ren.chiang@gmail.com>
* Ravu al Hemio <ondra.hosek@gmail.com>
* Ryan Flegel <rflegel@gmail.com>
* Tim Goya <tuxdev103@gmail.com>
* Tobias Markus <tobbi.bugs@gmail.com>
* Wolfgang Becker <uafr@gmx.de>

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgement in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

potext - An update and streamlining of tinygettext

We are stripping out a few things, like asserts, replacing the "file-system"
code with our own file-functions module, and removing logging in favor of our
message-functions module. The build system has been replaced by Meson.
Every module has been heavily modified for readability and C++11 and up.

If you do not like the changes or the GPL licensing, use the original
tinygettext project, available at the GitHub URL above.

// vim: sw=4 ts=4 wm=2 et
