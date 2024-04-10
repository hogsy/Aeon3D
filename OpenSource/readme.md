# hg3d

This is a fork of Genesis3D 1.1 that's updated to compile on current platforms w/ current compilers. I threw this together because I couldn't find a fork of the engine that wasn't trying to do a bunch of extra things I wasn't particularly interested in.

Plus no offence intended, but some other forks I'd seen seemed to have absolutely butchered some things.

## Roadmap

Some short-term things just to get this into a useful state.

- [ ] OpenGL driver to replace D3D/Glide drivers
- [ ] Replace networking DirectPlay code

Eh, I'm not really planning on doing much else at this stage? Some things I'm considering...

- Clean up all the includes - system includes via system.h, etc.
- Deprecating the editor and add support for Valve .map format instead

## License

The original engine was under the `GENESIS3D PUBLIC LICENSE`, however, it appears the licence has since been waivered by Genesis Technologies [here](https://www.genesis3d.com/licensing.html).

> Genesis3D 1.x is abandonware and the principal stakeholders are gone. You may use it free of license restrictions.

In such a case, I've opted to release this under [MIT](license), though you can find the original license [here](license.old).
