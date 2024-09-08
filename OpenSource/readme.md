# hg3d

This is a fork of Genesis3D 1.1 that's updated to compile on current platforms w/ current compilers. I threw this together because I couldn't find a fork of the engine that wasn't trying to do a bunch of extra things I wasn't particularly interested in.

Why Genesis3D? Eh, it's rather interesting historically, and this is my way of preserving it. This might also be helpful or useful to those that want to play around with an older, more simplistic engine rather than the behemoths we've got today.

Additionally, I didn't know at first, but it's got a very [liberal license](#license) compared to some others out there.

## Roadmap

Some short-term things just to get this into a useful state.

- [ ] OpenGL driver to replace D3D/Glide drivers (**in-progress**)
- [ ] Get the tools building
- [ ] Replace networking DirectPlay code
- [ ] Get it compiling under Linux/Unix targets (**in-progress**)

Eh, I'm not really planning on doing much else at this stage? Some things I'm considering...

- Clean up all the includes - system includes via system.h, etc.
- General code clean-up, it's a mess... :')
- Add support for Valve .map format
- Support for Valve WAD3 format in addition to TXL

## License

The original engine was under the `GENESIS3D PUBLIC LICENSE`, however, it appears the licence has since been waivered by Genesis Technologies [here](https://www.genesis3d.com/licensing.html).

> Genesis3D 1.x is abandonware and the principal stakeholders are gone. You may use it free of license restrictions.

In such a case, I've opted to release this under [MIT](license), though you can find the original license [here](license.old).

I've also opted to include the original GDemo sources and content too, though this, to my knowledge, was technically _not_ part of the Genesis3D 1.x distribution but distributed separately, so it's unclear if the waiver applies there. I would suggest considering it just for historical/educational purposes.
