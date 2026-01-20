# Atari STE overscan

https://github.com/user-attachments/assets/1382505c-a8ce-404d-a8d7-5cc342749daf

Extend your Atari STE's pixels beyond their limits with this overscan example
and blitter-powered bouncy ball demo to show the extra screen space in action.

The build produces three outputs:

- `NONE.TOS` as a baseline mode with no overscan for comparison (320x200)
- `VERTICAL.TOS` for top-bottom overscan (320x268)
- `FULL.TOS` for full-screen overscan (400x268)

So, why not drop the `overscan_ste_*.s` files into your own projects and give them a try!

## Build

You can build the examples by installing [atarist-toolkit-docker](https://github.com/sidecartridge/atarist-toolkit-docker) and running:

```bash
stcmd make
```

## Credits

Adapted from examples shared on [Atari-Forum](https://www.atari-forum.com/viewtopic.php?p=231136&sid=fb6439b4fd4d7a95dc81f13afed6924f#p231136).

## License

[BSD 2 Clause](LICENSE)
