portalDS (aka Aperture Science DS)
=======

This is the source code for portalDS, a homebrew adaptation of Valve Software's Portal for the Nintendo DS. Most of the code for this was written by myself, smea ( http://smealum.net , https://twitter.com/smealum ), and the graphics were made by Lobo ( http://infectuous.net ). You can email me at smealum@gmail.com; see my webpage for other means of contacting me.

While reading this, please keep in mind that this includes a bunch of legacy code that may date back as far as 2007; most of this probably isn't a good example of "good practice". Likewise, keep in mind that a number of features were rushed in the two weeks right before the neoflash competition ended in August 2013; a lot of these should probably be rewritten. (see git log to see what i'm referring to)

Compiling this requires devkitARM; see http://devkitpro.org for more information on setting everything up.
This is pretty much the game's final version. It's not quite complete feature-wise but I have no plans on continuing it.
The code is provided "as-is" (whatever that entails), and can be freely used so long as it's not for commercial purposes and that proper credit is given to the original author.
I'd love to hear about what people do with this, if anything, so shoot me an email if you feel like using some (or all) of the code ! I'll also try to answer any questions you may have.

Test chamber video : http://www.youtube.com/watch?v=TlXYyjhOYQU

Level editor video : http://www.youtube.com/watch?v=V2OwozMNJFE

Acknowledgements :

- the code for the rigid body dynamics engine is heavily based on chris hecker's articles : http://chrishecker.com/Rigid_Body_Dynamics
- the code for loading ini files was written by N. Devillard
- the code used for level data compression was taken from GRIT
- the code used for loading MD2 models and PCX files was written by David HENRY
- this project uses libnds and maxmod
- xmem.c was written by SunDEV

Please let me know if you think I missed anything.
