# Aeon3D

This is a fork of Genesis3D 1.1 that's updated to compile on current platforms w/ current compilers. I threw this together because I couldn't find a fork of the engine that wasn't trying to do a bunch of extra things I wasn't particularly interested in, nor one that just _worked_.

Why Genesis3D? Eh, it's rather interesting historically, and this is my way of preserving it. This might also be helpful or useful to those that want to play around with an older, more simplistic engine rather than the behemoths we've got today.

As far as a goal goes here, the main focus is to get the engine working, everything compiling and then gradually modernize it while retaining its simplicity.

Additionally, I didn't know at first, but it's got a very [liberal license](#license) compared to some others out there.

## Status

### What works?

- GTest can be built and run under Windows 11 (x86)
- Direct3D and 3dfx Glide renderers are working
- There's a command-line tool for compiling maps

### What's left?

Some short-term things just to get this into a useful state.

- [ ] OpenGL driver to replace D3D/Glide drivers (**in-progress**)
- [ ] Get the tools building
- [ ] Replace networking DirectPlay code
- [ ] Compile under x64 target on Windows
- [ ] Get it compiling under Linux/Unix targets (**in-progress**)

Some extra things I'm crawling towards...
- General code clean-up, it's a mess... :')
  - Clean up all the includes - system includes via system.h, etc.
  - Remove duplicate code
- Add support for Valve .map format
- Support for Valve WAD3 format in addition to TXL

## License

The original engine was under the `GENESIS3D PUBLIC LICENSE`, however, it appears the licence has since been waivered by Genesis Technologies [here](https://www.genesis3d.com/licensing.html).

> Genesis3D 1.x is abandonware and the principal stakeholders are gone. You may use it free of license restrictions.

In such a case, I've opted to release this under [MIT](license), though you can find the original license [here](license.old).

I've also opted to include the original GDemo sources and content too, though this, to my knowledge, was technically _not_ part of the Genesis3D 1.x distribution but distributed separately, so it's unclear if the waiver applies there. I would suggest considering it just for historical/educational purposes.
